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
      JMP @MAINL ;go to the main application

;This is the location that the built-in assembler will default to assembling in
asm_init_base .byte $00, $05, $00

PROMT .asc ")) "
      .byte $0
LDMSG .asc "RDY"
      .byte $0
ERRMSG .asc "ERR"
       .byte $0
OKMSG  .asc "OK"
       .byte $0
ENDLN .byte $A, $D, $0

#define command_count $04
cmd_str_table .word load_string
              .word ex_string
              .word st_string
              .word ad_string
cmd_ptr_table .word LODCM
              .word EXACM
              .word STOCM
              .word ADDRCM
load_string   .asc "load"
              .byte $0
ex_string     .asc "exam"
              .byte $0
st_string     .asc "stor"
              .byte $0
ad_string     .asc "addr"
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
      RTL

PRNTS PHY
      STA lcl_ptr
      XBA
      STA lcl_ptr + 1
      LDY #$00
      CLV
PRTOP LDA (lcl_ptr),Y 
      BEQ PRDN
      JSR @OUTCH
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
      JSR @OUTCH ;Print the converted value
      PLA        ;Get value again
      JSR NB2HX  ;Convert
      JSR @OUTCH ;Print
      RTS

;Printline - Print a newline to the console
;Args - None
PRTLN PHA
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

;AddWord - adds word on stack into AB
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

;AddLongaddress - Adds A to three-byte long address on stack
ADDL PHA
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

;Console command 'stor' - Store a byte to memory
;Format - 'stor AAAAAA BB' where 'A' is a hex digit
;         in a 24-bit address and 'B' is a hex digit
;         in a 8-bit value
STOCM PHA  ;Backup registers to be clobbered (S is +1)
      XBA
      PHA  ;(S is +2)
      PHX  ;(S is +3)
      PHY  ;(S is +4)     
      PHA  ;Push space for local vars (S is +5)
      PHA  ;(S is +6)
      PHA  ;(S is +7) 
      PHA  ;Push command line ptr arg onto stack from A/B (S is +8)
      XBA  
      PHA  ;(S is +9)
      LDX #$00 ;Push $0004 addend arg onto stack
      PHX  ;(S is +10)
      LDX #$04
      PHX  ;(S is +11)
      XBA
      JSR ADDW  ;Consume the initial command name
      PLX       ;Drop the addend argument from the stack, (S is +10)
      PLX       ;leaving the updated pointer (S is +9)
      JSR SKPWH ;Fast-forward past any whitespace characters
      PHX       ;Push room for parsed address retval (S is +10)
      PHX       ;(S is +11)
      PHX       ;(S is +12)
      JSR PRSAD ;Parse the address text at the pointer
      BCS STEP5 ;If the conversion failed, jump to fail message
      PLA       ;Otherwise, stash output value into local var area (S is +11)
tstbk STA 5,S   ;(11 - 5  = 2nd private byte + 4regs = 6)
      PLA       ;(S is +10)
      STA 5,S   ;(10 - 5 = 1st private byte + 4regs = 5)
      PLA       ;(S is +9)
      STA 5,S   ;(9 - 5 = 0th private byte + 4regs = 4)
      LDX #$00  ;Push $0006 addend arg onto stack
      PHX       ;(S is +10)
      LDX #$06
      PHX       ;(S is +11)
      JSR ADDW  ;Consume the address portion of the command string
      PLX       ;Drop the addend arg (S is +10)
      PLX       ;(S is +9)
      JSR SKPWH ;Fast-forward to next non-whitespace char
      LDA (1,S) ;Get the value at the fast-forwarded ptr location
      BEQ STEP2 ;If we hit the end of the string (zero), exit failed
      PHA       ;Make space for parse output on the stack (S is +10)
      JSR PRSBT ;And read the ensuing byte value
      PLA       ;Pull the parsed value (S is +9)
      TAX
      BCS STEP2 ;If the byte parse failed, exit failed
      PHB       ;Back up the previous DBR (S is +10)
      LDA 6,S   ;Load bank portion of address (10 - 6 = 0th private byte + 4 regs = 4)
      PHA       ;Set new DBR value (S is +11)
      PLB       ;(S is +10)
      TXA
      STA (4,S) ;Store value to 24-bit address (10 - 4 = 2nd private byte + 4 regs = 6)
      PLB       ;Restore DBR (S is +9)
      PLA       ;Drop string pointer (S is +8)
      PLA       ;(S is +7)
      CLC       ;Exit success
      BRA STEND
STEP5 PLA ;Dump backed-up values
      PLA 
      PLA 
STEP2 PLA
      PLA
SFALB SEC ;Exit fail
STEND PLA ;Dump local variable space
      PLA
      PLA
      PLY ;Restore regs
      PLX
      PLA
      XBA
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
      PHA       ;Put B/A string pointer onto the stack (addw arg 1) (S is +5)
      XBA       
      PHA       ;(S is +6)
      LDX #$00  ;Push 0x0004 addend (addw arg 2)
      PHX       ;(S is +7)
      LDX #$04
      PHX       ;(S is +8)
      JSR ADDW  ;Call ADDW to fast-forward past the 'addr' command chars
      PLX       ;Drop the addend arg from the stack, (S is +7)
      PLX       ;leaving the incremented address (S is +6)
      JSR SKPWH ;Fast-forward past any whitespace characters
      PHX       ;Push room for parsed address retval (S is +7)
      PHX       ;(S is +8)
      PHX       ;(S is +9)
      JSR PRSAD ;Parse the address text at the pointer
      BCS ADEP5 ;If the conversion failed, exit fail
      PLA       ;Otherwise, stash output value into local var area (S is +8)
      STA asm_base_lptr+2
      PLA       ;(S is +7)
      STA asm_base_lptr+1
      PLA       ;(S is +6)
      STA asm_base_lptr
      PLA       ;Drop string pointer (S is +5)
      PLA       ;(S is +4)
      CLC
      BRA ADEND  ;Exit success
ADEP5 PLX        ;Dump unneeded stack values
      PLX
      PLX
      PLX
      PLX
ADFAL SEC        ;Carry bit indicates command failure
ADEND PLY        ;Restore caller register values
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
      LDA [asm_base_lptr] ;Get value at global address pointer
      JSR PRTBT ;And print it
      JSR PRTLN ;And a newline
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

LODCM PHA
      PHX
      LDA #LDMSG/256
      XBA
      LDA #LDMSG&255
      JSR PRNTS
      JSR @RDCHR
      STA load_size
      LDX #$FF
GBYTE INX
      JSR @RDCHR
      STA $1000,X         ;TODO Upgrade past 256    
      CPX load_size
      BEQ LODDN
      LDA #$2E
      JSR @OUTCH
      BRA GBYTE
LODDN JSR PRTLN
      JSR @$001000
      CLC
      PLX
      PLA
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
      RTL

RDCHR
      LDA SRA
      AND #SR_RXRDY
      BEQ RDCHR
      LDA RHRA
      RTL

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
        JSR @OUTCH
        PLA
        BRA CHECK
CHECKPD PHA
        LDA #$0A
        JSR @OUTCH
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
      JSR @STCMP
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
      JSR PRTLN
      JSR PRTPMT
      LDY #$00
      STY cmd_buffer
      STY cmd_count
      PLA
PRSDN PLY
      PLX
      RTL

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
CKCHR JSR @RDCHR      ;Get the next character from the incoming ring buffer
      CMP #$00
      BEQ CKCHR       ;If the character was zero, try reading again
      JSR @OUTCH
      JSR @PARSE   
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

.dsb $FFEC - *,$00
.word PRNTS
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
