BOWSIE v1.02
---
### Features
- Added experimental support for yoshifanatics' Overworld Revolution. Note that the tool will attempt to detect the patch on its own, and adjust the ASM accordingly; there's no define.

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