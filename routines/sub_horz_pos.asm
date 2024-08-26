;===================================================
; sub_horz_pos: Check sprite's horizontal position
;               relative to player's
;
; Output:
;    Y is 0 if player is to the right, 1 otherwise
;===================================================

	LDY $0DD6|!addr
	LDA $1F17|!addr,y
	LDY #$00
	SEC
        SBC #$0008
	SEC
        SBC !ow_sprite_x_pos,x
	BPL $01
	INY
	RTL
