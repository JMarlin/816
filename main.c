#include "cpu.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

#define RAM_SIZE  16*1024
#define ROM_SIZE  32*1024
#define EXIT_DEBUGGER -1
#define QUIT_APPLICATION -2
#define INVALID_COMMAND 0

<<<<<<< HEAD
#define DBG_CMD_PROCEED 1
#define DBG_CMD_REMAIN 2

=======
struct termios oldit, newit, oldot, newot;
char dbg_inbuf[255];
>>>>>>> 89f4e9f6d5b00cf5ea8460112056537af8aac737
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
int debugger_is_enabled = 0;

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

<<<<<<< HEAD
int read_line(char* buffer, unsigned int buf_size) {

    char temp_char = 0;
    unsigned int char_count = 0;

    while(1) {
    
        while(!kbhit());

        temp_char = getch() & 0xFF;
        buffer[char_count] = temp_char;
        printf("%c", temp_char); fflush(stdout);

        if(temp_char == '\n')
            break;
        
        char_count++;

        if(char_count == (buf_size - 1)) {

            char_count++;
            break;
        }
    }

    buffer[char_count] = 0;
    return char_count;
}

typedef int (*DebugCommandFunction)(int, char**);

int debug_command_step(int argc, char* argv[]) {

    return DBG_CMD_PROCEED;
}

int debug_command_run(int argc, char* argv[]) {

    debugger_is_on = 0;
    return DBG_CMD_PROCEED;
}

int hex24_to_int(char* hex_string) {

    char* ptr;

    if(strlen(hex_string) != 6)
        return -1;

    return strtol(hex_string, &ptr, 16);
}

#define BP_MODE_CHECK 0
#define BP_MODE_INSERT 1
#define BP_MODE_REMOVE -1
#define BP_MODE_COUNT 2
#define BP_MODE_GET 3

int breakpoint_check(int address, int mode) {

    static unsigned int bp_count = 0;
    static int* bp_entry = (int*)0;

    int found_index = -1;
    int i;

    if(mode == 2) { //Mode 2 == get entry count 
        
        return (int)bp_count;
    }

    if(address > 0xFFFFFF || address < 0)
        return -1; //Bad address 

    if(mode == 3) { //Mode 3 == get entry at index  

        if(address >= bp_count)
            return 0; //Address not found

        return (int)bp_entry[address];
    }

    for(i = 0; i < bp_count; i++)
        if(bp_entry[i] == address) {
            
            found_index = i;
            break;
        }

    if(found_index == -1 && mode == 1) { //mode 1 == insert breakpoint

        bp_entry = (int*)realloc((void*)bp_entry, sizeof(int) * (bp_count + 1));
        bp_entry[bp_count++] = address;
    }

    if(found_index != -1 && mode == -1) { //mode -1 == delete breakpoint

        for(i = found_index; i < bp_count - 1; i++)
            bp_entry[found_index] = bp_entry[found_index + 1];

        bp_count--;
    }

    return found_index != -1;
}

int debug_command_break(int argc, char* argv[]) {

    if(argc != 2 && argc != 1) {

        printf("Wrong number of arguments.\nUsage: break <24-bit-hex-address>\n");
        fflush(stdout);

        return DBG_CMD_REMAIN;
    }

    if(argc == 1) {

        int i;

        for(i = 0; i < breakpoint_check(0, BP_MODE_COUNT); i++) {

            printf("    [%d] 0x%06X\n", i, breakpoint_check(i, BP_MODE_GET));
            fflush(stdout);
        }
    } else {

        int address = hex24_to_int(argv[1]);

        if(address < 0) {

            printf("Could not parse the given address '%s'\n", argv[1]);
            fflush(stdout);

            return DBG_CMD_REMAIN;
        } 

        switch(breakpoint_check(address, BP_MODE_INSERT)) {
            case 0:
                printf("Breakpoint added at %06X\n", address);
                fflush(stdout);
                break;
            case -1:
                printf("Bad breakpoint address\n");
                fflush(stdout);
                break;
            default:
                printf("Breakpoint already exists\n");
                fflush(stdout);
                break;
        }
    }

    return DBG_CMD_REMAIN;
}

int debug_command_args(int argc, char* argv[]) {

    int i;

    for(i = 0; i < argc; i++) {

        printf("%s\n", argv[i]);
        fflush(stdout);
    }

    return DBG_CMD_REMAIN;
}

int debug_command_exam(int argc, char* argv[]) {

    int count = 16;
    int address = 0;
    int i;

    if(argc != 2 && argc != 3) {

        printf("Usage: exam [base-address] <count>\n");
        fflush(stdout);

        return DBG_CMD_REMAIN;
    }

    if(argc == 3)
        count = atoi(argv[2]);

    address = hex24_to_int(argv[1]);

    if(address < 0) {

        printf("Bad address.\n");
        fflush(stdout);

        return DBG_CMD_REMAIN;
    }

    printf("%06X: ", address);
    fflush(stdout);

    for(i = 0; i < count; i++) {

        printf("%02X", MEM_readMem((int)address + i, 0, 0));

        if(i == count - 1)
            printf("\n");
        else
            printf(" ");
    }

    return DBG_CMD_REMAIN;
}

char** string_to_argv(char* input_buffer, int* argc) {

    char** return_array = (char**)0;
    int building = 0;
    char read_buffer[256];
    int chars_read;

    *argc = 0;

    while(1) {

        if(building) {

            if(*input_buffer <= ' ') {

                return_array[(*argc) - 1] = (char*)malloc(sizeof(char) * (chars_read + 1));
                memcpy((void*)return_array[(*argc) - 1], (void*)read_buffer, chars_read);
                return_array[(*argc) - 1][chars_read] = 0;
                building = 0;

                if(*input_buffer == 0)
                    break;

                continue;
            }

            read_buffer[chars_read++] = *input_buffer++;
        } else {

            if(*input_buffer <= ' ') {

                if(*input_buffer == 0)
                    break;

                input_buffer++;
                continue;
            }

            return_array = (char**)realloc((void*)return_array, sizeof(char*) * (++(*argc))); //This can fail. That would be bad.
            building = 1;
            chars_read = 0;
        }        
    } 

    return return_array;
}

void free_argv_memory(int argc, char* argv[]) {

    int i;

    for(i = 0; i < argc; i++)
        free((void*)argv[i]);
    
    free((void*)argv);
}

int parse_debug_command(char* command_buffer) {

#define COMMAND_COUNT 5
    char* command_entry[COMMAND_COUNT] = {
        "step",
        "run",
        "break",
        "args",
        "exam"
    };
    DebugCommandFunction command_function[COMMAND_COUNT] = {
        debug_command_step,
        debug_command_run,
        debug_command_break,
        debug_command_args,
        debug_command_exam
    };

    int argc;
    int retval = 0;
    char** argv = string_to_argv(command_buffer, &argc);

    if(argc == 0)
        return 0;

    int i;
    for(i = 0; i < COMMAND_COUNT; i++)
        if(!strcmp(command_entry[i], argv[0])) {

            retval = command_function[i](argc, argv); //TODO: Split command buffer into argvals
            break;
        }

    free_argv_memory(argc, argv);

    return retval;
}

void do_debugger(void) {

    static char in_buf[256] = {0};
    int parse_result = 0;
    int execution_paused = 1;

    CPU_debug();
    while(execution_paused) {
    
        printf("dbg>"); fflush(stdout);

        read_line(in_buf, 256);
        
        switch(parse_debug_command(in_buf)) {
            case DBG_CMD_PROCEED:
                execution_paused = 0;
                break;
            case DBG_CMD_REMAIN:
                 break;
            default:
                printf("Unknown debugger command.\n"); fflush(stdout);
                break;
        }
=======
void read_line(char* out_buffer, unsigned int bufsz) {

    int next_char = 0;
    int i = 0;
 
    while(1) {

        while((next_char = getch()) < 0);

        printf("%c", (char)(next_char&0xFF));
        fflush(stdout);

	if((char)(next_char&0xFF) == '\n') {
	    
	    out_buffer[i] = 0x0;
	    return;
	}

	out_buffer[i++] = (char)(next_char&0xFF);

	if(i == bufsz - 1) {

	    out_buffer[i] = 0x0;
	    return;
	}
    }
}

typedef int (*debug_func)(char*);

int debug_run(char* command_string) {

    debugger_is_on = 0;
    //CPU_setTrace(0);

    return EXIT_DEBUGGER;
}

int debug_step(char* command_string) {

    return EXIT_DEBUGGER;
}

int debug_exit(char* command_string) {

    return QUIT_APPLICATION;
}

int hex_to_int(char* hex_string, int* out_int) {

    return 1;
}

int debug_set_breakpoint(char* command_string) {

    char addrbuf[7] = {0};
    int break_addr;
 
    while(1) {

        printf("Break on what address?: ");
        fflush(stdout);
        read_line(addrbuf, 6);

        if(hex_to_int(addrbuf, &break_addr))
            break;

        printf("Bad address.\n");
        fflush(stdout);
    }
}

int dispatch_command(char* command_string) {

#define COMMAND_COUNT 4
    static char* command_names[COMMAND_COUNT] = {
        "run",
	"step",
        "break",
	"exit"
    };
    static debug_func command_pointers[COMMAND_COUNT] = {
        debug_run,
	debug_step,
        debug_set_breakpoint,
	debug_exit
    };

    //Todo: Later, we should split up the incoming string just like an incoming argv array

    int i;
    
    for(i = 0; i < COMMAND_COUNT; i++) {
        
	if(!strcmp(command_names[i], command_string))
	    return command_pointers[i](command_string);
    }

    return INVALID_COMMAND;
}

void enter_debug_console() {

    CPU_debug();

    while(1) {

	printf("dbg>");
	fflush(stdout);
	read_line(dbg_inbuf, 255);

	switch(dispatch_command(dbg_inbuf)) {
	    case EXIT_DEBUGGER:
                return;
		break;
	    case QUIT_APPLICATION:
		CPU_quit();
		return;
		break;
	    case INVALID_COMMAND:
	    default:
		printf("Invalid command.\n");
		fflush(stdout);
		break;
	}
>>>>>>> 89f4e9f6d5b00cf5ea8460112056537af8aac737
    }
}

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

<<<<<<< HEAD
    if(breakpoint_check(PC.A, BP_MODE_CHECK) > 0) {

        printf("[Breakpoint encountered at %06X]\n", PC.A);
        debugger_is_on = 1;
    }

    if(debugger_is_on)
        do_debugger();
=======
    //Todo
    //check_breakpoints();

    if(debugger_is_on) 
	enter_debug_console();
>>>>>>> 89f4e9f6d5b00cf5ea8460112056537af8aac737
    
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
<<<<<<< HEAD
            //We'll set ACTUAL breakpoints after this
            //debugger_is_on = debugger_is_on ? 0 : breakpoints_are_enabled;
            //CPU_setTrace(debugger_is_on);
        }
=======

            debugger_is_on = debugger_is_on ? 0 : debugger_is_enabled;
	    //CPU_setTrace(debugger_is_on);
	}
>>>>>>> 89f4e9f6d5b00cf5ea8460112056537af8aac737

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

//TODO
int input(char* inbuf, int bufsz) {

    while(!getch());
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

    //static struct termios oldit, newit, oldot, newot;

    //Get args
    if(argc > 1) {
   
        int i;
<<<<<<< HEAD
        for(i = 1; i < argc; i++) {
        
            if(!strcmp(argv[i], "--debug"))
                    debugger_is_on = 1;
        }
=======
	for(i = 1; i < argc; i++) {
	
	    if(!strcmp(argv[i], "--enable-debugger"))
                debugger_is_enabled = 1;
	}
>>>>>>> 89f4e9f6d5b00cf5ea8460112056537af8aac737
    }

    debugger_is_on = debugger_is_enabled;

    CPU_reset();
<<<<<<< HEAD
    //CPU_setTrace(0);
=======
    //CPU_setTrace(debugger_is_on);
>>>>>>> 89f4e9f6d5b00cf5ea8460112056537af8aac737
    CPUEvent_initialize();
    CPU_setUpdatePeriod(1);

    //Set stdin to non-blocking
    fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);

    //Turn off line-at-a-time
    tcgetattr(STDIN_FILENO, &oldit);
    newit = oldit;
    newit.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &newit);

    tcgetattr(STDOUT_FILENO, &oldot);
    newot = oldot;
    newot.c_lflag &= ~(ICANON);
    tcsetattr(STDOUT, TCSANOW, &newot);

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

    //Reset console mode
    fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) & ~O_NONBLOCK);
    oldit.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldit);
    tcsetattr(STDOUT_FILENO, TCSANOW, &oldot);
 
    return 0;
}

