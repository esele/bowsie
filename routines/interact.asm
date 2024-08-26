;====================================
; A basic interaction routine
; By Erik, originally for the fish
; and goomba sprites
;====================================

        LDA $1F17|!addr
        SEC
        SBC #$0008
        AND #$FFFE
        CMP !ow_sprite_x_pos,x
        BNE .return
        LDA !ow_sprite_y_pos,x
        AND #$FFFE
        STA $00
        LDA $1F19|!addr
        SEC
        SBC #$000C
        AND #$FFFE
        CMP $00
        BNE .return
        SEC
        RTL
.return
        CLC
        RTL
