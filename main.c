#include "cpu.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#define RAM_SIZE  16*1024
#define ROM_SIZE  32*1024

unsigned char* REN_rombuf;
unsigned char* REN_rambuf;
int romsize;
byte input_register;
byte output_register;
byte test_value = 0;
int tx_requested = 0;
byte spi_in = 0;
byte spi_out = 0;
int shifter = 0;
word32 last_time = 0;
word32 last_poll = 0;
int debugger_is_on = 0;
int breakpoints_are_enabled = 0;

void (*on_spi_complete)(void) = 0;

byte romcode[256] = {
 0xA9, 0x00, 0x85, 0xFD, 0xB8, 0xA9, 0xFF, 0x85,
 0xFE, 0xA9, 0xFF, 0x85, 0xFF, 0xEA, 0xEA, 0xEA,
 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xC6,
 0xFF, 0xD0, 0xF2, 0xC6, 0xFE, 0xD0, 0xEA, 0xA5,
 0xFD, 0xD0, 0x08, 0xA9, 0xFF, 0x85, 0xFD, 0x18,
 0xFB, 0x50, 0xDA, 0xA9, 0x00, 0x85, 0xFD, 0x38,
 0xFB, 0x50, 0xD2
};


void EMUL_handleWDM(byte opcode, word32 timestamp) {

    return;
}

int kbhit();
int getch();
void spi_print();

void EMUL_hardwareUpdate(word32 timestamp) {

    static int oldE = 0;
    static double old_timestamp = 0;
    static double old_time = 0;
    static double timef = 0;
    static byte tx_buffer, rx_buffer;
    
    struct timespec t;
    double clocks_elapsed;
    double time_elapsed;
    static long old_nsecs, new_nsecs, nsec_diff;

    //TODO: Replace this with some kind of actual debug console
    if(debugger_is_on)
        while(getch() < 1); //DEBUG REMOVE 
    
    //CPU emulation throttling
    while(1) {
    
        clock_gettime(CLOCK_REALTIME, &t); 
        new_nsecs = t.tv_nsec;

        if(new_nsecs < old_nsecs)
            nsec_diff = (new_nsecs + 1000000000) - old_nsecs;
        else
            nsec_diff = new_nsecs - old_nsecs;
        
        if(nsec_diff >= 160)
            break;
    }

    old_nsecs = new_nsecs;
/*
    clocks_elapsed = timestamp - old_timestamp;
    
    if(clocks_elapsed >= 12000000) {

        timef = ((double)t.tv_sec * 1000) + ((double)t.tv_nsec / 1.0e6);
        time_elapsed = timef - old_time;
        printf("IPS: %f/%f = %f\n", clocks_elapsed, time_elapsed, (1000 * clocks_elapsed)/time_elapsed);
        old_time = timef;
        old_timestamp = (double)timestamp;
    }
*/
    if(oldE != E) {

        int i;

        if(!E) printf("[Emulation mode off]\n");
            //for(i = 0; i < 256; i++)
                //printf("%02X ", REN_rambuf[i]);

        oldE = E;
    }

    if(tx_requested) {

        if(last_time == 0) {

            //Init new transfer
            rx_buffer = 0;
            tx_buffer = spi_out;
            shifter = 0;
            last_time = timestamp;
            input_register = 0;
        } else {

            //Wait 100 cycles between steps
            if(timestamp >= last_time + 100) {

                last_time = timestamp;

                if(input_register & 0x01) {

                    //^clock was high last step
                    //Get the current input value and then clock it low
                    rx_buffer = (((output_register & 0x1) * 0x80) | ((rx_buffer >> 1 )& 0x7F));
                    input_register = input_register & 0xFE;

                    //Increase the clock count, check for cycle completion, write any received value
                    shifter++;
                    
                    if(shifter == 8) {

                        tx_requested = 0;
                        spi_in = rx_buffer;

                        //if(spi_out || spi_in)
                        //    printf("[SPI: S-%02X R-%02X]\n", spi_out, spi_in);

                        if(on_spi_complete)
                            on_spi_complete();
                    }
                } else {

                    //^clock was low last step
                    //Clock it high and set the next data out value
                    input_register = ((tx_buffer & 0x80) >> 6) | 0x01;
                    tx_buffer = tx_buffer << 1;
                }
            }
        }
    } else {

        if(kbhit()) {

            //Queue a transfer (should have a helper function to do all of this)
            last_poll = timestamp;
            spi_out = (byte)(getch() & 0xFF);
            tx_requested = 1;
            on_spi_complete = spi_print;
            last_time = 0;
        } else if(timestamp >= last_poll + 10000) {

            last_poll = timestamp;
            spi_out = 0;
            tx_requested = 1;
            on_spi_complete = spi_print;
            last_time = 0;
        } 
    }
}

byte MEM_readMem(word32 address, word32 timestamp, word32 emulFlags) {

    byte b = 0;

    if(address & 0x8000)
        b = REN_rombuf[address & 0x7FFF];
    else if(address & 0x4000)
        { b = input_register;  }
    else
        b = REN_rambuf[address & 0x3FFF];

    return b;
}

void MEM_writeMem(word32 address, byte b, word32 timestamp) {

    if(address & 0x8000)
        output_register = b; 
    else if(!(address & 0x4000)) {
        
        if(address == 0x0000FF) {

            debugger_is_on = debugger_is_on ? 0 : breakpoints_are_enabled;
	    CPU_setTrace(debugger_is_on);
	}

        REN_rambuf[address & 0x3FFF] = b;
/*
        if(address >= 0x0200 && address < 0x0300) {

            int i;
            for(i = 0; i < 10; i++)
                printf("%02X ", REN_rambuf[0x200 + i]);

            printf("\n");
        }
        */
    }

    //Anything we try to write to 0x4000 - 0x7FFF should be dumped to space
}

int kbhit() {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch() {
    int r;
    unsigned char c;
    if((r = read(0, &c, sizeof(c))) < 0)
        return r;
    else
        return c;
}

void spi_print(void) {

    static byte rx_count = 0;
    static int printing = 0;

    if(printing) {

        printf("%c", spi_in);
        fflush(stdout);
        rx_count--;
    } else {

        if(spi_in) {
        
            //printf("[Found 0x%02X pending chars]\n", spi_in);
            rx_count = spi_in;
            printing = 1;
        }
    }

    if(rx_count) {

        spi_out = 0x00;
        tx_requested = 1;
        on_spi_complete = spi_print;
        last_time = 0;
    } else {

        printing = 0;
    }
}

int main(int argc, char* argv[]) {

    static struct termios oldit, newit, oldot, newot;

    //Get args
    if(argc > 1) {
   
        int i;
	for(i = 1; i < argc; i++) {
	
	    if(!strcmp(argv[i], "--enable-breakpoints"))
                breakpoints_are_enabled = 1;
	}
    }

    CPU_reset();
    CPU_setTrace(0);
    CPUEvent_initialize();
    CPU_setUpdatePeriod(1);

    //Set stdin to non-blocking
    fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);

    //Turn off line-at-a-time
    tcgetattr(0, &oldit);
    newit = oldit;
    newit.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &newit);

    tcgetattr(1, &oldot);
    newot = oldot;
    newot.c_lflag &= ~(ICANON);
    tcsetattr(0, TCSANOW, &newot);

    FILE* romfile = fopen("boot.rom", "rb");

    if(!romfile) {

        printf("Couldn't find ROM image 'boot.rom'.\n");
        return 0;
    }

    fseek(romfile, 0, SEEK_END);
    romsize = ftell(romfile);
    rewind(romfile);

    if(romsize > ROM_SIZE) {

        printf("System rom image size greater than 32k.\n");
        fclose(romfile);
        return 0;
    }

    REN_rombuf = (unsigned char*)malloc(ROM_SIZE);

    if(!REN_rombuf) {

        printf("Couldn't allocate ROM memory.\n");
        fclose(romfile);
        return 0;
    }

    if(fread(REN_rombuf, 1, romsize, romfile) != romsize) {
    
        printf("Couldn't read ROM image.\n");
        fclose(romfile);
        free(REN_rombuf);
        return 0;
    }

    //fclose(romfile);
    REN_rambuf = (unsigned char*)malloc(RAM_SIZE);

    if(!REN_rambuf) {

        printf("Couldn't allocate RAM memory.\n");
        free(REN_rombuf);
        return 0;
    }

    input_register = 0;

    printf("Starting execution\n");
    CPU_run();

    fclose(romfile);
    tcsetattr(0, TCSANOW, &oldit);
    tcsetattr(0, TCSANOW, &oldot);
 
    return 0;
}

