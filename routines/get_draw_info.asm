;===================================================
; get_draw_info: Find free OAM slots to draw tiles
;
; Input:
;    A contains the amount of tiles to draw,
;      minus 1
;    Clear carry to ask priority
;
; Output:
;    16-bit X/Y
;    carry set if off screen, clear otherwise
;    Y contains the oam index to use
;    $00 contains the X position, screen relative
;    $02 contains the Y position, screen relative
;    $04-$09 are destroyed
;===================================================

        REP #$10
        BCS +
        LDY !ow_sprite_oam_p
        BRA .start
+       LDY !ow_sprite_oam
.start
        ASL #2
        STA $04
        STZ $06
        %sub_offscreen()
        BCS .return
        SEP #$20

.oam_loop
        CPY #!oam_limit
        BCC +
.loop_back
        LDY #!oam_start
        LDA $06
        BNE .found_all_slots
        INC $06
+       LDA $0201|!addr,y
        CMP #$F0
        BEQ .check_tile_amount
.oam_next
        INY #4
        BRA .oam_loop

.check_tile_amount
        REP #$21
        TYA
        ADC $04
        STA $08
        SEP #$20
.tile_amount_loop
        CPY $08
        BEQ .found_all_slots
        INY #4
        CPY #!oam_limit
        BCS .loop_back
        LDA $0201|!addr,y
        CMP #$F0
        BEQ .tile_amount_loop
        BRA .oam_next

.found_all_slots
        REP #$21
        TYA
        ADC #$0004
        STA !ow_sprite_oam
        CLC
.return
        RTL
