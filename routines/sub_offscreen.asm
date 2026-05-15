;===============================================
; sub_offscreen: Check if sprite is off-screen
;
; Output:
;    Carry set if offscreen, clear otherwise
;    $00-$03 are destroyed
;===============================================

!offs_threshold_x = $0060
!offs_threshold_y = $0060

    if !bowsie_owrev
        LDA !ow_sprite_props,x
        LSR
        BCC .do_off_screen_checks

        LDA !ow_sprite_x_pos,x
        SBC $1A
        STA $00
        LDA !ow_sprite_y_pos,x
        SEC
        SBC !ow_sprite_z_pos,x
        SEC
        SBC $1C
        STA $02
        CLC
        RTL
    endif

.do_off_screen_checks
    LDA !ow_sprite_x_pos,x
    SEC
    SBC $1A
    STA $00
    if !offs_threshold_x != 0
        CLC
        ADC #!offs_threshold_x
    endif
    if and(not(!bowsie_owrev), !bowsie_widescreen_ow)    ;   widescreen overworld check
        CMP.w #$FFF0-(!offs_threshold_x*2)
        BCS +
    endif
    CMP.w #$0100+(!offs_threshold_x*2)
    BCS ++
+   LDA !ow_sprite_y_pos,x
    SEC
    SBC !ow_sprite_z_pos,x
    SEC
    SBC $1C
    STA $02
    if !offs_threshold_y != 0
    CLC
    ADC #!offs_threshold_y
    endif
    CMP.w #$00E0+(!offs_threshold_y*2)
++  RTL
