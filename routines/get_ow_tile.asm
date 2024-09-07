;===============================================
; get_ow_tile: Get tile position for OW sprite
; ---
;===============================================

    LDA !ow_sprite_x_pos,x
    CLC
    ADC #$0008
    LSR #4
    STA $00
    LDA !ow_sprite_y_pos,x
    CLC
    ADC #$0008
    LSR #4
    STA $02
    LDX $0DD6|!addr
    TXA
    LSR #2
    TAX

    PHK
    PEA.w .return-1
    PEA $8413
    JML $049885|!bank
.return
    RTL

