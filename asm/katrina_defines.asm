; In Katrina's system, all sprite tables are 16 bit, and instead of having
; the sprite index in X and discontiguous tables, we have 14C8+index in X,
; with contiguous tables per sprite.

incsrc bowsie_defines.asm       ; don't delete this! it's created during the tool run.

!oam_start_p    = $0070
!oam_start      = $00BC
!oam_limit      = $01E8

macro define_ow_sprite_table(name, address)
        !<name> = <address>
        !<address> = !<name>
endmacro

macro define_base2_address(name, address)
        !<name> = <address>|!addr
        !<address> = !<name>
endmacro

;   Tables
%define_ow_sprite_table(ow_sprite_num, 00)
%define_ow_sprite_table(ow_sprite_speed_x, 02)
%define_ow_sprite_table(ow_sprite_speed_y, 04)
%define_ow_sprite_table(ow_sprite_speed_z, 06)
%define_ow_sprite_table(ow_sprite_x_pos, 08)
%define_ow_sprite_table(ow_sprite_y_pos, 0A)
%define_ow_sprite_table(ow_sprite_z_pos, 0C)
%define_ow_sprite_table(ow_sprite_timer_1, 0E)
%define_ow_sprite_table(ow_sprite_timer_2, 10)
%define_ow_sprite_table(ow_sprite_timer_3, 12)
%define_ow_sprite_table(ow_sprite_misc_1, 14)
%define_ow_sprite_table(ow_sprite_misc_2, 16)
%define_ow_sprite_table(ow_sprite_misc_3, 18)
%define_ow_sprite_table(ow_sprite_misc_4, 1A)
%define_ow_sprite_table(ow_sprite_misc_5, 1C)
%define_ow_sprite_table(ow_sprite_extra_bits, 1E)
%define_ow_sprite_table(ow_sprite_speed_x_acc, 20)
%define_ow_sprite_table(ow_sprite_speed_y_acc, 22)
%define_ow_sprite_table(ow_sprite_speed_z_acc, 24)
;   Flags
%define_base2_address(ow_sprite_init, 188C)
%define_base2_address(ow_sprite_index, 1858)
%define_base2_address(ow_sprite_oam, 185A)
%define_base2_address(ow_sprite_oam_p, 185C)
