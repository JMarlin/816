extern "C" {
	#include "cpu.h"
}

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <string.h>
#include <signal.h>
#include <conio.h>
#include "system_controller_cpld.h"

#define RAM_SIZE  16*1024
#define ROM_SIZE  32*1024
#define EXIT_DEBUGGER -1
#define QUIT_APPLICATION -2
#define INVALID_COMMAND 0

#define DBG_CMD_PROCEED 1
#define DBG_CMD_REMAIN 2

//struct termios oldit, newit, oldot, newot;
unsigned char* REN_rombuf;
unsigned char* REN_rambuf;
unsigned char* send_file_data = (unsigned char*)0;
int romsize;
int send_file_size = 0;
int send_file_index = 0;
byte input_register = 0;
byte output_register;
byte uart_register = 0;
byte sending_file = 0;
byte test_value = 0;
int tx_requested = 0;
byte spi_in = 0;
byte spi_out = 0;
int shifter = 0;
word32 last_time = 0;
word32 last_poll = 0;
int debugger_is_on = 0;
int debugger_is_enabled = 0;
int interrupts_enabled = 1;
int step_over_addr = 0xFFFFFFFF;
SystemControllerCPLD* system_controller;

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

//int kbhit();
//int getch();
void spi_print();

void add_IRQ(unsigned int mask) {

    if(interrupts_enabled)
        CPU_addIRQ(mask);
}

int read_line(char* buffer, unsigned int buf_size) {

    char temp_char = 0;
    unsigned int char_count = 0;

    while(1) {
    
        //while(!kbhit());

        temp_char = getchar() & 0xFF;
        buffer[char_count] = temp_char;
        //printf("%c", temp_char); fflush(stdout);

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
#define BP_MODE_CLEAR_ALL 4

int breakpoint_check(int address, int mode) {

    static unsigned int bp_count = 0;
    static int* bp_entry = (int*)0;

    int found_index = -1;
    int i;

    if(mode == BP_MODE_COUNT) { //Mode 2 == get entry count 
        
        return (int)bp_count;
    }

    if(address > 0xFFFFFF || address < 0)
        return -1; //Bad address 

    if(mode == BP_MODE_GET) { //Mode 3 == get entry at index  

        if(address >= bp_count)
            return 0; //Address not found

        return (int)bp_entry[address];
    }

    if(mode == BP_MODE_CLEAR_ALL) {

        bp_count = 0;

        if(bp_entry)
            free(bp_entry);

        bp_entry = (int*)0;
    }

    for(i = 0; i < bp_count; i++)
        if(bp_entry[i] == address) {
            
            found_index = i;
            break;
        }

    if(found_index == -1 && mode == BP_MODE_INSERT) { //mode 1 == insert breakpoint

        bp_entry = (int*)realloc((void*)bp_entry, sizeof(int) * (bp_count + 1));
        bp_entry[bp_count++] = address;
    }

    if(found_index != -1 && mode == BP_MODE_REMOVE) { //mode -1 == delete breakpoint

        for(i = found_index; i < bp_count - 1; i++)
            bp_entry[found_index] = bp_entry[found_index + 1];

        bp_count--;
    }

    return found_index != -1;
}

int debug_command_break(int argc, char* argv[]) {

    if(argc != 2 && argc != 1) {

        printf("Wrong number of arguments.\nUsage: break <24-bit-hex-address | clear>\n");
        fflush(stdout);

        return DBG_CMD_REMAIN;
    }

    if(argc == 1) {

        int i;

        for(i = 0; i < breakpoint_check(0, BP_MODE_COUNT); i++) {

            printf("    [%d] 0x%06X\n", i, breakpoint_check(i, BP_MODE_GET));
            fflush(stdout);
        }
    } else if(!strcmp(argv[1], "clear")) {

	breakpoint_check(0, BP_MODE_CLEAR_ALL);
       
        printf("Breakpoints cleared\n");
        fflush(stdout);
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
                breakpoint_check(address, BP_MODE_REMOVE);
                printf("Breakpoint at %06X removed\n", address);
                fflush(stdout);
                break;
        }
    }

    return DBG_CMD_REMAIN;
}

int debug_command_toggleint(int argc, char* argv[]) {

    if(argc != 1) {

        printf("toggleint takes no arguments\n");
        fflush(stdout);
        return DBG_CMD_REMAIN;
    }

    interrupts_enabled = !interrupts_enabled;

    printf("interrupts are now %s\n", interrupts_enabled ? "ON" : "OFF");
    fflush(stdout);

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

int debug_command_exit(int argc, char* argv[]) {

	CPU_quit();
	return DBG_CMD_PROCEED;
}

int debug_command_key(int argc, char* argv[]) {

	input_register = '!';
	return DBG_CMD_REMAIN;
}

int debug_command_send(int argc, char* argv[]) {

	if(argc != 2) {
		
		printf("Usage: send <file to send>\n");
		return DBG_CMD_REMAIN;
	}

	send_file_data = (unsigned char*)malloc(send_file_size);
	if (!system_controller->StartSendFile(argv[1]))
		return DBG_CMD_REMAIN;

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

	int i;

#define COMMAND_COUNT 9
    char* command_entry[COMMAND_COUNT] = {
        "step",
        "run",
        "break",
        "args",
        "exam",
        "toggleint",
		"exit",
		"key",
        "send"
    };
    DebugCommandFunction command_function[COMMAND_COUNT] = {
        debug_command_step,
        debug_command_run,
        debug_command_break,
        debug_command_args,
        debug_command_exam,
        debug_command_toggleint,
		debug_command_exit,
		debug_command_key,
        debug_command_send
    };

    int argc;
    int retval = 0;
    char** argv = string_to_argv(command_buffer, &argc);

    if(argc == 0)
        return 0;

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
    }
}

void EMUL_hardwareUpdate(word32 timestamp) {

    static int oldE = 0;
    static double old_timestamp = 0;
    static double old_time = 0;
    static double timef = 0;
    static byte tx_buffer, rx_buffer;
    
    //struct timespec t;
    double clocks_elapsed;
    double time_elapsed;
    static long old_nsecs, new_nsecs, nsec_diff;
    static int i_count = 0;

    if(breakpoint_check(PC.A, BP_MODE_CHECK) > 0) {

        printf("[Breakpoint encountered at %06X]\n", PC.A);
        fflush(stdout);
        debugger_is_on = 1;
    }

	debugger_is_on = debugger_is_on || system_controller->Refresh(timestamp);

    if(debugger_is_on)
        do_debugger();
   

    //I want to do this based on time and not instruction count in the future
    //May even be possible to put this into a timer thread so that the interrupt
    //in the virtualized system is functionally mapped to an interrupt in the
    //host machine
    //if(i_count++ > 5000000) {

        //i_count = 0;
        //printf("[Interrupt fired]\n"); 
        //fflush(stdout);
        //CPU_addIRQ(1);
    //}

    //CPU emulation throttling
    //while(1) {
    //
    //    clock_gettime(CLOCK_REALTIME, &t); 
    //    new_nsecs = t.tv_nsec;
    //
    //    if(new_nsecs < old_nsecs)
    //        nsec_diff = (new_nsecs + 1000000000) - old_nsecs;
    //    else
    //        nsec_diff = new_nsecs - old_nsecs;
    //    
    //    if(nsec_diff >= 200) {
	//
    //        break;
    //    }
    //}

    //old_nsecs = new_nsecs;
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
}

byte MEM_readMem(word32 address, word32 timestamp, word32 emulFlags) {

    byte b = 0;

	system_controller->TryReadByte(address, timestamp, emulFlags, b);

    return b;
}

void MEM_writeMem(word32 address, byte b, word32 timestamp) {

	system_controller->TryWriteByte(address, timestamp, b);
}

#ifndef _WIN32 

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

#endif

//TODO
int input(char* inbuf, int bufsz) {

    getchar();
	return 0;
}

void int_handler(int value) {

    //if(debugger_is_on) {

    //    exit(0);
    //}

    debugger_is_on = 1;
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

    //Get args
    if(argc > 1) {
   
        int i;

        for(i = 1; i < argc; i++) {
        
            if(!strcmp(argv[i], "--debug"))
                    debugger_is_on = 1;
        }
    }

    //debugger_is_on = debugger_is_enabled;

    CPU_reset();
    //CPU_setTrace(0)
    CPUEvent_initialize();
    CPU_setUpdatePeriod(1);

	system_controller = new SystemControllerCPLD("boot.rom");

	if (!system_controller->GetInitOk()) {

		printf("Couldn't bring up the system. Exiting.\n");
		return 0;
	}

    printf("Starting execution\n");
    signal(SIGINT, int_handler);
    CPU_run();
 
    return 0;
}

