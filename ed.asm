* = $0500

;ROM monitor routines
#define ROM_OUTCH         $00FFDE
#define ROM_RDCHR         $00FFE2
#define ROM_ENTER_MONITOR $00FFE6
#define ROM_PRNTS         $00FFEA

;Reserved memory areas
#define LINE_BUFFER       $0400

BRA main

ed_str .asc "ed> "
       .byte $0

main
    LDA #ed_str/256
    XBA
    LDA #ed_str&255
    JSR @ROM_PRNTS
    LDY #$00
rdline_top
    JSR @ROM_RDCHR
    CMP #$0D
    BEQ 
    JSR @ROM_OUTCH
    BRA loop
end
    LDA #$0A
    JSR @ROM_OUTCH
    LDA #$0D
    JSR @ROM_OUTCH
    JSR @ROM_ENTER_MONITOR
