;===================================================
; finish_oam_write: Write to size table
; By Akaginite/33953YoShI, adapted by Ari
;
; Input:
;   16-bit AXY
;   A contains the number of tiles to draw - 1
;   X contains the size
;     * $00     - 8x8 tiles
;     * $02     - 16x16 tiles
;     * $80-$FF - manually set the size
;   If MaxTile isn't used:
;   Y contains the last OAM slot used. The GFX
;   routine has Y go down by four. This routine
;   has Y increase up by four.
;
; Output:
;   16-bit AXY
;   X contains the sprite index
;   $00-$03 and $08-$0C are destroyed
;   the size table is now set correctly
;===================================================

    STX $0B
    STA $08
    LDX !ow_sprite_index
    LDA !ow_sprite_y_pos,x
    SEC
    SBC !ow_sprite_z_pos,x
    SEC
    SBC $1C
    STA $00
    LDA !ow_sprite_x_pos,x
    SEC
    SBC $1A
    STA $02
    if !bowsie_maxtile
        LDY !maxtile_oam_buffer_index_1
        LDX !maxtile_oam_buffer_index_2
    else
        TYA
        LSR #2
        TAX
    endif
    LDA #$0000
    SEP #$21
    if !bowsie_maxtile
        PHB
        LDA.b #bank(!oam_buffer)
        PHA
        PLB
    endif

.loop
    LDA.w oam_buffer[$00].x_pos,y
    SBC $02
    REP #$21
    BPL +
    ORA #$FF00
+   ADC $02
    CMP #$0100
    LDA #$0000
    SEP #$20

    LDA $0B
    BPL +
    LDA.w oam_tilesize_buffer[$00].tile_sx,x
    AND #$02
+   ADC #$00
    STA.w oam_tilesize_buffer[$00].tile_sx,x

    LDA.w oam_buffer[$00].y_pos,y
    SEC
    SBC $00
    REP #$21
    BPL +
    ORA #$FF00
+   ADC $00
    CLC
    ADC #$0010
    CMP #$0100
    BCC .next

    LDA #$00F0
    SEP #$20
    STA.w oam_buffer[$00].y_pos,y

.next
    SEP #$21
    INY #4
    INX
    DEC $08
    BPL .loop

    if !bowsie_maxtile
        PLB
    endif
    LDX !ow_sprite_index
    REP #$20
    RTL

