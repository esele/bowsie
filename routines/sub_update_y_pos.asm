;===============================================
; sub_update_y_pos: Update sprite's Y position
;
;    $00-$01 is destroyed
;===============================================

	TXA
	CLC
	ADC.w #!bowsie_ow_slots*2
	TAX
	%sub_update_x_pos()
	LDX !ow_sprite_index
	RTL
