* = $8000

STRING .asc "Booting JOE/S R0"
       .byt $0A, $0D, $00
DMSG   .asc "JOE/S>"
       .byt $00
HMSG   .asc "Hello!"
       .byt $0A, $0D, $00
CMDSTR .asc "hi"
       .byt $00	

MAIN CLV
     LDY #$00
LOOP LDA STRING,Y
     BEQ KEY
     STA $7FFF
     INY
     BVC LOOP

KEY  LDA $7FFF
     BNE PRNT
     BVC KEY

PRNT LDY #$00
L2   LDA DMSG,Y
     BEQ ECHO_ENT
     STA $7FFF
     INY
     BVC L2

ECHO_ENT
     LDY #$00
ECHO LDA $7FFF
     BEQ ECHO
     STA $7FFF
     CMP #$0A
     BEQ PARSE
     STA $04,Y
     INY
     BVC ECHO
PARSE
     CLC
     XCE
     SEC
     XCE
     LDA #$00
     STA $04,Y
     LDA #$04
     STA $00
     LDA #$00
     STA $01
     LDA #<CMDSTR
     STA $02
     LDA #>CMDSTR
     STA $03
     JSR SCMP
     BCC PARSE_END
     LDY #$00
CM_LOOP
     LDA HMSG,Y
     BEQ PARSE_END
     STA $7FFF
     INY
     BVC CM_LOOP
PARSE_END
     BVC PRNT

SCMP CLV
     LDY #$00
SC_LOOP
     LDA ($00),Y
     BEQ SC_END
     CMP ($02),Y
     BNE SC_FAIL
     INY
     BVC SC_LOOP
SC_END
     LDA ($02),Y
     BNE SC_FAIL
     SEC
     RTS
SC_FAIL
     CLC
     RTS

QUIT BVC QUIT
     

.dsb $FFFC - *,$00
.word MAIN
