;===========================================================
; Custom Overworld Sprites: Lui's sprites on OW Revolution
;---------------------------
; Adapted by Erik
; This couldn't be done without resources/stuff made by:
; - Lui37, Medic et al.: original "custom ow sprites"
;   used in the ninth VLDC.
;   See https://smwc.me/1413938
; - yoshifanatic: author of OW Revolution
;   See https://smwc.me/t/128236
;===========================================================

assert read4($048000) == $524F4659, "This needs OW Revolution to work."

; Misc hijacks

if read1($0EF30F) == $42
    org read3($0EF30C)+256          ;   enable 2 extra bytes (if the table exists only)
        for i = 0..100 : db $05 : endfor
endif
    
org read3($02A862)+22               ;   loads the actual OW sprites
custom_ow_sprite_load_main:
if !sa1
    LDX.w #!bowsie_ow_slots*2-2
else
    LDX.b #!bowsie_ow_slots*2-2
endif
-   LDA.w !ow_sprite_num,x
    BEQ .free_slot
    DEX #2
    BPL -
    PLX
    JML $02A846|!bank

.free_slot
    DEY
    LDA [$CE],y
    PHA
    AND #$F0
    STA !ow_sprite_y_pos,x
    PLA
    AND #$01
    ORA $0A
    STA.w !ow_sprite_y_pos+1,x
    REP #$20
    LDA $00
    STA !ow_sprite_x_pos,x
    JML clear_tables            ;   honestly it may fit but I just don't want to risk overflowing

;---

; Main hack
org $0480DE     ; this one should *always* be fixed, if I'm not a fucking moron
run_ow_sprite:
    PHB
    REP #$21
    LDA $9D
    AND #$0002
    BNE .skip_running_sprites
    LDA #!oam_start
    STA !ow_sprite_oam
    LDA #!oam_start_p
    STA !ow_sprite_oam_p

    LDX.b #!bowsie_ow_slots*2-2     ;\
-   LDA !ow_sprite_num,x            ; | Main loop.
    BEQ .no_sprite                  ; | Call execute_ow_sprite for all sprites where     
    LDA !ow_sprite_init,x           ; | !ow_sprite_num,x is not zero.
    BNE +                           ;/
    JSR execute_ow_sprite_init      ;\
    INC !ow_sprite_init,x           ; | Or, in case !ow_sprite_init,x is still zero,
    BRA .no_sprite                  ; | call execute_ow_sprite_init and then INC it.
+                                   ;/
    JSR execute_ow_sprite
.no_sprite
    DEX #2
    BPL -

.skip_running_sprites
    JSL !owrev_draw_player_routine  ;\
    SEP #$30                        ; | Restore code
    PLB                             ;/
    RTL

assert pc() <= $048146              ; this routine is needed so let's just stop here.

;---

org !owrev_bank_4_freespace
;--------------------------------------------------------------------------------
; Routine that calls the ow-sprites main function
; Also reduces the sprite's timers by 1.
; Input: X                      = sprite index
;          !ow_sprite_num,x  = sprite number    
;--------------------------------------------------------------------------------
execute_ow_sprite:
    STX !ow_sprite_index

    LDA !ow_sprite_timer_1,x        ;\
    BEQ +                           ; |
    DEC !ow_sprite_timer_1,x        ; |
+   LDA !ow_sprite_timer_2,x        ; |
    BEQ +                           ; | Decrease timers 
    DEC !ow_sprite_timer_2,x        ; |
+   LDA !ow_sprite_timer_3,x        ; |
    BEQ +                           ; |
    DEC !ow_sprite_timer_3,x        ;/

+   LDA !ow_sprite_num,x            ;\
    ASL                             ; |
    ADC !ow_sprite_num,x            ; | Sprite number times 3 in x
    TXY                             ; |
    TAX                             ;/

    LDA.l ow_sprite_main_ptrs-3,x   ;\ 
    STA $00                         ; | Get sprite main pointer in $00
    SEP #$20                        ; | Sprite number 00 is <end> so the table
    LDA.l ow_sprite_main_ptrs-1,x   ; | is actually 1 indexed (hence those subtractions)
    STA $02                         ;/

    PHA                             ;\ 
    PLB                             ; | Setup bank (value still in A)
    REP #$20                        ; | A in 16 bit
    TYX                             ;/

    PHK                             ;\
    PEA.w return-1                  ;/ workaround for JSL [$0000]
    JML.w [!dp]
return:
    RTS

;--------------------------------------------------------------------------------
; Routine that calls the ow-sprites init function
; Also reduces the sprite's timers by 1.
; Input: X                      = sprite index
;          !ow_sprite_num,x  = sprite number    
;--------------------------------------------------------------------------------
execute_ow_sprite_init:
    STX !ow_sprite_index

    LDA !ow_sprite_num,x            ;\
    ASL                             ; |
    ADC !ow_sprite_num,x            ; | Sprite number times 3 in x
    TXY                             ; |
    TAX                             ;/

    LDA.l ow_sprite_init_ptrs-3,x   ;\ 
    STA $00                         ; | Get sprite init pointer in $00
    SEP #$20                        ; | sprite number 00 is <end> so the table
    LDA.l ow_sprite_init_ptrs-1,x   ; | is actually 1 indexed (hence those subtractions)
    STA $02                         ;/
    
    PHA                             ;\ 
    PLB                             ; | Setup bank (value still in A)
    REP #$20                        ; | A in 16 bit
    TYX                             ;/
    PHK                             ;\
    PEA.w return-1                  ;/ workaround for JSL [$0000]
    JML.w [!dp]

;---

clear_tables:
    STZ !ow_sprite_speed_x,x
    STZ !ow_sprite_speed_y,x
    STZ !ow_sprite_speed_z,x
    STZ !ow_sprite_z_pos,x
    STZ !ow_sprite_timer_1,x
    STZ !ow_sprite_timer_2,x
    STZ !ow_sprite_timer_3,x
    STZ !ow_sprite_misc_1,x
    STZ !ow_sprite_misc_2,x
    STZ !ow_sprite_misc_3,x
    STZ !ow_sprite_misc_4,x
    STZ !ow_sprite_misc_5,x
    STZ !ow_sprite_speed_x_acc,x
    STZ !ow_sprite_speed_y_acc,x
    STZ !ow_sprite_speed_z_acc,x
    STZ !ow_sprite_init,x
    INY #3
    LDA [$CE],y
    STA !ow_sprite_extra_bits,x
    DEY #2
    SEP #$20
    LDA $05
    STA !ow_sprite_num,x
    PLX
    JML $02A846|!bank

