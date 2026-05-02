# **MSX2DAAD**

**Content index**

- [Description](#description)
- [License](#license)
- [Download](https://github.com/nataliapc/msx2daad/releases)
- [Compilation](#compilation)
- [Create your own Adventure: Sources and Compilers](#create-your-own-adventure-sources-and-compilers)
- [Our Wiki pages with articles and tutorials](https://github.com/nataliapc/msx2daad/wiki)
- [Supported languages](#supported-languages)
- [Image format & transparency (since v3.0.0)](#image-format--transparency-since-v300)
- [MSX graphical modes & limitations](#msx-graphical-modes--limitations)
- [Aditional tools in /bin folder](#aditional-tools-in-bin-folder)
- [External tools](#external-tools)

***

### Description

This project is a **DAAD** interpreter created from scratch for **MSX2**/**MSX2+** systems and using the graphical capabilities of this machines.

**DAAD** is a multi-machine and multi-graphics adventure writer, enabling you to target a broad range of 8-bit and 16-bit systems.  
You can see the classic interpreters [here](https://github.com/daad-adventure-writer/daad).

**MSX2DAAD** is also compatible with [**DRC**](https://github.com/daad-adventure-writer/DRC/wiki) compiler, and [**Maluva DAAD extension**](https://github.com/Utodev/MALUVA/wiki) emulating his new functionalities to the classic interpreters for **DAAD V2**:

- `XPICTURE`: Load bitmap images from disk.
- `XLOAD`/`XSAVE`: To load/save your gameplay from/to disk.
- `XMES`: Use of external texts in a disk file, providing 64kb of additional compressed texts and leaving more free *RAM* memory to create your adventure.
- `XPLAY`: Play music using a simplified [MML](https://en.wikipedia.org/wiki/Music_Macro_Language) string, the same format of PLAY instruction from MSX1 Basic.

...and some others.

Starting with **v3.0.0**, MSX2DAAD also loads **DAAD V3** databases natively (the DDB version is auto-detected at load time). V3 adds three native condacts (`XMES`, `INDIR`, `SETAT`), new flag-53 bitmask semantics for the parser/object subsystem, and changes the behaviour of `PAUSE 0`, `SYNONYM` and `EXTERN`. Pre-v3.0.0 V2 adventures keep working without changes. See the [V3 sections in the wiki](https://github.com/nataliapc/msx2daad/wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference#daad-v3-native-condacts) for the full list.

![](https://www.eslamejor.com/img/msx2daad_sample.png) 

***

### License

There is a LICENSE file you can read. You have rights to use, copy, modify, etc, this interpreter in your own games. But if you do a commercial use of it, you must send me a copy of your game.  
Contact email: natypclicense@gmail.com

***

### Compilation

You can download the last binaries release, or compile the binaries yourself.

The build is dockerized: it uses **SDCC 4.5.0** inside `nataliapc/sdcc:4.5.0`, so the only host requirements are **Docker**, **PHP** (for `bin/precomp.php` and `bin/imgwizard.php`), and **openMSX** (for `make test`). No local SDCC install is needed.

To clean and compile the msx2daad\.com:
> make clean all

To test the /dsk folder content with openMSX (you need to add at least your DAAD\.DDB to /dsk):
> make test

To build all 14 release variants (EN/ES × SC5/6/7/8/10/12 + transcript) under `package/`:
> make package

The default build enables `-DDAADV3` (DAAD V3 + V2 compatibility). Almost every other choice is selectable via `-D` flags in `CXXFLAGS`: language (`-DLANG_EN`/`-DLANG_ES`), screen mode (`-DSCREEN=5|6|7|8|10|12`), font width (`-DFONTWIDTH=6|8`), and per-codec opt-out for the V2 image format (`-DCOMPRESSOR_{RAW,RLE,PLETTER,ZX0}`) — defining a subset trims the unused decoders out of the `.com`.

There is also a unit-test suite (Dockerized too) under `unitTests/`:
> cd unitTests && make all && make test

It produces 11 binaries (~442 OK / 0 FAIL / 16 TODO) covering condacts, parser, V3 paths, DOS library and engine internals.

***

### Create your own Adventure: Sources and Compilers

To create your own adventure you need a text source file (**.SCE** or **.DSF**), with the definition of your game, and must be compiled to **DAAD** tokens (**.DDB** file) using a compiler. The **.SCE** files are used by the original [**DC**](https://github.com/daad-adventure-writer/daad) compiler, that needs [DOSBOX](https://www.dosbox.com/) and source files using *CP437* encoding, so we discourage the use of this files.

We recommend the use of the new [**DRC**](https://github.com/daad-adventure-writer/DRC/wiki) compiler (for **.DSF** files). The source file must be encoded with *Windows-1252* or *ISO-8859-1* charsets.  
The **DSF** format is an adventure text source very similar to **SCE** but improved in several ways to create the **.DDB** compiled file.

There are empty templates in several languages to start your adventure:

- [English DSF blank template](https://github.com/daad-adventure-writer/DRC/blob/master/BLANK_EN.DSF)
- [Spanish DSF blank template](https://github.com/daad-adventure-writer/DRC/blob/master/BLANK_ES.DSF)

The compiled **DDB** file must be renamed to **DAAD.DDB** and added to the disk where you have the interpreter (**MSX2DAAD.COM**) and the font image (**FONT.IM8** for Screen8, etc...).

To learn more about **how to create your own adventure** your can:

-  Read our [**Wiki pages with several articles**](https://github.com/nataliapc/msx2daad/wiki) about **DAAD** and **MSX2DAAD**.
- Also you can follow this great [**DAAD Tutorial for beginners**](https://medium.com/@uto_dev/a-daad-tutorial-for-beginners-1-b2568ec4df05) writed by the author of the [**DRC**](https://github.com/daad-adventure-writer/DRC/wiki) compiler.

![](https://www.eslamejor.com/img/msx2daad_tutorial.png) 

***

### Supported languages

- English
- Spanish

***

### Image format & transparency (since v3.0.0)

Pictures are converted from standard MSX `.SCx` files to MSX2DAAD's chunked container with `bin/imgwizard.php` (current version **1.4.2**). The canonical command is now **`cx[l]`** (chunked V2 format with V9938 streaming via port `#9B`); the legacy `c`/`cl`/`s` commands still work but are marked **DEPRECATED** and emit a warning — new pictures should use `cx`/`cxl`.

`cx[l]` highlights:

- Four codecs per `V9938CmdData` chunk: **RAW (0)**, **RLE (1)**, **Pletter (2)**, **ZX0 (3)**. ZX0 is usually the smallest for monochrome / sparse data; Pletter remains a good general-purpose choice.
- **Transparency** via `--transparent-color=N` (2-pass `LMMC|AND` + `LMMC|OR`) on SC5/6/7/8.
- **SC10 (`.SCA`) transparency** uses the per-pixel YJK+YAE A flag; SC12 (pure YJK) is intentionally not supported.
- **Fixed-position images** via `--fixed=X,Y` (emits a `FIXEDIMG` chunk with global offsets).

See the [features-and-limits wiki](https://github.com/nataliapc/msx2daad/wiki/MSX2DAAD-Wiki:-MSX2DAAD-features-and-limits) for the full chunk table and per-mode caveats.

***

### MSX graphical modes & limitations

#### **Screen 5 (MSX2)**

- 256x212 (16 paletted colors from 512 GRB333)
- 42x26 characters (6x8 font)
- Color 0: Always PAPER (default: black)
- Color 1-14: For bitmap images
- Color 15: Always INK (default: white)
- Default EGA palette.
- Palette changes (using GFX condact) modify text and bitmap colors.
		
#### **Screen 6 (MSX2)**

- 512x212 (4 paletted colors from 512 GRB333)
- 85x26 characters (6x8 font)
- Color 0: Always PAPER (default: black)
- Color 1-2: For bitmap images
- Color 3: Always INK (default: white)
- Default Amber palette (4 amber tones).
- Palette changes (using GFX condact) modify text and bitmap colors.

#### **Screen 7 (MSX2)**

- 512x212 (16 paletted colors from 512 GRB333)
- 85x26 characters (6x8 font)
- Color 0: Always PAPER (default: black)
- Color 1-14: For bitmap images
- Color 15: Always INK (default: white)
- Default EGA palette.
- Palette changes (using GFX condact) modify text and bitmap colors.

#### **Screen 8 (MSX2)**

- 256x212 (256 fixed colors GRB332) **[*DEFAULT MODE*]**
- 42x26 characters (6x8 font)
- Bitmap mode with fixed palette (0-255)
- No palette changes allowed.

#### **Screen 10 (MSX2+)**

- 256x212 12499 colors (Mixed mode: RGB + YJK)
- 42x26 characters (6x8 font)
- Color 0: Always PAPER (default: black)
- Color 1-14: For bitmap images
- Color 15: Always INK (default: white)
- Bitmap mode with fixed palette (0-12499)
- Default EGA palette.
- Palette changes (using GFX condact) modify only text colors, the bitmaps aren't affected.

#### **Screen 12 (MSX2+)**
- 256x212 19268 colors (fixed palette YJK. Y vary each pixel, J & K remains each 4 pixels providing something like ~RGB555)
- 42x26 characters (6x8 font)
- Bitmap mode with fixed palette (0-19268)
- INK/PAPER color changes don't have effect. INK is always white and PAPER is always black.

***

### Aditional tools in /bin folder

#### **imgwizard.php** (current version: 1.4.2)
A tool to convert SC5, SC6, SC7, SC8, SCA (SC10) and SCC (SC12) images to the **msx2daad** chunked image format.  
Supported codecs: **raw** (no compression), **rle**, **pletter _[DEPRECATED]_** and **zx0**.

Since v3.0.0 the canonical command is **`cx[l]`** (chunked V2 format with V9938 streaming via port `#9B`):

	$> php imgwizard.php cx LOC1.SC8 0 0 256 198 zx0

It also supports `--transparent-color=N` (2-pass `LMMC|AND` + `LMMC|OR` on SC5/6/7/8; YJK+YAE A flag on SC10) and `--fixed=X,Y` for fixed-position pictures (`FIXEDIMG` chunk).

The legacy `c`/`cl`/`s` commands still work but are marked **DEPRECATED** and emit a warning — convert existing pictures to `cx`/`cxl` for new work:

	$> php imgwizard.php c LOC1.SC8 96 rle    # legacy, deprecated

You can execute the tool without arguments to see examples of all the options and uses.

#### **precomp.php**
Read a *DDB* adventure file and generates the file */include/daad_defines.h* with a list of condacts not used in the input adventure file.  
If after that you do a clean compilation, you will obtain a thin **msx2daad** executable.

Execute this tool from the root folder of **msx2daad**:

	$> php bin/precomp.php dsk/DAAD.DDB include/daad_defines.h

You can also use this feature using the makefile target:

	$> make precomp

#### **dsktool/dsktool.exe**
A Linux/Win32 tool to create and manage disk images (FAT12) from command line.  
You can create disk images of 360Kb, 720Kb, 1440Kb and 2880Kb sizes.  
List, add, delete, and update files of your MSX-DOS disk images.

#### **testdaad.c**
A command line program for testing your adventures using a text file with the commands and the expected texts in return.  
You need **openMSX** emulator configured and in the path and compile **msx2daad** with test support using *-DTEST* parameter.  
The **testdaad** program can be compiled with GCC/MinGW.

	#
	# Example of testing input file
	# msx2daad.com must be compiled with -DTEST
	#
	# > is used to send commands
	# < is used to expect output texts
	#
	>go north
	<bad direction
	>go east
	<bad direction
	>take sword
	<taken

***

### External tools

#### **WebMSX Emulator**

Easy to use online MSX Emulator where you can test your adventure without instalations.  
https://webmsx.org/

#### **Online MSX Screen Converter (jannone)**
You can use this page to convert yours original pictures to MSX format.  
Palette files are not generated and must be done handmade, but *screen 8* and *screen 12* files can be generated easily.  
http://msx.jannone.org/conv/

#### **MSXViewer5 Screen Converter (MarMSX)**
A powerful tool to convert pictures to MSX formats. Versions for Linux and Windows.  
You can export image and palette files.  
http://marmsx.msxall.com/msxvw/msxvw5/index_en.php

---

## More stars!

Please give us a star on [GitHub](https://github.com/nataliapc/msx2daad) if you like this project.

### Star History

[![Star History Chart](https://api.star-history.com/svg?repos=nataliapc/msx2daad&type=Date&theme=dark)](https://www.star-history.com/#nataliapc/msx2daad&Date)

---
