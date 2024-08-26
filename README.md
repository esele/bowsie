# BOWSIE - Better Overworld Sprite Inserting Engine
_Version 1.0 by Erik_
## Introduction
I love overworld sprites. I love them ever since I first played the 9th Vanilla Level Design Contest collaboration hack, with its elegant display of how they could be used not only to give more life to a map, but also as a way to have more interaction in the overworld area than just walking around and stuff. I loved them even more when Tob and I collaborated for the Overworld Design contest, and I took care of coding them.\
I _don't_ love that the only way to use them, for so long, was to either use carol's tool, which, how do I put this delicately, in my opinion is a BS tool 😭😂✌🏾, or have the patch used in VLDC9, with its unintuitive usage as a patch. So I decided to code a tool to streamline the process.\
I feel like this is what has been holding the widespread adoption of overworld sprites. My intention is to see more hacks with livelier overworlds.

## Usage
### The basics
Like _PIXI_ or any other tool for the game, _BOWSIE_ can be used in a headered Super Mario World ROM that has been expanded to at least 1MB, which can be achieved by saving a level in Lunar Magic. However, as this is a tool which deals with the overworld, to ensure the correct compatibility and to avoid any potential crashes after further editions of the ROM, you must save the OW at least once to be able to use this tool. You must also insert a custom overworld sprite. To do this, press Ctrl-Insert in Lunar Magic while on the sprite edit mode on the overworld.\
The easiest way to use the tool is to run bowsie.exe. Set the path to your ROM and to your list file, and the tool will do everything for you.\
The list file has basically the same format as _PIXI_. First, a sprite number between 1 and 7F, then the filename of the sprite. One per line. No comments are supported, for now.\
When you modify your overworld, __you need to rerun _BOWSIE___! Do not forget or else either the sprites won't show or the ROM will crash.

### Coding a sprite
I included the sprites I made for Tob and I's entry on that old OWDC and updated them for your reference.\
The tool looks for `init` and `main` labels to know where to add the pointers. You'd treat them like that `print "INIT "/"MAIN ",pc` in a _PIXI_ sprite. Unlike those though, you do not need to set the data bank: the handler will do it for you (assuming you use an included one ofc) and therefore there's no need for the `PHB : PHK : PLB : JSR sprite : PLB` stuff. You can put your code directly below the labels. Remember to return with an `RTL`!\
I included 10 shared subroutines, nine from the VLDC9 sprites and a basic interaction routine by me, so you don't have to paste them in every sprite. They work just like _PIXI_ (for practical purposes): just call `%routine_name()`.

## Settings
_BOWSIE_ is designed to be a pretty customizable tool with the intention of being versatile, usable both by hackers who just want to insert any sprites without worry for anything else, by power users who would love to customize the inner workings of the tool and deploy it in various hacks without having to copy the tool, and by anyone in between.\
As mentioned in the Usage section, BOWSIE reads its configuration from a file named `bowsie_config.json`. There are several options, which might seem intimidating at first, but as you'll see here, they're pretty straightforward!
 * `verbose`: this flag determines whether to display extra information about every sprite inserted and action taken within the tool, useful for debugging issues.\
   __Values__: `true` to display said info, `false` otherwise.
 * `generate_map16`: this flag determines if _BOWSIE_ will attempt to parse OW sprite Map16 data for display in Lunar Magic. This info is stored in a separate JSON file. See below for more details.\
   __Values__: `true` to generate sprite Map16 data, `false` otherwise.
 * `slots`: this value controls the amount of sprites, per map, that you can have.\
   __Values__: any integer value between 1 and 24 is valid, altough it is possible to disable the upper bound check if you're absolutely sure of what you're doing (see below).
 * `method`: this flag tells _BOWSIE_ the insertion system to use.\
   __Values__: the tool recognizes three methods out the box:
    * `vldc9` refers to the system used in, as its name says, the compilation for the 9th Vanilla Level Design Contest. It's the default method and, for most intents and purposes, the one you use.
    * `katrina` refers to a modification of the VLDC9 method with a different RAM use configuartion. You can learn more below.
    * `custom` is a special flag which allows the user to specify its own implementation of an overworld sprite insertion system different from the ones above. This value is only intended for coders. Its purpose is to allow for the programming of any new system which you desire, tailored to your own needs, and to be able to use it with the tool itself instead of recurring to the creation of manual data tables and the insertion of various files by hand, which can get messy pretty quick.

   If you're interested in knowing the differences, please read the Implementation details section.
 * `replace_original`: this flag allows you to decide whether to keep the original overworld sprite system in place or replace it with the implementation you have chosen. Normally, the system installs itself over the original handler, resulting in being uncapable of using the original sprites. \
   __Values__: `true` erases the vanilla game handler. `false` tells _BOWSIE_ to keep the original system intact. In this case, where the code for the tool goes depends entirely of the `omtre_detect` setting. \
   Do note that one of my goals is to eventually disassemble every vanilla sprite and convert it for use with _BOWSIE_. After that is done, the intent is to remove this setting, so you can choose which sprites from the original game you want to use.
 * `omtre_detect`: this flag tells _BOWSIE_ to install the chosen system in the vanilla freespace created by the Overworld Player Sprite Expansion (_OPSE_) patch, if it's detected in the ROM.\
   __Values__: `true` to install the code in the _OPSE_-created freespace. `false` to put it someplace else: either overwriting the vanilla handler or in expanded freespace, should `replace_original` be set to false.

Additionally, there are two advanced settings.
 * `custom_dir`: this is the path to the asm file for your custom overworld sprite handler, in case `method` is set to `custom`. You can leave it `null` if you are not implementing a different handler.
 * `bypass_ram_check`: this flag tells the tool to ignore the boundary checks on RAM addresses used by the overworld sprite systems. What this means is, this would allow you to, theoretically, have an unlimited amount of sprites (obviously up to console restrictions) per map.\
   __Values__: `true` makes _BOWSIE_ ignore potential RAM violations. `false` will keep a RAM check in place: in practice, this means throwing an error should `slots` be set to a value greater than 24.\
   Do note that it is up to you to remap the sprite tables to adequate free RAM so you don't accidentally step over memory assigned to another purposes. _BOWSIE_ will __not__ do this for you. Check `defines.asm` for more info.
## Implementation details
### The ASM base for the game
The code can be seen in the asm folder. For the `vldc9` setting, its `vldc9.asm`. For Katrina's system, it's `katrina.asm`. You will notice there's no defines file. The tool generates them on the fly, based on the settings chosen.\
Depending on the settings chosen will be where the handler is put.
 * If you're _not_ keeping the original system, the original loader at `$04F675` is replaced with the custom code. Everything those systems need fit in this space.
 * If you're keeping the original sprites but have _OPSE_ installed, part of the code will be put within the vanilla freespace the patch creates. Unfortunately, it's too little (around 60 bytes less than needed), so part of the handler needs to be put within the expanded area. The loader routine now is at `$048C8B` and the main call is at `$04F708`.
 * If you are keeping the original sprites and don't use _OPSE_, everything goes in freespace. The main call is also at `$04F708`.

The tool puts its pointers in a free area in bank 4. By default, this is at `$04EF3E`. Here, the first four bytes are the tool's signature `0x00CAC705` (00cactus), to know whether to perform freespace cleanup in later executions; the pointers to the shared subroutines; the init routine pointers for all 127 sprites; and the main routine pointers. The tool finalises this by writing `0x555555` at the end, to know when to stop cleaning. Unused sprites set both pointers to an `RTL` in bank 4, namely `$048414`, to avoid crashes on accidental insertion.\
If I counted correctly, with the defualt subroutines and accounting for all the routine pointers, there's space for about 13 more routines (there's `0x29` free bytes by default)
Obviously, a custon system would need to accound for where to run and where its code will go. Still, the defines generated might help you if you want to publish your system to the public!\
The way routines are inserted varies slightly with _PIXI_. _BOWSIE_ inserts every routine first, then creates the call macros which are simply a `JSL routine` instruction. What _PIXI_ does is put the routine inside a macro, _then_ give you the `JSL` when you call said macro. That's why _PIXI_ routines use macro labels and _BOWSIE_'s don't.

### The tool itself
As I am a pretty nitpicky person, I decided to code _BOWSIE_ in the latest (as of 2024) C++ standard: that is, [C++23](https://en.wikipedia.org/wiki/C%2B%2B23). This tool makes heavy use of both modules and the \<print\> header introduced in the standard (well, modules are C++20, but I mean...). As of March, really the only compiler with good support for both of these features happens to be the Microsoft Visual C++ (_MSVC_) compiler: I'm sure _GCC_ outright doesn't support either feature, and for all I tried I couldn't get _LLVM_ to compile the standard library.\
If you're interested in modifying and compiling the tool but are not familiar with this standard, I suggest checking up on the [documentation](https://learn.microsoft.com/en-us/cpp/cpp/tutorial-named-modules-cpp) to get up to speed. You can compile the tool by installing the latest MSVC version and running\
`cl /Febowsie /std:c++latest /EHsc /nologo /W2 /reference "std=std.ifc" /reference "rapidjson=rapidjson.ifc" /reference "asar=asar.ifc" bowsie.cpp std.obj rapidjson.obj asar.obj`\
in the x86 Native Tools Command Prompt. This, of course, on Windows.\
If you're wondering _why_ I chose to code the tool this way, well, it was to try something new. I wanted to code the tool in [Julia](https://julialang.org/), a high level programming language aimed at scientific computing which I used to implement the code I needed for my Bachelor of Sciences' thesis. In theory, Julia is able to compile to a native code executable; in practice, this results in distributing a program with like, 90 DLLs. I really dislike C++ and the only way I saw to motivate myself to do this was by challenging myself to learn the newest tricks of the standard.\
If you believe the tool is poorly coded, that's because I'm not a programmer: I'm a mathematician who happens to like coding. That's how I got my job. Condescendence aside, if you see area for improvement, please submit a pull request with your improvements. Who knows, perhaps this ends up like the ship of Thesseus lmao

### Future features?
One feature I'd like to add is per-map sprites, similar to per-level sprites. Of course, I'd also would like that every time the overworld is saved, Lunar Magic kept the pointers where they are if you don't add more sprites, but I mean, I asked for this [5 years ago](https://smwc.me/1502976) and it didn't happen...\
You can also suggest more features if you think of something. 😃

## Common questions
 * __Q__: Why a separate sprite tool instead of _PIXI_ integration?\
 _A_: The reason is threefold. The first is because I already got overworld sprites working on _PIXI_, all the way back in 2018, and Jack and Tattletale refused to merge my pull request! So I figured the devs don't care and I never tried again. The second, I wanted to give the user more customization than what _PIXI_ offers, where you can't have multiple sprite systems; but also more ease, with one single JSON file for settings instead of multiple defines in (potentially separate) ASM files; and to motivate not copying the tool around if you have multiple hacks, only looking for the config file in your ROM path. Finally, I just wanted to see if this was something I was capable of. I came in knowing the bare essentials of C++, which I hadn't applied since 2020.
 * __Q__: Why not carol's original overworld sprite tool?\
 _A_: Because I hate it. But who knows, perhaps you can use them together? I didn't try.
 * __Q__: Can you include the VLDC9 sprites?\
 _A_: Perhaps, but I think they're better off in the sprites section of the website should the tool be approved by the community.
 * __Q__: Why all the options to have the code in someplace or whatnot?\
 _A_: So you can run the tool in whichever way you like. A disadvantage of the original system for some might have been the fact that you were forced to lose the original sprites.
 * __Q__: Why MSVC?\
 _A_: Bevause Clang refused to compile the standard library as a module and I didn't even attempt again after settling.
 * __Q__: For the love of God why did you insist in C++23?\
 _A_: I already told you because I wanted to learn modules!\
  Look, it would've been easier for me to code this in Python, but I dislike having to pass the script around instead of a binary executable.

## Thank you
 * To Lui37, Medic, and the entire VLDC9 team for the original implementation of the patch.
 * To Katrina for her alternative implementation which was proposed in [this](?) thread.
 * To GrenCaret/Green and Stivi, who contacted me during my hiatus to check on me. The former doing so encouraged me to return, especially because I didn't even know them yet they checked in on me. The latter asked me for the implementation of the older overworld sprites patch, and after chatting a bit with him, I got enough motivation to do this tool. Thank you guys. You're real ones. 🥺
   * And of course you too Mirann and bench, but you already know that. 😉