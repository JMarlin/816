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
#define cmd_buffer $0400
#define in_reg $4000
#define out_reg $FFFF
#define clock_pin_mask $01
#define din_pin_mask $02
#define dout_pin_mask $01

#define prnts_ptr $FFEC

.byte 49   ;This needs to be updated to the size of the result stream - 1

* = $1000

START JMP @MAIN ;go to the main application

MSG .asc "Loaded and ran code over serial!"
    .byte $A, $D, $0

MAIN
      LDA #MSG/256
      XBA
      LDA #MSG&255
      LDX #0
      JSR (prnts_ptr,X)      ;print command prompt
      RTL

CDEND
