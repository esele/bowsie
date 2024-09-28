BOWSIE v1.03
---
### Features
- Added the `%distance()` shared subroutine, used in VLDC9 sprites to get the absolute positional difference between the player and a sprite.
- Modified `%interact()` so it accounts for player 2 as well.
- The list file can now contain blank lines.
- The list file can now contain _comment lines_, which are lines that begin with a semicolon `;`.
### Fixes
- Fixed various bugs which were causing incorrect sprite Map16 generation:
  - The generator was miscounting when using 16x16 tiles. This was most noticeable when using json files which defined more than 1 16x16 tile.
  - Y flip was being ignored.
  - Priority was using two bits instead of one. I forget Map16 data is _YXPCCCTT_, and not _YXPPCCCT_... [silly me](https://i.kym-cdn.com/photos/images/original/002/086/808/90f.gif).
- Fixed an anomaly regarding some spawn X positions due a rogue carry in the VLDC9 system.
- Fixed a bug with `%get_draw_info()` where carry, set when off-screen, was being lost when asking for priority, making sprites get drawn even when off-screen.
- Now the sprite spawn routine clears the init state flag.
- Made the VLDC9 system read the pointer directly from ROM instead of using an Asar read. This _should_ fix issues related to crashing when not re-running the tool, at the expense of three bytes and some cycles. Thanks go to Thomas/kaizoman for suggesting this alternative!

Note that since OW Revolution support is still immature, I'm not pushing the version number to v1.10 yet.

BOWSIE v1.02
---
### Features
- Added experimental support for yoshifanatics' Overworld Revolution. Note that the tool will attempt to detect the patch on its own.\
The following additions are all related to this above feature:
  - Added the `owrev` sprite system, which modifies yoshifanatic's system with a variant of Lui's system. It's included at `owrev.asm`.
  - Added reads in the tool to find the current freespace in bank 4, where BOWSIE installs some of its code, and the routine `SMW_ProcessOWPlayers_DrawOverworldPlayerAndBorderSprites`, as the original jump is overwritten.
  - Added an `RTL` at `$04FFFF` so the uninserted sprites have a correct pointer.
  - Note that for now, overworld sprite Map16 data is still generated instead of regular sprite Map16. This will be corrected at a later date.
  - Note that the shadow drawing routine from OW Revolution is erased.
  - Note that _BOWSIE_ won't remove OW Revolution's shared subroutines, wasting space.
  - Note that this replaces yoshifanatic's own sprites with _BOWSIE_'s! Don't insert sprites with OW Revolution.
- This support is extremely experimental. Once it has been tested properly, the version number will be pushed to 1.10.

BOWSIE v1.01
---
### Features
- Moved back the OAM priority further so sprites which need priority can actually go behind the player.
- Added the shared subroutine `%get_ow_tile()`, which a sprite can call to get the index to the 16x16 tile the sprite is currently on, via a call to `$049885`. This routine is commonly used by carol's sprites.
### Fixes
- Fixed an issue in the VLDC9 system where the player's graphics routine, which in the vanilla game runs _after the sprites_, could overwrite certain sprite tiles, if the user didn't ask priority from `%get_draw_info()`. This happens whether _OPSE_ is used or not.
- Fixed an issue where `%get_draw_info()` was always writing the next OAM index to `!ow_sprite_oam`. This caused conflicts when using sprites with and without priority calls at the same time.
- Fixed an issue where BOWSIE failed to find the relevant directories when calling the tool from the command line as an absolute path. Thanks to Stivi for reporting this!

BOWSIE v1.00
---
- Initial release.