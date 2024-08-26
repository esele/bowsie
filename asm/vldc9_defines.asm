; In Lui's system, all sprite tables are 16 bit, with the sprite index in X
; and discontiguous tables, just like in the base vanilla sprites.

incsrc bowsie_defines.asm       ; don't delete this! it's created during the tool run.

!oam_start_p    = $0070
!oam_start      = $00BC
!oam_limit      = $01E8

macro define_ow_sprite_table(name, address)
        !<name> = $<address>|!addr
        !<address> = !<name>
endmacro

;   Tables
%define_ow_sprite_table(ow_sprite_num, 14C8)
%define_ow_sprite_table(ow_sprite_speed_x, 14F8)
%define_ow_sprite_table(ow_sprite_speed_y, 1528)
%define_ow_sprite_table(ow_sprite_speed_z, 1558)
%define_ow_sprite_table(ow_sprite_x_pos, 1588)
%define_ow_sprite_table(ow_sprite_y_pos, 15B8)
%define_ow_sprite_table(ow_sprite_z_pos, 15E8)
%define_ow_sprite_table(ow_sprite_timer_1, 1618)
%define_ow_sprite_table(ow_sprite_timer_2, 1648)
%define_ow_sprite_table(ow_sprite_timer_3, 1678)
%define_ow_sprite_table(ow_sprite_misc_1, 16A8)
%define_ow_sprite_table(ow_sprite_misc_2, 16D8)
%define_ow_sprite_table(ow_sprite_misc_3, 1708)
%define_ow_sprite_table(ow_sprite_misc_4, 1738)
%define_ow_sprite_table(ow_sprite_misc_5, 1768)
%define_ow_sprite_table(ow_sprite_extra_bits, 1798)
%define_ow_sprite_table(ow_sprite_speed_x_acc, 17C8)
%define_ow_sprite_table(ow_sprite_speed_y_acc, 17F8)
%define_ow_sprite_table(ow_sprite_speed_z_acc, 1828)
%define_ow_sprite_table(ow_sprite_init, 188C)
;   Flags
%define_ow_sprite_table(ow_sprite_index, 1858)
%define_ow_sprite_table(ow_sprite_oam, 185A)
%define_ow_sprite_table(ow_sprite_oam_p, 185C)
