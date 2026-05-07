;===========================================================
; Custom Overworld Sprites: VLDC9 sprites on OW Revolution
; Adapted by Ari
;---------------------------
; This couldn't be done without resources/stuff made by:
; - Lui37, Medic et al.: original "custom ow sprites"
;   used in the ninth VLDC.
;   See https://smwc.me/1413938
; - yoshifanatic: author of OW Revolution
;   See https://smwc.me/t/128236
;===========================================================

assert read4($048000) == $524F4659, "This needs OW Revolution to work."

org $02A861                         ;   loads the actual OW sprites
    JML custom_ow_sprite_load

org $0480F2
    skip 4
    ;   four bytes reserved for custom OW sprite code in OW Revolution
    JSR.w run_ow_sprite             ;   process sprites
    RTL

    skip 4
    ;   four bytes reserved for custom spawn sprite routine in OW Revolution
    JSR.w spawn_overworld_sprite    ;   the equivalent of OWRev's SpawnOverworldSprite
    RTL

;---

if read1($0EF30F) == $42
    org read3($0EF30C)+256          ;   insert extra size table
    extra_byte_table:
        db $03
            incbin "extra_size.bin"
        for i = 128..256 : db $03 : endfor
endif

;---

org $04F800|!bank
; Sprite loader
custom_ow_sprite_load:
; this part is basically OWRev
    LDA [$CE],y                     ;\  restore code
    STA $05                         ;/
    LDA $0100|!addr                 ;\
    SEC                             ; |
    SBC #$0C                        ; | verify that we're actually on the OW
    CMP #$03                        ; |
    BCC .main                       ;/
    LDA $05
    JML $02A865|!bank

.main
    DEY
; actual custom code to load begins here
    PHX
if !sa1
    LDX.w #(!bowsie_ow_slots-1)*2
else
    LDX.b #(!bowsie_ow_slots-1)*2
endif
-   LDA.w !ow_sprite_num,x
    BEQ .free_slot
    DEX #2
    BPL -
    PLX
    JML $02A846|!bank

.free_slot
    DEY                             ;   get Y position
    LDA [$CE],y                     ;\
    PHA                             ; |
    AND #$F0                        ; | store Y position low byte
    STA !ow_sprite_y_pos,x          ; | 
    PLA                             ; |
    AND #$01                        ; | store Y position high byte
    ORA $0A                         ; |
    STA.w !ow_sprite_y_pos+1,x      ;/
    REP #$21
    LDA $00                         ;\  store X position (wow, if only Y pos were this simple!)
    STA !ow_sprite_x_pos,x          ;/

    STZ !ow_sprite_speed_x,x        ;\
    STZ !ow_sprite_speed_y,x        ; |
    STZ !ow_sprite_speed_z,x        ; |
    STZ !ow_sprite_z_pos,x          ; |
    STZ !ow_sprite_timer_1,x        ; |
    STZ !ow_sprite_timer_2,x        ; |
    STZ !ow_sprite_timer_3,x        ; |
    STZ !ow_sprite_misc_1,x         ; | clear sprite tables
    STZ !ow_sprite_misc_2,x         ; |
    STZ !ow_sprite_misc_3,x         ; |
    STZ !ow_sprite_misc_4,x         ; |
    STZ !ow_sprite_misc_5,x         ; |
    STZ !ow_sprite_speed_x_acc,x    ; |
    STZ !ow_sprite_speed_y_acc,x    ; |
    STZ !ow_sprite_speed_z_acc,x    ; |
    STZ !ow_sprite_init,x           ;/

    INY #3                          ;   move to the extra bytes
    PHX
    LDX $05
    if not(!sa1)
        REP #$10
    endif

    LDA.l extra_byte_table,x        ;\
    AND #$00FF                      ; | if there's no extra bytes (so only three bytes),
    SBC #$0002                      ; | move on
    BEQ .no_extra                   ;/
    CMP #$0005                      ;\  if there's more than 4 extra bytes,
    BCS .extra_byte_ptr             ;/  put a pointer instead

.regular
    STZ $0A
    STZ $0C
    SEP #$20
    STA $0E

    LDX #$0000                  ;\
-   LDA [$CE],y                 ; |
    STA $0A,x                   ; | loop through the amount of extra bytes:
    INY                         ; | extract them in $0A-$0D
    INX                         ; | the non-filled values initialize to 00
    CPX $0E                     ; |
    BCC -                       ;/

    if not(!sa1)
        SEP #$10
    endif
    REP #$20
    PLX                         ;\
    LDA $0A                     ; |
    STA !ow_sprite_extra_1,x    ; | store retrieved extra bytes in the extra byte tables
    LDA $0C                     ; |
    STA !ow_sprite_extra_2,x    ;/
    BRA .next_sprite

.no_extra
    if not(!sa1)
        SEP #$10
    endif
    PLX

.next_sprite
    SEP #$20
    DEY #2                          ;   correct this offset, for later
    LDA $05                         ;\  store the sprite number
    STA !ow_sprite_num,x            ;/
    LDA $02
    STA !ow_sprite_load_index,x

    ;   Show sprites during fade-in.
    PHY
    REP #$20
    if !sa1
        SEP #$10
    endif
    PHB
    LDA $1A
    PHA

    LDA $12,s                       ;\  recover true X position
    STA $1A                         ;/
    JSR execute_ow_sprite_init      ;   run sprite init routine
    INC !ow_sprite_init,x           ;   mark initialization as done
    JSR execute_ow_sprite           ;   run sprite main once

    PLA                             ;\  restore X position used in sprite load
    STA $1A                         ;/
    PLB
    if !sa1
        REP #$10
    endif
    SEP #$20
    PLY

    PLX
    JML $02A846|!bank               ;   return (we're back pretty soon either way)

.extra_byte_ptr
    STA $0A
    if not(!sa1)
        SEP #$10
    endif
    PLX

    TYA                         ;\
    CLC                         ; |
    ADC $CE                     ; |
    STA !ow_sprite_extra_1,x    ; | insert pointer to extra bytes:
    LDA $D0                     ; | now contained in the extra byte tables
    ADC #$0000                  ; |
    AND #$00FF                  ; |
    STA !ow_sprite_extra_2,x    ;/

    TYA                         ;\
    ADC $0A                     ; | offset the next slots adequately
    TAY                         ;/
    BRA .next_sprite

;---

; Actual handler
run_ow_sprite:
    PHB
    REP #$21
    LDA $9D                         ;\
    AND #$0002                      ; | OWRev uses bit 1 of $9D to skip sprite code completely
    BNE .skip_running_sprites       ;/

    LDX.b #(!bowsie_ow_slots-1)*2   ;\
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
    PLB
return:
    RTS

;---

;--------------------------------------------------------------------------------
; Routine that calls an OW sprite's main function
; Also reduces the sprite's timers by 1.
; Input:                X = sprite index
;        !ow_sprite_num,x = sprite number    
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

;--------------------------------------------------------------------------------
; Routine that calls an OW sprite's init function
; Input:                X = sprite index
;        !ow_sprite_num,x = sprite number    
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

spawn_overworld_sprite:
    PHX
    PHY
    PHP
    REP #$30

    LDX.w #(!bowsie_ow_slots-1)*2
.loop
    LDA !ow_sprite_num,x
    BEQ .spawn
    DEX #2
    BPL .loop

    PLP
    PLY
    PLX
    CLC
    RTS

.spawn
    LDA $0001,y
    STA !ow_sprite_num,x
    LDA $0003,y
    STA !ow_sprite_x_pos,x
    LDA $0005,y
    STA !ow_sprite_y_pos,x
    LDA $0007,y
    STA !ow_sprite_z_pos,x
    LDA $0009,y
    STA !ow_sprite_extra_1,x
    LDA $000B,y
    STA !ow_sprite_extra_2,x

    STZ !ow_sprite_init,x
    STZ !ow_sprite_speed_x,x
    STZ !ow_sprite_speed_x_acc,x
    STZ !ow_sprite_speed_y,x
    STZ !ow_sprite_speed_y_acc,x
    STZ !ow_sprite_speed_z,x
    STZ !ow_sprite_speed_z_acc,x
    STZ !ow_sprite_timer_1,x
    STZ !ow_sprite_timer_2,x
    STZ !ow_sprite_timer_3,x
    STZ !ow_sprite_misc_1,x
    STZ !ow_sprite_misc_2,x
    STZ !ow_sprite_misc_3,x
    STZ !ow_sprite_misc_4,x
    STZ !ow_sprite_misc_5,x
	SEP #$20
	LDA #$FF
	STA !ow_sprite_load_index,x

    PLP
    PLY
    PLX
    SEC
    RTS

;---

assert pc() <= $04FC00|!bank

freedata cleaned

