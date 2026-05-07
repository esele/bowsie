BOWSIE v1.21
---
### Fixes
- Fixed the incorrect extra byte offset being considered when running meOWmeOW on OW Revolution ROMs. This meant meOWmeOW always ran in them. Mostly inconsequential, but...
- Fixed a meOWmeOW bug when using OW Revolution, where an incorrect offset was being added to sprite data when reading an `0xFF` byte not followed by a `0xFE` (terminator). This caused the data to misalign and BOWSIE could even crash. Thanks to Stivi for discovering this and helping me reproduce! (that after a couple hours of thinking his ROM was cursed LOL)

BOWSIE v1.20
---
### Features
#### Overworld Revolution
- BOWSIE no longer sets the "Alternate sprite system" bit, as Overworld Revolution can now detect an alternate system on its own.
- Now a OR ROM won't crash after reapplying said patch when BOWSIE is in use.
- Made the freespace BOWSIE requires a fixed position: `$04F800` for the code and `$04FC00` for its signature and pointers. In comparison, VLDC9 places its code for the regular overworld at `$04F625` and `$04EF3E` respectively.

#### MaxTile
- Added support for the dynamic OW player when using Sonikku's 32x32 player patch.
  - When using said patch, BOWSIE now requires the latest version to be inserted, otherwise the tool will *not* insert.
- Added a setting to select a custom priority buffer at `%get_draw_info()`, by setting the highest bit in A next to the number of tiles, i.e.,

```
    LDA #$8xxx
    LDY #$<priority>
    %get_draw_info()
```  

#### BOWSIE
- BOWSIE can now use a dynamically linked Asar library, for further compatibility with Callisto.
- Given how outdated it is, BOWSIE now ships without the contiguous table `katrina.asm` system.

### Fixes
- Fixed BOWSIE not cleaning up correctly when using Overworld Revolution, causing freespace leaks and, in the worst case, the tool to get stuck.
- Fixed the incorrect mapper type being selected in large SA-1 ROMs. This noticeably caused BOWSIE to completely crash when running meOWmeOW. Thanks to Mario90 for reporting this and helping me test the fix! Embarrassing to say, it was a typo error...
- Fixed a math issue dating from the old PIXI implementation days, where sprites were sometimes misaligned by 4 pixels in the X axis. This had been reported by everyone and their mother, but, special thanks to CroNo486 for helping me confirm it is fully gone now!
- Fixed the overworld player sprite wrapping incorrectly in maps which used the Widescreen Overworld patch. Thanks to CroNo486 for noticing this!
- Fixed a regression issue with the Map16 tooltip generator were the X flip-only and Y flip-only cases were swapped. Thanks to CroNo486 for reporting this!

BOWSIE v1.16
---
### Features
- MaxTile: Added preliminary support for Sonikku's 32x32 player patch.

### Fixes
- Fixed an obscure issue and crash with the Map16 generator where it was reading values which had already been popped out of the vectors containing tile information multiple times. For some reason this had never caused any crashes before, neither in debug nor release builds. In fact, I'm not sure why it suddenly started crashing now. In any case, the previous code was wrong.
- Fixed a regression issue where the speed routines were doing the sign inversion incorrectly. Thanks to Ramon for noticing this!
  - This does mean the VLDC9 sprites might slowly go out of bounds though, due to faulty programming.
- Fixed BOWSIE not assembling on Lunar Magic versions older than 3.51, given the extra bytes size table not being properly disabled. Thanks go to Stivi and CroNo486 for reporting this!
- Fixed the tool's documentation not crediting Ari. Instead it credited some other guy who died. 🏳️‍⚧️

BOWSIE v1.15
---
### Fixes
- Fixed the VLDC9 system potentially erasing some code used by AddmusicK, causing a crash if you ran BOWSIE in a ROM using said tool. This was a regression issue introduced in BOWSIE v1.10. Thanks to Moltz for reporting this and helping me with figuring out what exactly was causing the crash!

BOWSIE v1.14
---
### Fixes
- Actually fixed the extra byte table size changes... Oops!
- Fixed the incorrect mapper type being selected in BOWSIE and meOWmeOW. This was very apparent in 4MB+ SA-1 ROMs, as Lunar Magic can put some data in banks 80+, which the tool was incorrectly assuming as a FastROM mirror. Originally, multiple users (chief amongst them Stivi and simon.caio) had reported this bug to me, but I never understood what was causing it... until now, because I so happened to be making an overworld in a SA-1 ROM myself. Still, thanks to the two of them!

BOWSIE v1.13
---
### Fixes
- Fixed temporary binary files not being erased.
- Fixed BOWSIE and meOWmeOW not accounting for the differences in the extra byte table when using Lunar Magic 3.60 and newer.

BOWSIE v1.12
---
### Fixes
- Fixed meOWmeOW not accounting for an uninitialized sprite data list of `0x000000` in addition to the `0xFFFFFF` one. Thanks go to Stivi for reporting this!

BOWSIE v1.11
---
Of course I couldn't make an update without breaking a couple things LOL

### Features
- Made `%random_range()` more similar to PIXI's own `%Random()`.

### Fixes
- Fixed inversion bias in the position update routines, which existed since the VLDC9 days. Thanks go to Stivi for reporting this and Ice Man for helping me test!
- BOWSIE did not invoke the SA-1 chip on sprite init, which was pretty noticeable when using routines which use SA-1's registers (eg. the VLDC9 Eerie which called `%random_range()`). Thanks to Ice Man for reporting this!

BOWSIE v1.10
---
### Features
Due to the sheer amount of changes, the regular changelog is now broken down in subsections of each change, instead of per overworld system.  

#### Extra bytes and meOWmeOW
- Added support for arbitrarily sized extra bytes, which you may know from level sprites as the "Extension" field. Thanks to FuSoYa for adding this feature to Lunar Magic!
  - This feature needs Lunar Magic 3.51 or newer to work.
    - Note that LM 3.51 has a bug where sprite ID 00 is considered valid in the extra bytes table, so therefore BOWSIE code is currently shifted to keep this phantom sprite in mind. This is subject to change once FuSoYa fixes that, so sprite ID 00 is still unusable.
  - BOWSIE will now enable two 8-bit extra bytes to form a full-sized extra byte, the way OW Revolution sprites work.
  - Extra bytes used are controlled by the `!extra` define in a sprite's ASM file. For example, `!extra = 5` will enable five extra bytes.
  - Added two tables, `!ow_sprite_extra_1` and `!ow_sprite_extra_2`, to access four 8-bit extra bytes.
  - You're limited to eight extra bytes.
- Added *meOWmeOW*, which will detect changes in extra bytes, similar to Akaginite's *MeiMei*.
  - Added the `meowmeow` setting, which allows you to enable or disable *meOWmeOW* if so desired.
  - *meOWmeOW* also works with OW Revolution. Remember that it expects sprites with extra bit set to 1 and sprite numbers `01-7F`.

#### MaxTile
- Added suport for SA-1 *MaxTile*.
- Added support for OW Revolution's *MaxTile*.
- Added support for a custom version (henceforth referred to as the *BOWSIE implementation*) of *MaxTile*, which implements said OAM slot allocation system, and rewrites the border player, the sprite border surrounding said player, the overworld player sprite, and the lives exchanger, to use the BOWSIE implementation.
- Added `use_maxtile`, which enables the use of the *MaxTile* allocation system for OAM slots.
  - This setting redirects `%get_draw_info()` to *MaxTile*'s routine to acquire slots with a determined priority.
  - OW Revolution ROMs *always* use *MaxTile*.
  - SA-1 can be set to not use *MaxTile* if you prefer the regular way around graphics routines. After all, SA-1 has Legacy mode and that arranges OAM at the very end either way.
  - The BOWSIE implementation, just like the OW Revolution implementation, does not implement Legacy mode, as this is very taxing outside SA-1.
  - The BOWSIE implementation does not support the event path fade effect.
  - The BOWSIE implementation *cannot* be disabled once installed!
- Note that *MaxTile* graphics routines move OAM slots with `INY`, *not* `DEY`!

#### Shared subroutine improvements
- Added `%finish_oam_write()`, an adaptation of Akaginite's routine to set tile sizes and the X position high byte in the OAM size table. While normally this isn't necessary, due to the side border, using this routine fixes the odd quirks of sprites disappearing when using Medic's Widescreen Overworld patch.
- Added `%erase_sprite()`. In it's simplest form, it just sets a sprite number to zero, but when using OW Revolution, it also marks the sprite to be reloaded correctly.
- `%sub_horz_pos()` and `%sub_vert_pos()` now store the distance between player and sprite in scratch RAM `$0E`.
- `%sub_offscreen()` now has more tolerance in how far off screen a sprite can be before despawning. This is more noticeable for big OW Revolution maps.

#### Macros
- Added three helper macros for hybrid *MaxTile* and regular graphics routines: `%maxtile_set_databank()`, which preserves and sets the data bank to the bank of the OAM buffer; `%maxtile_clear_databank()`, which restores the data bank, and `%maxtile_get_tilesize_index()`, which loads the index for the high table in Y. The former two do nothing when MaxTile is disabled. The latter uses the internal pointer to the tilesize table when MaxTile is enabled, and a traditional right shift by two when disabled.
- Split the shadow macros between those which write to the OAM size table and those which don't, intended for use with `%finish_oam_write()`.

#### New labels and definitions
- In addition to regular `!ow_sprite_*` defines for tables, you can now use `ow_sprite_*` *labels* for tables. Not only are they there in case you prefer the latter: they are intended for hybrid graphics routines, so Asar's label parser assigns the adequate size.
- Added `oam_buffer` and `oam_tilesize_buffer` structs, intended for graphics routines.
  - `oam_buffer` includes four labels: `x_pos`, `y_pos`, `tile` and `props`.
  - `oam_tilesize_buffer` includes one label: `tile_sx`.
- Added `!next_oam_slot`, `!next_oam_tilesize_slot` and `!adjacent_oam_slot`, helper defines for hybrid graphics routines.
  - `!next_oam_slot` and `!next_oam_tilesize_slot` move to the next allocated OAM slot.
  - `!adjacent_oam_slot` is intended for writes to the next allocated OAM slot when not increasing the buffer.
- Added a `maxtile_get_slot` label, to jump to the subroutine to acquire OAM slots using *MaxTile*, when you don't want to use `%get_draw_info()`.
- The macros above mentioned include `bank` statements so Asar's label parser can optimize as necessary. This is especially required to not abuse size indicators in hybrid graphics routines.

#### Lunar Magic
- BOWSIE now sets a bit that indicates Lunar Magic to stop overwriting code in the `$04F625`-`$04F6F8` and `$04F8A6`-`$058000` areas. This will allow you to finally save the overworld without having to re-run BOWSIE!
  - Note that this feature is not yet available as of Lunar Magic 3.51 and is awaiting a future release.

#### OW Revolution
- Added a missing table, `!ow_sprite_load_index`, which is the equivalent to level sprites' `!161A`: the index to the table used to determine whether a sprite should respawn.
- Sprites now show when entering the overworld in fade-ins.

#### Contiguous table/Katrina's sprite system
- This system has been "unofficialized". BOWSIE still ships with `katrina`, but now you have to set the `custom_method_name` setting to `katrina` to use it.

#### BOWSIE
- The codebase of BOWSIE suffered several changes.
  - BOWSIE now includes CMake lists: from now on, all releases will be built using CMake with Clang as the default compiler and Ninja as the generator. Check the readme for more details.
  - The main branch of BOWSIE can now be compiled with C++20. A branch planned for the future still uses modules.
- For extra organization, you can now insert sprites in subfolders, as well sprites with spaces in their filenames.
- You can now pass BOWSIE settings as arguments to the program directly, in case you don't wish to use `bowsie-config.json`. Run `bowsie -h` for more info.
- When not choosing a custom method, BOWSIE now selects the adequate insertion method to use.
- In addition to the two points above, BOWSIE now comes bundled with default settings. These are:
  - Verbose mode disabled.
  - Map16 generation enabled.
  - meOWmeOW enabled.
  - 32 slots when using OW Revolution and 24 otherwise.
  - MaxTile enabled.
- Added support for 6 and 8MB SA-1 ROMs.
- BOWSIE now enables aggressive direct page (`optimize dp always` with `dpbase` set to the `!dp` define) and address size (`optimize address mirrors`) optimization by default.

#### Deprecated features
- Removed the support for the vanilla sprite system, in anticipation of yoshifanatic's disassemblies. Please use those instead, they don't have the odd restriction the vanilla sprites have either way.
  - `omtre_detect` was also removed as a consequence, as everything fits in vanilla freespace now!


### Bug Fixes
- Fixed being able to pass negative values to the Map16 generator, as well as a crash that occurred as a result of invalid memory allocation when not drawing any tiles. With the compiler change, this crash was made apparent.
- Fixed a crash in the VLDC9 system when using sprite numbers `55-7F` -- the high byte of the index to the pointers was being lost.
- Fixed reading garbage when the pointer to sprite data was not inserted (equal to `$000000`). This was a regression issue introduced when I cleaned the code in my modified PIXI builds.
- Fixed a freespace leak in bank 4 when using OW Revolution.
- Fixed the vanilla shadow macro using a non-assigned OAM slot. Thanks to simoncaio for reporting this and helping me test the fix!
- Fixed `%interact()` not taking a sprite's Z position into account. Thanks to simoncaio for making me realize this was the case!

BOWSIE v1.10 RELEASE CANDIDATE 2
---
### Features
- Added a macro library. By default it includes two macros:
  - A macro to draw a shadow, like the VLDC9 sprites which use it.
  - A macro to draw the vanilla game's shadow.

BOWSIE v1.10 RELEASE CANDIDATE 1
---
### Features
- The tool now pauses after an error or when sprites insert successfully... [Turns out regular people run BOWSIE from outside the console just like they'd do PIXI huh](https://i.kym-cdn.com/photos/images/original/002/086/808/90f.gif)

___OW Revolution___
- Upgraded to Overworld Revolution v1.1.1.
- `slots` can now be up to 32 when using `owrev`, without needing to disable RAM checks.
  - Due to this, some tables have moved RAM address locations.
- `owrev` now inserts its main handler at the pointer located at `$0480D2` instead of overwriting OW Revolution code.
- `owrev` now hacks `$02A861` directly for sprite load instead of overwriting OW Revolution code.
- Added the sprite table `!ow_sprite_props`, corresponding to OW Revolution's `!RAM_SMW_OWSpr_Properties`. It's used to know when to process an sprite even if it's offscreen. Due to overworld map sizes, it's pretty necessary.
  - Updated `%sub_offscreen()` to account for this fact.
- Removed BOWSIE pointers for previously overwritten functions.
- The bank 4 freespace finder now stops if we reach BOWSIE's default area. This is done in advance of potential future OW Revolution upgrades, as to know when to update the `JSR : NOP` to an `autoclean JSL`.
- Special thanks to yoshifanatic for all his support to make this update possible. 😃

Overworld Revolution support can now be considered mature.

___Non-OW Revolution___
- __Katrina__: added `!bowsie_contiguous` so the user can tell when a sprite is using contiguous sprite tables, like Katrina's system does.
  - Updated `%spawn_sprite()` to account for this fact.

The following features apply to both VLDC9 and Katrina's systems.
- Separated the main sprite handler into its own separate function.
- Sprites now appear during fade-in instead of awkwardly appearing out of nowhere.
### Fixes
- __OW Revolution__: now the extension bytes are set for all 256 sprites with extra bit 1, instead of just the first hundred. Oops!
- __VLDC9/Katrina__: fixed RAM addresses not clearing on level load. This was very noticeable with, eg. Yoshi spawning turning around.
- Changed `warnpc` to `assert pc()` and removed `math pri off`, in advance of Asar 2.0.
- Fixed an issue where BOWSIE couldn't find the ASM and sprite directory when calling the tool using relative paths.

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
- Added experimental support for yoshifanatics' Overworld Revolution. Note that the tool will attempt to detect the patch on its own.  
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