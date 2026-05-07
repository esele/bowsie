; Macro library for BOWSIE

; General purpose macros

;---

; MaxTile macros

; maxtile_set_databank: set data bank when using MaxTile
macro maxtile_set_databank()
    if !bowsie_maxtile
        PEA.w (bank(main)<<8)|(bank(!oam_buffer))
        PLB

        bank bank(!oam_buffer)
    endif
endmacro

; maxtile_clear_databank: restore data bank when using MaxTile
macro maxtile_clear_databank()
    if !bowsie_maxtile
        PLB

        bank auto
    endif
endmacro

; maxtile_get_tilesize_index: set adequate index to tile size table when using MaxTile
macro maxtile_get_tilesize_index()
    if !bowsie_maxtile
        LDA.l !maxtile_oam_buffer_index_2
        TAY
    else
        TYA
        LSR #2
        TAY
    endif
endmacro

;---

; Graphics-related macros

; draw_shadow: draw a shadow tile based on Z position
; parameters: shadow_tile   - tile for the shadow
;             shawdow_props - YXPPCCCT props for the shadow
; expects 8-bit A
; can be called before or after an OAM slot move
macro draw_shadow(shadow_tile, shadow_props)
    bank bank(!oam_buffer)

    LDA $02
    CLC
    ADC ow_sprite_z_pos,x
    XBA
    LDA $00

    REP #$20
    STA.w oam_buffer[$00].x_pos,y
    LDA.w #(<shadow_props><<16|<shadow_tile>)
    STA.w oam_buffer[$00].tile,y
    SEP #$20

    bank auto
endmacro

; draw_shadow_with_tilesize: draw a shadow tile based on Z position with specific tilesize
; parameters: shadow_tile   - tile for the shadow
;             shawdow_props - YXPPCCCT props for the shadow
;             tilesize      - tile size for $0420
; expects 8-bit A
; can be called before or after an OAM slot move
macro draw_shadow_with_tilesize(shadow_tile, shadow_props, tilesize)
    LDA $02
    CLC
    ADC ow_sprite_z_pos,x
    XBA
    LDA $00

    REP #$20
    STA.w oam_buffer[$00].x_pos,y
    LDA.w #(<shadow_props><<16|<shadow_tile>)
    STA.w oam_buffer[$00].tile,y

    PHY
    %maxtile_get_tilesize_index
    SEP #$20
    LDA.b #<tilesize>
    STA.w oam_tilesize_buffer[$00].tile_sx,y
    PLY
endmacro

; draw_vanilla_shadow: draw the vanilla shadow tiles based on Z position
; the vanilla shadow is tile $229 with props yxPPccct/yXPPccct
; expects 8-bit A
; can be called before of after an OAM slot move
macro draw_vanilla_shadow()
    bank bank(!oam_buffer)

    LDA $02
    CLC
    ADC ow_sprite_z_pos,x
    CLC
    ADC #$08
    STA.w oam_buffer[!adjacent_oam_slot].y_pos,y
    STA.w oam_buffer[$00].y_pos,y

    LDA $00
    STA.w oam_buffer[!adjacent_oam_slot].x_pos,y
    CLC
    ADC #$08
    STA.w oam_buffer[$00].x_pos,y

    REP #$20
    LDA #$3029
    STA.w oam_buffer[!adjacent_oam_slot].tile,y
    LDA #$7029
    STA.w oam_buffer[$00].tile,y
    SEP #$20

    bank auto
endmacro

; draw_vanilla_shadow_with_tilesize: draw the vanilla shadow tiles based on Z position while setting the tile size buffer
; the vanilla shadow is tile $229 with props yxPPccct/yXPPccct
; tilesize is 00
; expects 8-bit A
; can be called before of after an OAM slot move
macro draw_vanilla_shadow_with_tilesize()
    LDA $02
    CLC
    ADC ow_sprite_z_pos,x
    CLC
    ADC #$08
    STA.w oam_buffer[!adjacent_oam_slot].y_pos,y
    STA.w oam_buffer[$00].y_pos,y

    LDA $00
    STA.w oam_buffer[!adjacent_oam_slot].x_pos,y
    CLC
    ADC #$08
    STA.w oam_buffer[$00].x_pos,y

    REP #$20
    LDA #$3029
    STA.w oam_buffer[!adjacent_oam_slot].tile,y
    LDA #$7029
    STA.w oam_buffer[$00].tile,y

    PHY
    %maxtile_get_tilesize_index()
    SEP #$20
    LDA.b #$00
    STA.w oam_tilesize_buffer[$00].tile_sx,y
    STA.w oam_tilesize_buffer[!adjacent_oam_slot].tile_sx,y
    PLY
endmacro

