;=========================================================
; Custom Overworld Sprites
; LM Implementation by Erik
;
; This couldn't be done without resources/stuff made by:
; - Lui37, Medic et al.: original "custom ow sprites"
;   used in the ninth VLDC. Most of this is based on his
;   work.
;   See https://smwc.me/1413938
; - Katrina: she wrote the proposal for the new overworld
;   sprite design. I followed some of her guidelines and
;   implemented her two design proposals.
;   See https://bin.smwcentral.net/u/8006/owc.txt
; - FuSoYa: added the custom overworld sprite support into
;   Lunar Magic, based on the format used by VLDC9.
;=========================================================

        lorom
        !sa1    = 0
        !dp     = $0000
        !addr   = $0000
        !bank   = $800000
if read1($00FFD5) == $23
        sa1rom
        !sa1    = 1
        !dp     = $3000
        !addr   = $6000
        !bank   = $000000
endif


assert read3($0EF55D) != $FFFFFF, "Please insert any custom overworld sprite with Lunar Magic first. (Press Insert when in sprite mode)"
org read3($0EF55D)
map_offsets:

incsrc "main/defines.asm"

org $00A165                             ;   jump to new ow sprite load (this one will run in gamemode $0C)
        JML ow_sprite_load

org $009AA4                             ;   nuke jump to original ow sprite load
        BRA $02 : NOP #2

org $04F675                             ;   nuke original ow sprite load (which runs in gamemode $05)
        padbyte $EA : pad $04F6F8

;   main hijack, within vanilla freespace
org $04F675|!bank
ow_sprite_load:
        PHB
        LDX $0DB3|!addr
        LDA $1F11|!addr,x
        ASL
        TAX
        REP #$21
        LDA.w #map_offsets
        ADC.l map_offsets,x
        STA $6B
        LDY.b #map_offsets>>16
        STY $6D

        LDY #$00
.sprite_load_loop
        LDA [$6B],y
        BEQ .end_spawning
        AND #$007F
        STA $00
        LDA [$6B],y
        AND #$1F80
        XBA
        ASL
        BCC +
        INC
+       ASL #3
        STA $02
        INY
        LDA [$6B],y
        AND #$07E0
        ASL #3
        XBA
        ASL #3
        STA $04
        LDA [$6B],y
        AND #$F800
        XBA
        STA $06
        INY #2
        LDA [$6B],y
        STA $08

        JSL spawn_sprite
        BCC .end_spawning
        ; PHY
        ; JSR execute_ow_sprite_init
        ; PLY

        INY
        BRA .sprite_load_loop

.end_spawning
        ; STZ !ow_sprite_index
        SEP #$20
        PLB
        JSL $04D6E9|!bank
        JML $00A169|!bank
warnpc $04F6F8|!bank

org $04F76E|!bank
run_ow_sprite:
        PHB
        REP #$21
        LDA #!oam_start
        STA !ow_sprite_oam
        LDX.b #!slots*2-2
-       LDA !ow_sprite_num,x
        BEQ .no_sprite
        LDA !ow_sprite_init,x
        BNE +
        JSR execute_ow_sprite_init
        INC !ow_sprite_init,x
        BRA .no_sprite
+       JSR execute_ow_sprite
.no_sprite
        DEX #2
        BPL -
        ; STZ !ow_sprite_index
        SEP #$20
        PLB
        RTS

execute_ow_sprite:
        STX !ow_sprite_index

        LDA !ow_sprite_timer_1,x
        BEQ +
        DEC !ow_sprite_timer_1,x
+       LDA !ow_sprite_timer_2,x
        BEQ +
        DEC !ow_sprite_timer_2,x
+       LDA !ow_sprite_timer_3,x
        BEQ +
        DEC !ow_sprite_timer_3,x

+       LDA !ow_sprite_num,x
        ASL
        ADC !ow_sprite_num,x
        TXY
        TAX
        LDA.l ow_sprite_main_ptrs-3,x
        STA $00
        SEP #$20
        LDA.l ow_sprite_main_ptrs-1,x
        STA $02
        PHA
        PLB
        REP #$20
        TYX
        PHK
        PEA.w .return_execute-1
        JML.w [!dp]
.return_execute
        RTS

execute_ow_sprite_init:
        STX !ow_sprite_index

        LDA !ow_sprite_num,x
        ASL
        ADC !ow_sprite_num,x
        TXY
        TAX
        LDA.l ow_sprite_init_ptrs-3,x
        STA $00
        SEP #$20
        LDA.l ow_sprite_init_ptrs-1,x
        STA $02
        PHA
        PLB
        REP #$20
        TYX
        PHK
        PEA.w .return_execute-1
        JML.w [!dp]
.return_execute
        RTS


warnpc $04F8A6|!bank

freecode
prot subroutines_start

incsrc "main/data.asm"
incsrc "main/subroutines.asm"

