# BOWSIE - Better Overworld Sprite Insertion Engine
#### Version 1.21 by Arinsu/Ari

## Introduction
I love overworld sprites. I love them ever since I first played the 9th Vanilla Level Design Contest collaboration hack, with its elegant display of how they could be used not only to give more life to a map, but also as a way to have more interaction in the overworld area than just walking around and stuff. I loved them even more when Tob and I collaborated for the Overworld Design contest, and I took care of coding them.  
I _don't_ love that the only way to use them, for so long, was to either use carol's tool, which, how do I put this delicately, in my opinion is a BS tool 😭😂✌🏾, or have the patch used in VLDC9, with its unintuitive usage as a patch. So I decided to code a tool to streamline the process.  
I feel like this is what has been holding the widespread adoption of overworld sprites. My intention is to see more hacks with livelier overworlds.

## Usage
### The basics
Like _PIXI_ or any other tool for the game, _BOWSIE_ can be used in a headered Super Mario World ROM that has been expanded to at least 1MB, which can be achieved by saving a level in Lunar Magic. Additionally, _BOWSIE_ works out of the box with yoshifanatic's wonderful Overworld Revolution patch.  
The easiest way to use the tool is to run `bowsie.exe`. Set the path to your ROM and to your list file, and the tool will do everything for you.  
The list file has basically the same format as _PIXI_. First, a sprite number between 1 and 7F, then the filename of the sprite. One per line. No comments are supported, for now.  
When you modify your overworld, if you're using a Lunar Magic version prior to 3.60, __you need to rerun _BOWSIE___! Do not forget or else either the sprites won't show or the ROM will crash. This was fixed in Lunar Magic 3.60.

## Settings
_BOWSIE_ is designed to be a pretty customizable tool with the intention of being versatile, usable both by hackers who just want to insert any sprites without worry for anything else, by power users who would love to customize the inner workings of the tool and deploy it in various hacks without having to copy the tool, and by anyone in between.  
BOWSIE has two ways to parse its settings. The first is to pass them as traditional command line arguments, in the form `--<setting_name>=<value>`. The second is via an optional file named `bowsie_config.json`.  
There are several options, which might seem intimidating at first, but as you'll see here, they're pretty straightforward!
* `verbose`: this flag determines whether to display extra information about every sprite inserted and action taken within the tool, useful for debugging issues.  
  __Values__: `true` to display said info, `false` otherwise.  
  __Default__: `false`.
* `generate_map16`: this flag determines if _BOWSIE_ will attempt to parse OW sprite Map16 data for display in Lunar Magic. This info is stored in a separate JSON file. See below for more details.  
  __Values__: `true` to generate sprite Map16 data, `false` otherwise.
  __Default__: `true`.
* `meowmeow`: this flag tells _BOWSIE_ to enable _meOWmeOW_, its integrated extra bytes fixer.
  __Values__: `true` to enable _meOWmeOW_, `false` otherwise.
  __Default__: `true`.
* `slots`: this value controls the amount of sprites, per map, that you can have.  
  __Values__: any integer value between 1 and 24 (in a traditional overworld) or 32 (when using OW Revolution) is valid, altough it is possible to disable the upper bound check if you're absolutely sure of what you're doing (see below).  
  __Default__: `24` for a traditional overworld and `32` for OW Revolution ROMs.
 * `use_maxtile`: this flag tells _BOWSIE_'s ASM to enable _MaxTile_, an OAM allocation technology originally made by Vitor Vilela for SA-1.  
  __Values__: `true` to enable MaxTile, `false` otherwise. Note that this setting is hardcoded to `true` when you're using OW Revolution.  
  __Default__: `true`.

Additionally, there are two advanced settings.
* `custom_method_name`: this is the name for a custom overworld sprite handler. _BOWSIE_ will look for a file named `<value>.asm` in the `asm` directory, which it will apply. It's up to you to design said method and make it BOWSIE compatible. A custom method, `katrina`, is included by default.
  __Values__: any string is allowed. When passing a json settings file, set it to `null` to disable this feature.  
  __Default__: no custom method. Internally, a blank string.
* `bypass_ram_check`: this flag tells the tool to ignore the boundary checks on RAM addresses used by the overworld sprite systems. What this means is, this would allow you to, theoretically, have an unlimited amount of sprites (obviously up to console restrictions) per map.  
  __Values__: `true` makes _BOWSIE_ ignore potential RAM violations. `false` will keep a RAM check in place: in practice, this means throwing an error should `slots` be set to a value greater than 24 (traditional) or 32 (OW Revolution).  
  Do note that it is up to you to remap the sprite tables to adequate free RAM so you don't accidentally step over memory assigned to another purposes. _BOWSIE_ will __not__ do this for you. Check `defines.asm` for more info.  
  __Default__: `true`.

## Implementation details
### Coding a sprite
The tool looks for `init` and `main` labels to know where to add the pointers. You'd treat them like that `print "INIT "/"MAIN ",pc` in a _PIXI_ sprite. Unlike those though, you do not need to set the data bank: the system will do it for you (assuming you use an included one) and therefore there's no need for the `PHB : PHK : PLB : JSR sprite : PLB` wrapper. You can put your code directly below the labels. Remember to return with an `RTL`!  
The tool includes 14 shared subroutines, ten from the VLDC9 sprites; a distance routine by carol; a routine to erase a sprite by yoshifanatic; an adaptation of Akaginite's OAM size table write routine; and a basic interaction routine by me;. They work just like _PIXI_ (for practical purposes): just call the `%routine_name()` macro.

### The ASM base for the game
The code can be seen in the asm folder. For traditional overworlds, the method inserted is in `vldc9.asm`. For Overworld Revolution, the system is in `owrev.asm`. Additionally, there's a custom method included, Katrina's system, named `katrina.asm`.  
You will notice there's no traditional "defines" file, outside of method-specific defines. The tool generates them on the fly, based on the settings chosen.  
When using `vldc9` or `katrina`, the original overworld sprite loader at `$04F675` is replaced with the custom code. Everything those systems need fit in this space.  
When using `owrev`, the code is inserted in bank 4 freespace created by OW Revolution isself.

The tool puts its pointers in a free area in bank 4. By default, this is at `$04EF3E`. Here, the first four bytes are the tool's signature `0x00CAC705` (00cactus), to know whether to perform freespace cleanup in later executions; the pointers to the shared subroutines; the init routine pointers for all 127 sprites; and the main routine pointers. The tool finalises this by writing `0x555555` at the end, to know when to stop cleaning. Unused sprites set both pointers to an `RTL` in bank 4, namely `$048414`, to avoid crashes on accidental insertion.  
If I counted correctly, with the defualt subroutines and accounting for all the routine pointers, there's space for about 0 more routines (there's `0x1D` free bytes by default)
Obviously, a custom system would need to accound for where to run and where its code will go. Still, the defines generated might help you if you want to publish your system to the public!  
The way routines are inserted varies slightly with _PIXI_. _BOWSIE_ inserts every routine first, then creates the call macros which are simply a `JSL routine` instruction. What _PIXI_ does is put the routine inside a macro, _then_ give you the `JSL` when you call said macro. That's why _PIXI_ routines use macro labels and _BOWSIE_'s don't.

### The tool itself
Originally, I decided to code _BOWSIE_ in the latest (as of 2024) C++ standard: that is, [C++23](https://en.wikipedia.org/wiki/C%2B%2B23). This tool makes heavy use of both [named modules](https://en.wikipedia.org/wiki/Precompiled_header#Modules) and the \<print\> header introduced in the standard (well, modules are C++20, but I mean...). As of 2025, really the only compiler with good support for both of these features happens to be the Microsoft Visual C++ (_MSVC_) compiler: I'm sure _GCC_ outright doesn't support either feature, and for all I tried I couldn't get _LLVM_ to compile the standard library.
Thanks to Atari2.0's help, BOWSIE now uses C++20 and compiles with GCC and Clang just as well.

## Common questions
* __Q__: Why a separate sprite tool instead of _PIXI_ integration?  
  _A_: The reason is threefold.
  1. I already got overworld sprites working on _PIXI_, all the way back in 2018, and Jack and Tattletale refused to merge my pull request! So I figured the devs don't care and I never tried again.  
  Atari2.0, the current mantainer for _PIXI_, has given greenlight to potentially merge the tools in the future, but as of now, I'm not interested in doing so.
  2. I wanted to give the user more customization than what _PIXI_ offers, where you can't have multiple sprite systems; but also more ease, with the potential of passing one single JSON file for settings; and to motivate not copying the tool around if you have multiple hacks, only looking for the config file in your ROM path.
  3. Finally, I just wanted to see if this was something I was capable of. I came in knowing the bare essentials of C++, which I hadn't applied since 2020.
* __Q__: Why not carol's original overworld sprite tool? Or Alcaro and wiiq's update?  
  _A_: Because these tools are *so* old there's no use in going back and cleaning them. You're basically starting from scratch.
* __Q__: Can you include some sprites?  
  _A_: Sprites are better off in the sprites section of the website. That being said, as long as they're not accepted, you can check [this thread](https://smwc.me/t/128300) to find some.
* __Q__: Why all the options to have the code in someplace or whatnot?  
  _A_: So you can run the tool in whichever way you like.
* __Q__: Why MSVC?  
  _A_: Bevause Clang refused to compile the standard library as a module and I didn't even attempt again after settling.
* __Q__: For the love of God why did you insist in C++23?  
  _A_: I already told you because I wanted to learn modules!  
  Look, it would've been easier for me to code this in Python, but I dislike having to pass the script around instead of a binary executable.

## Thank you
* To Lui37, Medic, and the entire VLDC9 team for the original implementation of the patch.
* To Katrina for her alternative implementation which was proposed in [this](https://smwc.me/t/94775) thread.
* To yoshifanatic for all of his pointers and willingness to rearrange OW Revolution so _BOWSIE_ and said patch work seamlessly.
* To GrenCaret/Green and Stivi, who contacted me during my hiatus to check on me. The former doing so encouraged me to return, especially because I didn't even know them yet they checked in on me. The latter asked me for the implementation of the older overworld sprites patch, and after chatting a bit with him, I got enough motivation to do this tool.
  * And of course you too Ringo/Mirann and Burning Loaf/Bench, but you already know that. 😉
* Contributors:
  * Atari2.0
  * yoshifanatic