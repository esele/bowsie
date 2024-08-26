;===============================================
; sub_update_z_pos: Update sprite's Z position
;
;    $00-$01 is destroyed
;===============================================

	TXA
	CLC
	ADC.w #!bowsie_ow_slots*4
	TAX
	%sub_update_x_pos()
	LDX !ow_sprite_index
	RTL
