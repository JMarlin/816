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
#define str_base $08
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
#define cmd_buffer $0400
#define in_reg $4000
#define out_reg $FFFF
#define clock_pin_mask $01
#define din_pin_mask $02
#define dout_pin_mask $01

START CLV       ;Clear overflow, disable emulation mode
      CLC
      XCE
      LDA #$01  ;Set stack pointer to $01FF (grows downward)
      XBA
      LDA #$FF
      TCS
      JMP @MAINL ;go to the main application

PROMT .asc "-->"
      .byte $0

HLMSG .asc "Hello to you too!"
      .byte $A, $D, $0
LDMSG .asc "Ready for data"
      .byte $0
EXMSG .asc "Requested address: 0x"
      .byte $0
EQMSG .asc " = 0x"
      .byte $0
STMSG .asc "Storing to address: 0x"
      .byte $0
ASMSG .asc " <- 0x"
      .byte $0
SPFALMSG .asc "Expected a space following address."
         .byte $0
BPFALMSG .asc "Unable to parse input byte value."
         .byte $0
APFALMSG .asc "Unable to parse input address."
         .byte $0
ENDLN .byte $A, $D, $0

#define command_count $04
cmd_str_table .word hello_string
              .word load_string
              .word ex_string
              .word st_string
cmd_ptr_table .word HLOCM
              .word LODCM
              .word EXACM
              .word STOCM
hello_string  .asc "hello"
              .byte $0
load_string   .asc "load"
              .byte $0
ex_string     .asc "x"
              .byte $0
st_string     .asc "s"
              .byte $0
int_str .asc "INTERRUPT"
        .byte $0A, $00

OUTCH 
      STA $8000
      RTL

PRNTS PHY
      STA str_base
      XBA
      STA str_base + 1
      LDY #$00
      CLV
PRTOP LDA (str_base),Y 
      BEQ PRDN
      JSR @OUTCH
      INY     ;Cleverly, this should force us to also exit if and when Y wraps
      BVC PRTOP
PRDN  PLY
      RTS

HLOCM PHA
      PHX
      LDA #HLMSG/256
      XBA
      LDA #HLMSG&255
      JSR PRNTS
      PLX
      PLA
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
;Arguments | BA - 16-bit pointer to input string
;Returns   | A = Parsed byte
;          | Carry = set if parsing failed
PRSBT PHX 
      PHY
      STA str_base     ;Store the base address of the string
      XBA
      STA str_base + 1
      LDA (str_base)   ;Load the first byte from that address
      JSR PRSHX        ;Convert hex nybble to value
      BCS PBEND        ;If conversion failed, exit
      ASL ;Shift to high-order nybble
      ASL
      ASL
      ASL
      STA temp_reg     ;Back up value
      LDY #$01         ;Use Y to index next character
      LDA (str_base),Y 
      JSR PRSHX        ;Convert hex nybble to value
      BCS PBEND        ;If conversion failed, exit
      AND #$0F         ;Make sure top nybble is cleared
      ORA temp_reg     ;Insert top nybble
      CLC              ;Make sure we don't flag a failure
PBEND PLY
      PLX
      RTS

;PRSAD - Parse 24-bit Address from ASCII string
;Arguments | BA = 16-bit pointer to input string
;Returns   | X = Parsed address bank
;          | B = Parsed address hi
;          | A = Parsed address lo
;          | Carry = set if parsing failed
PRSAD PHY
      STA temp_ptr
      XBA
      STA temp_ptr + 1
      XBA
      JSR PRSBT ;Parse first two characters for bank
      BCS PAEND ;If carry was set, exit fail
      PHA       ;Backup bank value
      LDA temp_ptr + 1
      XBA
      LDA temp_ptr
      CLC       ;Increment string pointer to next byte
      ADC #$02
      XBA
      ADC #$00
      XBA      
      STA temp_ptr
      XBA
      STA temp_ptr + 1
      XBA
      JSR PRSBT ;Parse second byte
      BCS PAFLB ;Exit on fail
      PHA       ;Backup hi byte value
      LDA temp_ptr + 1
      XBA
      LDA temp_ptr
      CLC       ;Increment string pointer to next byte
      ADC #$02
      XBA
      ADC #$00
      XBA      
      JSR PRSBT ;Parse third byte
      BCS PAFLA ;Exit on fail
      TAY       ;Backup lo byte value
      PLA       ;Restore hi byte value
      XBA       ;Stash in B
      PLA       ;Restore bank value  
      TAX       ;Stash in X
      TYA       ;Put lo byte value back in A
      BRA PAEND
PAFLA PLA       ;Drop stashed value
PAFLB PLA       ;Drop stashed value
PAEND PLY
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
PNTLN PHA
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

;AddWord - adds XY into AB
;Args - XY = Value to be added
;     - AB = Value to be added to, output
ADDW PHX
     PHY
     CLC
     ADC $1,S
     XBA
     ADC $2,S
     XBA
     PLY
     PLX
     RTS

;Console command 's' - Store a byte to memory
;Format - 's AAAAAA BB' where 'A' is a hex digit
;         in a 24-bit address and 'B' is a hex digit
;         in a 8-bit value
STOCM PHA
      XBA
      PHA
      PHX
      PHY
      LDX #$00
      LDY #$02
      XBA
      JSR ADDW ;Consume the first two characters
      PHA      ;Backup string address
      XBA
      PHA
      XBA
      JSR PRSAD ;Parse the address
      BCS SFALA ;If the conversion failed, jump to fail message
      PHA       ;Otherwise, backup the calcuated values
      XBA
      PHA
      PHX
      PLA 
      STA long_ptr + 2
      PLA 
      STA long_ptr + 1
      PLA
      STA long_ptr
      PLA      ;Restore the command string ptr
      XBA
      PLA
      LDX #$00
      LDY #$06
      JSR ADDW ;Consume the address portion of the command string
      PHA      ;Backup updated pointer
      XBA
      PHA 
      XBA
      STA str_base     ;Load next character
      XBA
      STA str_base + 1
      LDA (str_base)
      CMP #$20         ;If it is not a space...
      BNE SFALB        ;Exit failed
      PLA              ;Restore cmd pointer again
      XBA
      PLA
TSTBK LDX #$00
      LDY #$01
      JSR ADDW ;Consume the space
      JSR PRSBT ;And read the ensuing byte value
      BCS SFALC ;If the byte parse failed, exit failed
      TAX       ;Backup the byte to be written
      LDA #STMSG/256 ;Print beginning of string
      XBA
      LDA #STMSG&255
      JSR PRNTS
      LDA long_ptr + 2 ;Restore bank value
      JSR PRTBT  ;Print the value
      LDA long_ptr + 1 ;Restore hi byte
      JSR PRTBT  ;Print
      LDA long_ptr ;Restore lo byte
      JSR PRTBT  ;Print
      LDA #ASMSG/256 ;Print ' <- '
      XBA 
      LDA #ASMSG&255
      JSR PRNTS
      TXA            ;Restore byte to write
      STA [long_ptr] ;Store value to 24-bit address
      JSR PRTBT      ;Print the written value
      BRA STEND  ;Exit success
SFALA LDA #APFALMSG/256 ;Print failure message
      XBA 
      LDA #APFALMSG&255
      JSR PRNTS
      BRA STEND
SFALB LDA #SPFALMSG/256
      XBA 
      LDA #SPFALMSG&255
      BRA STEND
SFALC LDA #BPFALMSG/256
      XBA 
      LDA #BPFALMSG&255
STEND JSR PNTLN
      PLY
      PLX
      PLA
      XBA
      PLA
      RTS

;Console command 'x' - eXamine a byte of memory
;Format - 'x AAAAAA' where 'A' is a hex digit in a 
;         24-bit address
EXACM PHA
      XBA
      PHA
      PHX
      PHY
      LDX #$00
      LDY #$02
      XBA
      JSR ADDW ;Consume the first two characters
      JSR PRSAD ;Parse the address
      BCS EXFAL ;If the conversion failed, jump to fail message
      PHA       ;Otherwise, backup the calcuated values
      XBA
      PHA
      PHX
      LDA #EXMSG/256
      XBA
      LDA #EXMSG&255
      JSR PRNTS
      PLA        ;Restore bank value
      STA long_ptr + 2
      JSR PRTBT  ;Print the value
      PLA        ;Restore hi byte
      STA long_ptr + 1
      JSR PRTBT  ;Print
      PLA        ;Restore lo byte
      STA long_ptr
      JSR PRTBT  ;Print
      LDA #EQMSG/256 ;Print ' = '
      XBA 
      LDA #EQMSG&255
      JSR PRNTS
      LDA [long_ptr] ;Load value from 24-bit address
      JSR PRTBT      ;Print the peeked value
      BRA EXEND  ;Exit success
EXFAL LDA #APFALMSG/256 ;Print failure message
      XBA 
      LDA #APFALMSG&255
      JSR PRNTS
EXEND JSR PNTLN
      PLY
      PLX
      PLA
      XBA
      PLA
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
LODDN LDA #$0A
      JSR @OUTCH
      LDA #$0D
      JSR @OUTCH
      JSR @$001000
      PLX
      PLA
      RTS

STCMP PHY
      STA str_base
      XBA
      STA str_base + 1
      STY cmp_base
      STX cmp_base + 1
      LDY #$00
      CLV
      SEC
SCTOP LDA (str_base),Y
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
      LDA $4001
      AND #$01
      BEQ RDCHR
      LDA $4000
      RTL

PARSE PHX
      PHY
      LDX cmd_count
      CPX #$FF
      BEQ CHECK 
      CMP #$0A
      BEQ CHECK 
      CMP #$0D
      BEQ CHECK
      STA cmd_buffer,X
      INX 
      STX cmd_count
      BRA PRSDN
CHECK PHA
      LDA #$00
      STA cmd_buffer,X ;We'll parse this some day
      LDX #>cmd_buffer
      LDY #$00
      STY cmd_count
CMPLP
      LDY cmd_count
      CPY #command_count*2
      BEQ PRPMT
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
PRPMT LDA #PROMT/256
      XBA
      LDA #PROMT&255
      JSR PRNTS
      LDY #$00
      STY cmd_buffer
      STY cmd_count
      PLA
PRSDN PLY
      PLX
      RTL

MAINL
      LDA #$AA
      STA out_buf
      STA out_reg     ;Set display
      LDA #$00
      STA out_count   ;init output buffer
      STA cmd_count
      STA inbuf_count
      STA inbuf_ptr
      PHP             ;Clear IRQ Disable bit
      PLA
      AND #$FB
      PHA
      PLP
      LDA #PROMT/256
      XBA
      LDA #PROMT&255
      JSR PRNTS      ;print command prompt
CKCHR JSR @RDCHR      ;Get the next character from the incoming ring buffer
      CMP #$00
      BEQ CKCHR       ;If the character was zero, try reading again
      JSR @OUTCH
      JSR @PARSE   
SKPAD BRA CKCHR

CDEND

.dsb $FFEC - *,$00
.word PRNTS
.word 0     ;FFEE,EF
.word 0     ;FFF0,F1
.word 0     ;FFF2,F3
.word 0     ;FFF4,F5
.word 0     ;FFF6,F7
.word 0     ;FFF8,F9
.word 0     ;FFFA,FB
.word START ;FFFC,FD
.byte $00
.byte $AA
