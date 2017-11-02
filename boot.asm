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
#define cmd_index $0E
#define cmd_buffer $0300
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
      .byte $A, $0
LDMSG .asc "LOAD not implemented yet."
      .byte $A, $0

#define command_count $02
cmd_str_table .word hello_string
              .word load_string
cmd_ptr_table .word HLOCM
              .word LODCM
hello_string .asc "hello"
             .byte $0
load_string  .asc "load"
             .byte $0

DOSPI PHX
      LDA #$00      ;reset byte counter
      STA clk_count
WFLO  LDA in_reg    ;check clock pin
      AND #clock_pin_mask
      BNE WFLO     ;if clock was not low, check it again
SETOB LDA out_buf   ;set spi out pin from spi_out buffer
      AND #$FE
      STA out_buf
      LDA spi_out
      AND #dout_pin_mask
      ORA out_buf   ;A = (out_buf & 0xFE) | (spi_out & 0x01)
      STA out_buf   ;Save computed out reg value
      STA out_reg   ;Output computed out reg value
      LSR spi_out   ;shift spi_out to the right
WFHI  LDA in_reg    ;check clock pin
      STA in_buf    ;save the value in case we need to get the data line value
      AND #clock_pin_mask
      BEQ WFHI      ;if it wasn't high check it again
      LDA in_buf    ;if it was high, check what the value of the data pin was
      AND #din_pin_mask
      BEQ CLRB      ;if it was low, jump to clear the buffer lsb
      LDA spi_in    ;otherwise, set the buffer lsb
      ORA #$01
      CLV 
      BVC CHKDN
CLRB  LDA spi_in    ;clear the buffer lsb
      AND #$FE
CHKDN STA spi_in    ;write the modified spi_in buffer back to memory
      LDY clk_count ;load and increment the clock counter
      INY
      TYA           ;store updated counter
      STA clk_count
      CMP #$08      ;check to see if we've read eight bits yet
      BEQ SPIDN     ;if so, jump to return section
      ASL spi_in    ;otherwise, shift the input spi buffer and get the next bit 
      CLV
      BVC WFLO 
SPIDN PLX
      RTL           ;full byte has been transfered, return to the caller

OUTCH PHX
      LDX out_count
      CPX #$FF
      BEQ OCHDN     ;if the out buffer is full, bail out
      STA txt_buffer,X 
      INX
      STX out_count
OCHDN PLX
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
      RTL

HLOCM PHA
      PHX
      LDA #HLMSG/256
      XBA
      LDA #HLMSG&255
      JSR @PRNTS
      PLX
      PLA
      RTS

LODCM PHA
      PHX
      LDA #LDMSG/256
      XBA
      LDA #LDMSG&255
      JSR @PRNTS
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
      BNE SCFL
      CMP #$00
      BEQ SCDN
      INY     ;Cleverly, this should force us to also exit if and when Y wraps
      BVC SCTOP
SCFL  CLC
SCDN  PLY
      RTL

PARSE PHX
      PHY
      LDX cmd_count
      CPX #$FF
      BEQ CHECK 
      CMP #$0A
      BEQ CHECK 
      STA cmd_buffer,X
      INX 
      STX cmd_count
      CLV
      BVC PRSDN
CHECK PHA
      LDA #$00
      STA cmd_buffer,X ;We'll parse this some day
      LDX #>cmd_buffer
      LDY #$00
      STY cmd_index
CMPLP
      LDY cmd_index
      CPY #command_count*2
      BEQ PRPMT
      LDA cmd_str_table,Y
      XBA
      INY
      LDA cmd_str_table,Y
      INY
      XBA
      STY cmd_index
      LDY #<cmd_buffer
      JSR @STCMP
      BCC CMPLP
      LDA cmd_index
      SBC #$02
      TAX
      JSR (cmd_ptr_table,X)
PRPMT LDA #PROMT/256
      XBA
      LDA #PROMT&255
      JSR @PRNTS
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
      LDA #PROMT/256
      XBA
      LDA #PROMT&255
      JSR @PRNTS      ;print command prompt
CONSL
      LDA out_count   ;load SPI output buffer with number of waiting bytes
      STA spi_out
      JSR @DOSPI      ;do an SPI transfer 
      LDA spi_in      ;get the value sent by the master, store temporarily
      STA incoming_ch 
      LDX #$00        ;wait for master to flush output buffer
FLSHL CPX out_count
      BEQ ENDFL
      LDA txt_buffer,X 
      STA spi_out
      JSR @DOSPI
      INX 
      CLV
      BVC FLSHL
ENDFL LDX #$00
      STX out_count
      LDA incoming_ch ;Add the char we were sent to the output buffer if nonzero
      BEQ SKPAD
      JSR @OUTCH
      JSR @PARSE   
SKPAD CLV
      BVC CONSL

CDEND
.dsb $FFFC - *,$00
.word START
.byte $00
.byte $AA
