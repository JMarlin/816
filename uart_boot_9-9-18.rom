* = $8000

#define clk_count $00
#define spi_in $01
#define spi_out $02
#define out_buf $03
#define in_buf $04
#define out_count $05
#define txt_buffer $0200
#define incoming_ch $06
#define cmd_count $07
#define lcl_ptr $08
#define cmp_base $0A
#define temp_addr $0C
#define inbuf_count $0E
#define inbuf_ptr $0F
#define inbuf $0300
#define cmd_count $10
#define load_size $11
#define temp_reg $12
#define temp_ptr $13
#define long_ptr $15
#define asm_base_lptr $18
#define cmd_buffer $0400
#define MRA $7FF0  ;R/W
#define SRA $7FF1  ;R
#define CSRA $7FF1 ;W
#define MISR $7FF2 ;R
#define CRA $7FF2  ;W
#define RHRA $7FF3 ;R
#define THRA $7FF3 ;W
#define IPCR $7FF4 ;R
#define ACR  $7FF4 ;W
#define ISR  $7FF5 ;R
#define IMR  $7FF5 ;W
#define CTU  $7FF6 ;R/W
#define CTL  $7FF7 ;R/W
#define MRB  $7FF8 ;R/W
#define SRB  $7FF9 ;R
#define CSRB $7FF9 ;W
#define CRB  $7FFA ;W
#define RHRB $7FFB ;R
#define THRB $7FFB ;W
#define IVR  $7FFC ;R/W
#define IP   $7FFD ;R
#define OPCR $7FFD ;W
#define SCC  $7FFE ;R
#define SOPBC $7FFE ;W
#define STC  $7FFF ;R
#define COPBC $7FFF ;W
#define SR_TXRDY $04
#define SR_RXRDY $01
#define mrak_mask $01
#define mdat_mask $02
#define dout_pin_mask $01

START CLV       ;Clear overflow, disable emulation mode
      CLC
      XCE
      LDA #$01  ;Set stack pointer to $01FF (grows downward)
      XBA
      LDA #$FF
      TCS
      JMP MAINL ;go to the main application

;This is the location that the built-in assembler will default to assembling in
asm_init_base .byte $00, $05, $00

PROMT .asc ")) "
      .byte $0
ERRMSG .asc "ERR"
       .byte $0
OKMSG  .asc "OK"
       .byte $0
ENDLN .byte $A, $D, $0

#define command_count $06
cmd_str_table .word go_string
              .word ex_string
              .word st_string
              .word ad_string
              .word ld_string
              .word dm_string
cmd_ptr_table .word GOCM
              .word EXACM
              .word STOCM
              .word ADDRCM
              .word LOADCM
              .word DUMPCM
go_string   .asc "go"
              .byte $0
ex_string     .asc "exam"
              .byte $0
st_string     .asc "stor"
              .byte $0
ad_string     .asc "addr"
              .byte $0
ld_string     .asc "load"
              .byte $0
dm_string     .asc "dump"
              .byte $0

int_str .asc "INTERRUPT"
        .byte $0A, $00

INITUART
      PHA  ;Back up A
      LDA #$D0 
      STA CRA  ;exit standby, if required
      LDA #$20
      STA CRA  ;Reset receiver (flush FIFO, disable RX)
      LDA #$30
      STA CRA  ;Reset transmitter (flush FIFO, disable TX)
      LDA #$BB 
      STA CSRA ;Set CSRA to 1011 1011 (9600/9600)
      LDA #$10 
      STA CRA  ;Send reset MSR command to CRA
      LDA #$13 
      STA MRA  ;Set MR1A to 00010011 (No parity, even parity, 8 bits per char)
      LDA #$07 
      STA MRA  ;Set MR2A (auto) to 0000 0111 (Normal channel mode, don't disable TX via CTS, stop bit length = 1.000)
      LDA #$05 ;Send CRA command to enable TX and RX 
      STA CRA
      PLA  ;Restore A backup
      RTS

OUTCH 
      PHA ;Backup character to write
OCHLP LDA SRA ;Get current UART status
      AND #SR_TXRDY ;Check to see if THRA is ready for a character
      BEQ OCHLP ;Keep polling until TXRDY isn't zero
      PLA ;Restore the character to write
      STA THRA ;And put it in the output FIFO
      RTS

PRNTS PHY
      STA lcl_ptr
      XBA
      STA lcl_ptr + 1
      LDY #$00
      CLV
PRTOP LDA (lcl_ptr),Y 
      BEQ PRDN
      JSR OUTCH
      INY     ;Cleverly, this should force us to also exit if and when Y wraps
      BVC PRTOP
PRDN  PLY
      RTS

;PRSHX - Parse byte from ASCII hex digit to 4-bit value
;Arguments | A - ASCII hex digit
;Returns   | A - Value of digit
;          | Carry = set if parsing failed
PRSHX CMP #$30  ;If input less than '0'
      BCC PXFAL ;Fail, invalid character
      SBC #$30  ;Subtract '0'
      CMP #$A   ;If result is less than 0xA
      BCC PXEND ;Then it is our result
      CMP #$11  ;If the result is less than ('A' - '0')
      BCC PXFAL ;Fail, invalid character
      SBC #$7   ;Subtract ('A' - '0' - 0xA)
      CMP #$10  ;If result is less than 0x10
      BCC PXEND ;Then it is our result
      CMP #$2A  ;If result is less than 'a' - 0x30 - 0x7
      BCC PXFAL ;Fail, invalid character
      SBC #$20  ;Subtract ('a' - 'A')
      CMP #$10  ;If result is less than 0x10
      BCC PXEND ;Then it is our result, else fail through
PXFAL SEC
      RTS
PXEND CLC
      RTS

;PRSBT - Parse byte from ASCII string
;Arguments | S+5\S+4 - 16-bit local pointer to input string
;Returns   | S+3 = Parsed byte, output
;          | Carry = set if parsing failed
PRSBT PHY              ;(S is now +1)
      PHA              ;(S is now +2)
      LDA ($6,S)       ;Load the first byte from arg pointer (6 - 2 = 4)
      JSR PRSHX        ;Convert hex nybble to value (result in A)
      BCS PBEND        ;If conversion failed, exit
      ASL              ;Shift result into high-order nybble
      ASL
      ASL
      ASL
      STA $5,S         ;Store in stack output arg (5 - 2 = 3)
      LDY #$01         ;Use Y to index next character
      LDA ($6,S),Y     ;(6 - 2 = 4)
      JSR PRSHX        ;Convert hex nybble to value
      BCS PBEND        ;If conversion failed, exit
      AND #$0F         ;Make sure top nybble is cleared
      ORA $5,S         ;Insert previously calculated top nybble (5 - 2 = 3)
      STA $5,S         ;Write final value back to stack output arg (5 - 2 = 3)
      CLC              ;Make sure we don't flag a failure
PBEND PLA
      PLY
      RTS

;PRSAD - Parse 24-bit Address from ASCII string
;Arguments | S+7\S+6 = 16-bit local pointer to input string
;Returns   | S+5\S+4\S+3 = 24-bit parsed address from string, out
;          | Carry = set if parsing failed
PRSAD PHA       ;Backup register to be clobbered (S is now +1)
      LDA $8,S  ;Push pointer to character to parse onto stack (8 - 1 = 7)
      PHA       ;(S is now +2)
      LDA $8,S  ;(8 - 2 = 6)
      PHA       ;(S is now +3)
      PHA       ;Make space on stack for output value (S is now +4)
      JSR PRSBT ;Parse first two characters for bank
      BCS PAEP4 ;If function failed, return fail
      PLA       ;Stash value into output (S is now +3)
      STA $8,S  ;(8 - 3 = 5)
      LDA #$00  ;Push $0002 addend onto the stack
      PHA       ;(S is now +4)
      LDA #$02
      PHA       ;(S is now +5)
      JSR ADDW  ;Increment to the next two chars in the string (address is still on stack)
      PLA       ;Drop half of input addend, leaving room for output value (S is now +4) 
      JSR PRSBT ;Parse second byte 
      BCS PAEP4 ;If function failed, return fail
      PLA       ;Stash value into output (S is now +3)
      STA $7,S  ;(7 - 3 = 4)
      LDA #$00  ;Push $0002 addend onto the stack
      PHA       ;(S is now +4)
      LDA #$02
      PHA       ;(S is now +5)
      JSR ADDW  ;Increment to the next two chars in the string (address is still on stack)
      PLA       ;Drop half of input addend, leaving room for output value (S is now +4) 
      JSR PRSBT ;Parse third byte 
      BCS PAEP4 ;If function failed, return fail
      PLA       ;Stash value into output (S is now +3)
      STA $6,S  ;(6 - 3 = 4)
      CLC       ;Exit OK
      BRA PAEP3
PAEP4 PLA       
PAEP3 PLA  
      PLA     
      PLA
      RTS

;NB2HX - Convert a nybble to ASCII hex digit
;Arguments | A = Low nybble contains input value
;Returns   | A = ASCII digit
NB2HX AND #$0F ;Clear top nybble
      CLC 
      ADC #$30 ;Add '0'
      CMP #$3A ;If value is less than '9' + 1, return the value
      BCC NHEND
      ADC #$6  ;Otherwise, add 'A' - '9'
NHEND RTS

;PRTBT - Print a byte value as ASCII hex
;Arguments | A = Value to print
;Returns   | nothing
PRTBT PHA        ;Make a second copy for later
      LSR        ;Get the high-order nybble first
      LSR
      LSR
      LSR
      JSR NB2HX  ;Convert the value back to ASCII hex
      JSR OUTCH ;Print the converted value
      PLA        ;Get value again
      JSR NB2HX  ;Convert
      JSR OUTCH ;Print
      RTS

;Printline - Print a newline to the console
;Args - None
PRTSN PHA
      XBA
      PHA
      LDA #ENDLN/256
      XBA
      LDA #ENDLN&255
      JSR PRNTS
      PLA
      XBA
      PLA
      RTS

;AddWord - adds first word on stack to second word on stack
;NOTE - We should really just go whole-hog with 16-bit mode, here, but whatever
;Args - S+6/S+5 = Value to be added to, output
;     - S+4/S+3 = Value to be added
ADDW CLC
     LDA $5,S
     ADC $3,S
     STA $5,S
     LDA $6,S
     ADC $4,S
     STA $6,S
     RTS

;AddLong - adds first 24-bit long on stack to second 24-bit long on stack
;Args - S+8/S+7/S+6 = Value to be added to, output
;     - S+5/S+4/S+3 = Value to be added
ADDL CLC
     LDA $6,S
     ADC $3,S
     STA $6,S
     LDA $7,S
     ADC $4,S
     STA $7,S
     LDA $8,S
     ADC $5,S
     STA $8,S
     RTS

;ADD Address - Adds A to three-byte long address on stack
ADDA PHA
     CLC
     ADC $4,S
     STA $4,S
     LDA #$00
     ADC $5,S
     STA $5,S
     LDA #$00
     ADC $6,S
     STA $6,S
     PLA
     RTS

;Increment pointer - Adds one to the global base pointer
INPTR
      PHA
      LDA asm_base_lptr+2
      PHA 
      LDA asm_base_lptr+1
      PHA
      LDA asm_base_lptr
      PHA
      LDA #$01
      JSR ADDA
      PLA
      STA asm_base_lptr
      PLA
      STA asm_base_lptr+1
      PLA
      STA asm_base_lptr+2
      PLA
      RTS

;Console command 'dump' - dump the contents of memory, beginning at the current
;global pointer location and spanning the number of bytes passed by the user,
;to the console
;Format - 'dump AAAAAA' where AAAAAA is the 24-bit length of bytes to dump
DUMPCM
      PHA       ;Backup clobbered registers (S is +1)
      XBA       
      PHA       ;(S is +2)
      PHX       ;(S is +3)
      PHY       ;(S is +4)
      PHA       ;Push space for local vars (S is +5)
      PHA       ;(S is +6)
      PHA       ;(S is +7)
      PHA       ;Put B/A string pointer onto the stack (addw arg 1) (S is +8)
      XBA       
      PHA       ;(S is +9)
      LDX #$00  ;Push 0x0004 addend (addw arg 2)
      PHX       ;(S is +10)
      LDX #$04
      PHX       ;(S is +11)
      JSR ADDW  ;Call ADDW to fast-forward past the 'load' command chars
      PLX       ;Drop the addend arg from the stack, (S is +10)
      PLX       ;leaving the incremented address (S is +9)
      JSR SKPWH ;Fast-forward past any whitespace
      LDA (1,S) ;Check to make sure didn't hit end of string
      BEQ DMEP2 ;If we did, exit fail
      PHA       ;Push space for return addrlen (S is +10)
      PHA       ;(S is +11)
      PHA       ;(S is +12)
      JSR PRSAD ;Read 24-bit hex length
      BCS DMEP5 ;If failed, exit fail
      PLA       ;Get parsed value (S is +11)
      STA 5,S   ;Stash it into local variable space (11 - 5 = 6 = 2nd local + 4 regs)
      PLA       ;(S is +10)
      STA 5,S   ;(10 - 5 = 5 = 1st local + 4 regs)
      PLA       ;(S is +9)
      STA 5,S   ;(9 - 5 = 4 = 0th local + 4 regs)
      LDX #$00  ;Push 0x0006 addend (addw arg 2)
      PHX       ;(S is +10)
      LDX #$06
      PHX       ;(S is +11)
      JSR ADDW  ;Call ADDW to fast-forward past the parsed addrlen
      PLX       ;Drop the addend arg from the stack, (S is +10)
      PLX       ;leaving the incremented address (S is +9)
      JSR SKPWH ;Skip to end of string
      LDA (1,S) ;Check to make sure we hit end of string
      BNE DMEP2 ;Not end of string, fail
      PLA        ;Drop unneeded string pointer (S is +8)
      PLA        ;(S is +7)
      LDA asm_base_lptr+2 ;Push arg 1 of ADDL onto stack 
      PHA        ;(S is +8)
      LDA asm_base_lptr+1
      PHA        ;(S is +9)
      LDA asm_base_lptr
      PHA        ;(S is +10)
      LDA 6,S    ;Push arg 2 of ADDL onto stack (user's length to dump) (10 - 6 = 4 = 0th local + 4regs)
      PHA        ;(S is +11)
      LDA 6,S    ;(11 - 6 = 5 = 1st local + 4regs)
      PHA        ;(S is +12)
      LDA 6,S    ;(12 - 6 = 6 = 2nd local + 4regs)
      PHA        ;(S is +13)
      JSR ADDL   ;Add the read length to the initial address to get the terminal address
      PLA        ;Dump the addend (S is +12)
      PLA        ;(S is +11)
      PLA        ;(S is +10) (we leave the terminal address on the stack)
DMLOOP           ;Console read loop begins here
      LDA 1,S    ;Compare terminal address on stack to global pointer
      CMP asm_base_lptr
      BNE DMCNT
      LDA 2,S 
      CMP asm_base_lptr+1
      BNE DMCNT
      LDA 3,S 
      CMP asm_base_lptr+2
      BNE DMCNT
      BRA DMSUC  ;If all three byte positions match, break the loop
DMCNT 
      JSR EXABT  ;Output the byte at the current global address and increment the address
      LDA #$20   ;Add a space to pretty things up very slightly
      JSR OUTCH
      BRA DMLOOP ;And loop
DMSUC
      JSR PRTSN  ;Add a newline
      PLA        ;Clear terminal count value from the stack (S is +9)
      PLA        ;(S is +8)
      PLA        ;(S is +7)
      CLC
      BRA DMEND  ;Exit success
DMEP5 PLA        ;Drop unneeded stack values
      PLA
      PLA
DMEP2 PLA
      PLA
DMFAL SEC        ;Carry bit indicates command failure
DMEND PLX        ;Clear local variable area
      PLX
      PLX
      PLY        ;Restore caller register values
      PLX
      PLA
      XBA
      PLA
      RTS        ;Return to caller

;Console command 'load' - Load a stream of data from the console memory starting at the global
;pointer location. Loading ends when the console receives a null character
;Format - 'load'
LOADCM
      PHA       ;Backup clobbered registers (S is +1)
      XBA       
      PHA       ;(S is +2)
      PHX       ;(S is +3)
      PHY       ;(S is +4)
      PHA       ;Push space for local vars (S is +5)
      PHA       ;(S is +6)
      PHA       ;Put B/A string pointer onto the stack (addw arg 1) (S is +7)
      XBA       
      PHA       ;(S is +8)
      LDX #$00  ;Push 0x0004 addend (addw arg 2)
      PHX       ;(S is +9)
      LDX #$04
      PHX       ;(S is +10)
      JSR ADDW  ;Call ADDW to fast-forward past the 'load' command chars
      PLX       ;Drop the addend arg from the stack, (S is +9)
      PLX       ;leaving the incremented address (S is +8)
      JSR SKPWH ;Fast-forward past any whitespace
      LDA (1,S) ;Check to make sure we hit end of string
      BNE LDEP2 ;If not, exit fail
      PLA        ;Drop unneeded string pointer (S is +7)
      PLA        ;(S is +6)
LDLOOP          ;Console read loop begins here
      JSR RDCHR ;Read ms nybble into A
      BEQ LDSUC  ;If we reached the end (null), exit success
      CMP #$21   ;If it is 0x20 or less...
      BCC LDLOOP ;Then keep reading so as to skip any whitespace
      STA cmd_buffer ;We coopt the command buffer as temp storage because we are monsters
      JSR RDCHR ;Read the ls nybble into A
      STA cmd_buffer+1 ;Store into the next location
      LDA #$00
      STA cmd_buffer+2 ;Add terminating character for good measure
      LDA #cmd_buffer/256 ;Push the cmd buffer address onto the stack
      PHA       ;(S is +7)
      LDA #cmd_buffer&255
      PHA       ;(S is +8)
      JSR STOBT ;Consume and store the ensuing byte value, incrementing the global pointer
      BCS LDEP2 ;If store function failed, return fail
      PLA        ;Remove the pushed pointer
      PLA
      LDA #$2E   ;Write a '.' for each parsed and loaded byte
      JSR OUTCH
      BRA LDLOOP ;Otherwise, loop again
LDSUC
      CLC
      BRA LDEND  ;Exit success
LDEP2 PLA        ;Drop unneeded stack values
      PLA
LDFAL SEC        ;Carry bit indicates command failure
LDEND PLX        ;Clear local variable area
      PLX
      PLY        ;Restore caller register values
      PLX
      PLA
      XBA
      PLA
      RTS        ;Return to caller

;Console command 'stor' - Store a byte to memory
;Format - 'stor AA' store AA at current global pointer location
STOCM PHA       ;Backup clobbered registers (S is +1)
      XBA       
      PHA       ;(S is +2)
      PHX       ;(S is +3)
      PHY       ;(S is +4)
      PHA       ;Push space for local vars (S is +5)
      PHA       ;Put B/A string pointer onto the stack (addw arg 1) (S is +6)
      XBA       
      PHA       ;(S is +7)
      LDX #$00  ;Push 0x0004 addend (addw arg 2)
      PHX       ;(S is +8)
      LDX #$04
      PHX       ;(S is +9)
      JSR ADDW  ;Call ADDW to fast-forward past the 'stor' command chars
      PLX       ;Drop the addend arg from the stack, (S is +8)
      PLX       ;leaving the incremented address (S is +7)
      JSR STOBT ;Consume and store the ensuing byte value
      BCS STEP2 ;If store function failed, return fail
      PLA        ;Drop unneeded string pointer (S is +6)
      PLA        ;(S is +5)
      BRA STEND  ;Exit success
STEP2 PLA        ;Drop unneeded stack values
      PLA
STFAL SEC        ;Carry bit indicates command failure
STEND PLX        ;Clear local variable area
      PLY        ;Restore caller register values
      PLX
      PLA
      XBA
      PLA
      RTS        ;Return to caller

;STore ByTe - read a hex byte from word-ptr string on stack, store the
;             result at the global pointer location, and inc the
;             global pointer
STOBT
      PHA       ;Back-up regs (S is +1)
      PHA       ;Create local var space for a single variable (S is +2)
      LDA 6,S   ;Load string pointer from our args stack into the arg stack to be passed
      PHA       ;(S is +3) 
      LDA 6,S              
      PHA       ;(S is +4)
      JSR SKPWH ;Fast-forward past any whitespace characters
      LDA (1,S) ;Load current character
      BEQ SBEP2 ;If it's a null char, die
      PHA       ;Push space for return byte (S is +5)
      CLC
      JSR PRSBT ;Get byte value
      BCS SBEP3 ;If failed, exit fail
      PLA       ;Get parsed byte value (S is +4)
      STA 3,S   ;Stash it into local variable space (4 - 3 = 1 = 0th local + 1 reg)
      LDX #$00  ;Push 0x0002 addend (addw arg 2)
      PHX       ;(S is +5)
      LDX #$02
      PHX       ;(S is +6)
      JSR ADDW  ;Call ADDW to fast-forward past the parsed byte
      PLX       ;Drop the addend arg from the stack, (S is +5)
      PLX       ;leaving the incremented address (S is +4)
      JSR SKPWH ;Skip to end of string
      LDA (1,S) ;Check to make sure we hit end of string
      BNE SBEP2 ;If not, exit fail
      LDA 3,S   ;Retrieve parsed byte from locals
      STA [asm_base_lptr] ;Set value at global address pointer
      JSR INPTR ;Increment the pointer
      PLA       ;Return updated string pointer to caller (S is +3)
      STA 6,S   
      PLA       ;(S is +2)
      STA 6,S   
      CLC
      BRA SBEND
SBEP3 PLA  ;Drop unneeded stack values
SBEP2 PLA  
      PLA
      SEC  ;Carry indicates fail
SBEND PLA  ;Drop local var stack space (S is +1)
      PLA  ;Restore A register (S is +0)
      RTS  ;Return

;Console command 'go' - GO to the current address and start executing
;Format - 'go' no arguments
GOCM
    PHA       ;Backup A value
    XBA
    PHA
    PHA       ;Put B/A string pointer onto the stack (addw arg 1) (S is +1)
    XBA       
    PHA       ;(S is +2)
    LDX #$00  ;Push 0x0002 addend (addw arg 2)
    PHX       ;(S is +3)
    LDX #$02
    PHX       ;(S is +4)
    JSR ADDW  ;Call ADDW to fast-forward past the 'go' command chars
    PLX       ;Drop the addend arg from the stack, (S is +3)
    PLX       ;leaving the incremented address (S is +2)
    JSR SKPWH ;Fast-forward past any whitespace characters
    LDA (1,S) ;Check current character
    BNE GOE   ;If it wasn't the end of the string, exit fail
    JMP [asm_base_lptr]
    BRA GOEND ;This will never actually return
GOE 
    SEC
GOEND 
    PLA       ;Dump unneeded stack values
    PLA
    RTS 


;Console command 'addr' - set the current storage ADDRess
;Format - 'addr AAAAAA' where 'A' is a hex digit in a 
;         24-bit address
ADDRCM 
      PHA       ;Backup clobbered registers (S is +1)
      XBA       
      PHA       ;(S is +2)
      PHX       ;(S is +3)
      PHY       ;(S is +4)
      PHA       ;Create variable space on the stack (S is +5)
      PHA       ;(S is +6)
      PHA       ;(S is +7)
      PHA       ;Put B/A string pointer onto the stack (addw arg 1) (S is +8)
      XBA       
      PHA       ;(S is +9)
      LDX #$00  ;Push 0x0004 addend (addw arg 2)
      PHX       ;(S is +10)
      LDX #$04
      PHX       ;(S is +11)
      JSR ADDW  ;Call ADDW to fast-forward past the 'addr' command chars
      PLX       ;Drop the addend arg from the stack, (S is +10)
      PLX       ;leaving the incremented address (S is +9)
tsbk2 JSR SKPWH ;Fast-forward past any whitespace characters
      PHX       ;Push room for parsed address retval (S is +10)
      PHX       ;(S is +11)
      PHX       ;(S is +12)
      JSR PRSAD ;Parse the address text at the pointer
      BCS ADEP5 ;If the conversion failed, exit fail
      PLA       ;Otherwise, stash parsed value into local variable area (S is +11)
      STA 5,S   ;(11 - 5 = 6 = 2nd local byte + 4 regs)
      PLA       ;(S is +10)
      STA 5,S   ;(10 - 5 = 5 = 1st local byte + 4 regs)
      PLA       ;(S is +9)
      STA 5,S   ;(9 - 5 = 4 = 0th local byte + 4 regs) now the string pointer is on the stack again
      LDX #$00  ;Push 0x0006 addend (addw arg 2)
      PHX       ;(S is +10)
      LDX #$06
      PHX       ;(S is +11)
      JSR ADDW  ;Call ADDW to fast-forward past the parsed digits
      PLX       ;Drop the addend arg from the stack, (S is +10)
      PLX       ;leaving the incremented address (S is +9)
      JSR SKPWH ;Skip to what SHOULD be the end of the string
tstbk LDA (1,S) ;Check next non-white byte
      BNE ADEP2 ;If it wasn't zero, exit fail
      LDA 3,S   ;Store parsed value to global pointer (9 - 3 = 6 = 2nd local byte + 4 regs)
      STA asm_base_lptr
      LDA 4,S   ;(9 - 2 = 5 = 1st local byte + 4 regs)
      STA asm_base_lptr+1
      LDA 5,S   ;(9 - 3 = 4 = 0th local byte + 4 regs)
      STA asm_base_lptr+2
      PLA       ;Drop string pointer (S is +8)
      PLA       ;(S is +7)
      CLC
      BRA ADEND  ;Exit success
ADEP5 PLX        ;Dump unneeded stack values
      PLX
      PLX
ADEP2 PLX
      PLX
ADFAL SEC        ;Carry bit indicates command failure
ADEND PLA        ;Dump local variable space
      PLA 
      PLA
      PLY        ;Restore caller register values
      PLX
      PLA
      XBA
      PLA
      RTS        ;Return to caller 


;Console command 'exam' - eXamine a byte of memory
;Format - 'exam AAAAAA' where 'A' is a hex digit in a 
;         24-bit address
EXACM PHA       ;Backup clobbered registers (S is +1)
      XBA       
      PHA       ;(S is +2)
      PHX       ;(S is +3)
      PHY       ;(S is +4)
      PHA       ;Put B/A string pointer onto the stack (addw arg 1) (S is +5)
      XBA       
      PHA       ;(S is +6)
      LDX #$00  ;Push 0x0004 addend (addw arg 2)
      PHX       ;(S is +7)
      LDX #$04
      PHX       ;(S is +8)
      JSR ADDW  ;Call ADDW to fast-forward past the 'exam' command chars
      PLX       ;Drop the addend arg from the stack, (S is +7)
      PLX       ;leaving the incremented address (S is +6)
      JSR SKPWH ;Fast-forward past any whitespace characters
      LDA (1,S) ;Load current character
      BNE EXEP2 ;If it's not a null char, die
      JSR EXABT ;Print the byte at the current global pointer location and advance
      JSR PRTSN ;And a newline
      PLA       ;Drop string pointer (S is +5)
      PLA       ;(S is +4)
      CLC
      BRA EXEND  ;Exit success
EXEP2 PLX        ;Dump unneeded stack values
      PLX
EXFAL SEC        ;Carry bit indicates command failure
EXEND PLY        ;Restore caller register values
      PLX
      PLA
      XBA
      PLA
      RTS        ;Return to caller

EXABT
      PHA       ;Backup A value
      LDA [asm_base_lptr] ;Get value at global address pointer
      JSR PRTBT ;And print it
      JSR INPTR ;Increment the global pointer
      PLA       ;Restore A value
      RTS

;Advance the local pointer on the stack until it no longer
;points to a whitespace character
;Args = S+4/S+3 - Pointer to be modified
SKPWH PHA      ;Back up previous A value (S is now +1)
SWTOP LDA ($4,S) ;Check byte value at pointer (4 - 1 = 3)
      BEQ SWEXI  ;Zero/end-of-string is a special exit case
      CMP #$21   ;If it's >= 0x21, non whitespace
      BCS SWEXI  ;So we can also exit (BCS = BGE)
      LDA $5,S   ;Get our argument and push it onto the stack for ADDW (5 - 1 = 4)
      PHA        ;(S is now +2)
      LDA $5,S   ;(5 - 2 = 3)
      PHA        ;(S is now +3)
      LDA #$00   ;Push $0001 addend
      PHA        ;(S is now +4)
      LDA #$01   
      PHA        ;(S is now +5)
      JSR ADDW   ;And add the pointer value we pushed to it
      PLA        ;Clear the addend argument, leaving the result (S is now +4)
      PLA        ;(S is now +3)
      PLA        ;Put the updated value back into our args area (S is now +2)
      STA $5,S   ;(5 - 2 = 3)
      PLA        ;(S is now +1)
      STA $5,S   ;(5 - 1 = 4)
      BRA SWTOP
SWEXI PLA        ;Restore original A value
      RTS

STCMP PHY
      STA lcl_ptr
      XBA
      STA lcl_ptr + 1
      STY cmp_base
      STX cmp_base + 1
      LDY #$00
      CLV
      SEC
SCTOP LDA (lcl_ptr),Y
      CMP (cmp_base),Y
      BNE CBLK ;If they don't match, double-check for end-of-string vs space
      CMP #$00
      BEQ SCDN ;If both are zero, exit success
      INY      ;Cleverly, this should force us to also exit if and when Y wraps
      BVC SCTOP
CBLK  XBA 
      LDA (cmp_base),Y
      CMP #00  ;Compare base string to null character
      BNE CSWP ;Not null, go check the other way around
      XBA
      CMP #$20 ;base = 0 && cmp = ' '?
      BNE SCFL ;   no, fail
      BVC SCDN ;   yes, success
CSWP  CMP #$20  ;Switch the comparison (check blank in cmp string)
      BNE SCFL ;   compare string isn't null or space, fail
      XBA
      CMP #$00 ;cmp = 0 && base = ' '?
      BEQ SCDN ;   yes, success
               ;   no, fail through
SCFL  CLC
SCDN  PLY
      RTS

RDCHR
      LDA SRA
      AND #SR_RXRDY
      BEQ RDCHR
      LDA RHRA
      RTS

PARSE PHX
      PHY
      LDX cmd_count
      CPX #$FF
      BEQ CHECK 
      CMP #$0A
      BEQ CHECKPA
      CMP #$0D
      BEQ CHECKPD
      STA cmd_buffer,X
      INX 
      STX cmd_count
      BRA PRSDN
CHECKPA PHA
        LDA #$0D
        JSR OUTCH
        PLA
        BRA CHECK
CHECKPD PHA
        LDA #$0A
        JSR OUTCH
        PLA
CHECK PHA
      LDA #$00
      STA cmd_buffer,X ;We'll parse this some day
      LDX #>cmd_buffer
      LDY #$00
      STY cmd_count
CMPLP
      LDY cmd_count
      CPY #command_count*2
      BEQ RESER
      LDA cmd_str_table,Y
      XBA
      INY
      LDA cmd_str_table,Y
      INY
      XBA
      STY cmd_count
      LDY #<cmd_buffer
      JSR STCMP
      BCC CMPLP
      LDA cmd_count
      SBC #$02
      TAX
      LDA #cmd_buffer/256
      XBA
      LDA #cmd_buffer&256
      JSR (cmd_ptr_table,X)
      BCC RESOK
RESER LDA #ERRMSG/256
      XBA
      LDA #ERRMSG&255
      BRA PRPMT
RESOK LDA #OKMSG/256
      XBA
      LDA #OKMSG&255
PRPMT JSR PRNTS
      JSR PRTSN
      JSR PRTPMT
      LDY #$00
      STY cmd_buffer
      STY cmd_count
      PLA
PRSDN PLY
      PLX
      RTS

PRTPMT
      PHA
      XBA
      PHA
      LDA asm_base_lptr+2
      JSR PRTBT
      LDA asm_base_lptr+1
      JSR PRTBT
      LDA asm_base_lptr
      JSR PRTBT
      LDA #PROMT/256
      XBA
      LDA #PROMT&255
      JSR PRNTS       ;print command prompt
      PLA 
      XBA
      PLA
      RTS

MAINL
      LDA #$00
      STA out_count   ;init output buffer
      STA cmd_count
      STA inbuf_count
      STA inbuf_ptr
      JSR INITUART    ;Init the UART
      LDX #$03        ;Init assembler base pointer
NXTAB DEX
      LDA asm_init_base,X
      STA asm_base_lptr,X
      CPX #$00
      BNE NXTAB
      ;PHP             ;Clear IRQ Disable bit
      ;PLA             ;Disabled until we start playing with interrupt servicing on the DUART
      ;AND #$FB
      ;PHA
      ;PLP
      JSR PRTPMT
CKCHR JSR RDCHR      ;Get the next character from the incoming ring buffer
      CMP #$00
      BEQ CKCHR       ;If the character was zero, try reading again
      JSR OUTCH
      JSR PARSE   
SKPAD BRA CKCHR

IRQH
;      LDA in_reg ;Make sure that the input isn't spurious
;      AND #mrak_mask
;      BEQ IOVR   ;Die if the input isn't actually high
;      LDA #$7F
;DLMR  DEC        ;Delay 128 times to ensure MRAK stays high
;      BEQ CHK2
;      BRA DLMR
;CHK2  LDA in_reg ;Check it again
;      PHA
;      AND #mrak_mask
;      BEQ IOVR   ;And die if it went low since the beginning of the delay
;      ASL $shift_reg  ;Shift the value accumulator (TODO - define this address)
;      PLA 
;      AND #mdat_mask ;Get the sampled MDAT value
;      BEQ NOBT       ;Skip setting the low bit
;      INC $shift_reg ;Set the low bit
;NOBT  STA 
;      LDA #mrak_mask ;Raise CRAK handshake signal to indicate that we've finisehd sampling the bit
;      STA out_reg
;WTMR  LDA in_reg     ;Wait for MRAK to go low, indicating that the other device acknowledged our handshake
;      AND #mrak_mask 
;      BEQ MRLO
;      BRA WTMR
;MRLO  LDA #$00       ;Handshaking complete, lower our CRAk
;      ;TODO -- increase bit count, if ==8 then store in input buffer area and reset bit count
;IOVR
      RTI

CDEND

.dsb $FFDE - *,$00

;===========| FFE6-FFED Application Utils |===========
;To be called from application software, ex.
;    jsl $00FFEA    <- print string to console
JSR OUTCH   ;FFDE,DF,E0 -- print character utility
RTL         ;FFE1
JSR RDCHR   ;FFE2,E3,E4 -- get character utility
RTL         ;FFE5
JMP $8000   ;FFE6,E7,E8 -- re-enter monitor utility
.byte $00   ;FFE9 -- we keep the entry points dword-aligned
JSR PRNTS   ;FFEA,EB,EC -- print utility
RTL         ;FFED

;===========| FFEE- Interrupt/Reset Vectors |===========
.word IRQH  ;FFEE,EF -- IRQB vector
.word 0     ;FFF0,F1
.word 0     ;FFF2,F3
.word 0     ;FFF4,F5
.word 0     ;FFF6,F7
.word 0     ;FFF8,F9
.word 0     ;FFFA,FB
.word START ;FFFC,FD
.byte $00
.byte $AA
