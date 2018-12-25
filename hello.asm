* = $0500

#define ROM_OUTCH         $00FFDE
#define ROM_RDCHR         $00FFE2
#define ROM_ENTER_MONITOR $00FFE6
#define ROM_PRNTS         $00FFEA

BRA main

hello_str .asc "Press keys to see them echoed back to you."
          .byte $0A, $0D
          .asc "Press the space bar to return to the monitor."
          .byte $0A, $0D
          .byte $0

main
    LDA #hello_str/256
    XBA
    LDA #hello_str&255
    JSR @ROM_PRNTS
loop
    JSR @ROM_RDCHR
    CMP #$20
    BEQ return
    JSR @ROM_OUTCH
    BRA loop
return
    LDA #$0A
    JSR @ROM_OUTCH
    LDA #$0D
    JSR @ROM_OUTCH
    JSR @ROM_ENTER_MONITOR
