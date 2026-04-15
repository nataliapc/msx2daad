[Jump to content](#content)

## [DAAD Ready](/)

#### DAAD Ready Documentation

En español [aquí![](res/spanish.gif)](doc_es.html)

This document uses parts from the DAAD v2 Manual (C) Infinite Imaginations 1988/89/91, which was written by Tim Gilberts, compiled back to modern format by Pedro Fernandez from Andres Samudio’s disks, and had some amendments appointed or made by a few people more, and very specially by Stefan Vogt. This manual has been expanded, modified, cut, and cropped by Uto.

PLEASE NOTICE THIS IS NOT THE DAAD MANUAL, BUT AN ADAPTED VERSION MADE FOR DAAD READY PACKAGE.

READ THIS MANUAL TO USE DAAD READY, YOU WON’T NEED TO READ DAAD ORIGINAL MANUAL, BUT ONCE YOU KNOW WELL HOW DAAD READY WORKS, MAYBE YOU WANT TO HAVE A LOOK AT IT.

#### TABLE OF CONTENTS

* [Overview](#overview)
  + [The Source](#thesource)
  + [The Compiler](#thecompiler)
  + [The Interpreters](#theinterpreters)
  + [Multimedia](#multimedia)
  + [Getting help](#help)
  + [Supported Languages](#supportedlanguages)
* [How to use DAAD Ready](#howtousedaadready)
  + [Other Details](#otherdetails)
* [DAAD Programming](#daadprogramming)
  + [Overview](#programmingoverview)
  + [The flags](#theflags)
  + [Layout windows](#windows)
  + [The processes workflow](#mainloop)
  + [First steps](#first_steps)
  + [CondActs](#condacts)
  + [Indirection](#indirection)
  + [Conditions](#conditions)
  + [Actions](#actions)
  + [Maluva Condacts](#maluvacondacts)
  + [Meta Condacts](#metacondacts)
  + [DAAD for The Quill or PAW developers](#daadforquillpawdevelopers)
  + [Errors](#errors)
  + [The Parser](#theparser)
  + [System Flags](#systemflags)
* [The Source File](#thesourcefile)
  + [Sections](#sections)
  + [Escape Chars](#escapechars)
  + [Preprocessor Commands](#preprocessorcommands)
* [Appendix](#appendix)
  + [A - The Character Set](#appendixa)
  + [B - DAAD Ready Customization](#appendixb)
  + [C - Supported languages](#appendixc)
  + [D - Symbols](#appendixd)
  + [E - Greetings](#appendixe)
  + [F - Licenses](#appendixf)

#### OVERVIEW

DAAD Ready makes easy for developers to have their game running in emulators, without having to care about building disks, transferring files, and may other options that required a lot of manual work both in the current emulator world, and in the original real machines one.

With DAAD Ready you will just have to concentrate in creating your game, cause building a .DSK, .D64 or whatever other format required for your target platform, is as easy as double clicking the .BAT file for your target (i.e. C64.BAT to generate the .D64 file for Commodore 64).

Due to all this, in this manual you may find references to DAAD itself, the original tool, and DAAD Ready, the package you have in your hands, which includes DRC (the new compiler, by Uto), original DAAD interpreters (by Infinite Imaginations), Maluva Extension (to use disk based graphics instead of vector ones, by Uto), the new MSX2 interpreter (by NatyPC), the new Plus/4 interpreter (by Imre Szell, based on the C64 one), the new MSDOS interpreter (by Uto) and several tools for testing and packaging (emulators and other tools). The use of all these is transparent to the writer/programmer, that, as said above, just must click a BAT file to get the game running in the included emulator for each target.

The basic principle behind DAAD is to provide a system where a game needs to be written only once and will then work on all machines. This of course is not truly possible without sacrificing facilities. So, a compromise has been arrived at where an amount of extra work may need to be done to allow each machine to be used to best advantage. The core of the game design remains independent reducing development and debugging time considerably.

The basic principle behind DAAD Ready is to make all that possible in one click, and use modern storage units, which leads to using raster graphics rather than vector ones.

DAAD Ready can create games for:

* **Amstrad CPC** (floppy disk, so either 664 or 6128). Also, M4 interface works. With graphics.
* **Commodore 64 or 128** (floppy disk, with graphics)
* **Atari ST**
* **Commodore Amiga**
* **MS-DOS** (VGA 256 colours)
* **MSX** 1 with at least 64K of RAM (floppy disk, with graphics)
* **MSX 2** computers (floppy disk, with graphics)
* **Amstrad PCW** 8000/9000 Series (floppy disk, no graphics)
* **Commodore Plus/4** (floppy disk, with graphics)
* **Sinclair ZX Spectrum 48k** (tape, no graphics)
* **Sinclair ZX Spectrum 48K with DivMMC/IDE** (SD Card, with graphics)
* **Sinclair ZX Spectrum +3** (floppy disk, with graphics).
* **Sinclair ZX Spectrum 128K** (tape, with graphics)
* **Sinclair ZX Spectrum Next** (SD Card, with graphics)
* **ZX-Uno** (SD card, with graphics)
* **Windows** (Setup file)- bundles DOS version with DOSBOX
* **HTML/Javascript** (to play in the browser)
* **Windows Experimental**: using ADP interpreter, still in beta version, but creating a native Windows game. Unlike old Windows target, does not yet create a Setup file. Also, can’t do SVGA graphics or 256 colours graphics like DOS+DOSBOX bundle does.
* **MacOS Experimental**: same than above, but for MacOS. Please notice MacOS can’t run this unsigned game unless you allow it in the settings.

Also, there are two OLD MSDOS and PLUS3 targets, which are deprecated, and remain for historical reasons.

Please notice DAAD Ready is no table to generate games with graphics for Amstrad PCW, so the only way to add them is using the original DOS software included with DAAD. Please check the original DAAD manual.

DAAD uses a programming language designed especially for writing Adventure Games. It is loosely based on the QUILL and PAW systems (by Gilsoft), and a familiarity with their operation is useful. There is a section further on this manual where you can find some help if you are/were a Quill or PAW author.

DAAD Ready can be divided into four main functional area:

###### The Source

The source (.DSF files, DAAD Source File) are a collection of tables, which contain all the information to define an adventure game. That includes location descriptions, objects description & weight, and the game logic.

The .DSF files are in the format of an ASCII Text file using ISO 8859-1 encoding. Please bear this in mind cause many modern editors tend to use UTF-8 by default, and that can have side effects if you are creating a non-English game with special non English characters (i.e. "ñ", "á", "è", etc.). For English, you do not really care about that.

###### The Compiler

This program is provided for the development machine only. The Compiler checks the source file for errors and converts it into a DAAD database for the specified machine (.DDB file) which the Interpreters can understand. DAAD Ready includes DRC as compiler, DRC is a new compiler made by Uto in late 10s decade. You don’t really have to care too much about the compiler, cause DAAD Ready takes care of using it for you, but you can find your game does not compile well if you make some mistakes when changing DSF files. For those cases, you will see compile errors that you would have to fix. In case you are not sure, reverse latest changes in DSF file, and try again.

###### The Interpreters

These are the real heart of DAAD. There is an interpreter for each computer supported on the DAAD system. It runs the game using its collection of routines to carry out the tasks needed by adventure games. It provides the machine independent aspect of DAAD. DAAD Ready also takes care of the interpreters, and creates a .DSK file, .D64 , etc. file for each target machine, which already includes the interpreter, so you don’t really have to care about choosing the right interpreter, DAAD Ready does it for you.

###### Multimedia

Unlike original DAAD, DAAD Ready is not using vector graphics for location images, it is using raster graphics in lieu. There is an [additional manual](multimedia_en.html) dedicated to multimedia.

###### Getting help

Should you need help understanding this or other DAAD topics, you may join the following groups in Telegram:

* [DAAD Ready official support group (English and Spanish)](https://t.me/daadready)
* [A general purpose text adventure group (English)](https://t.me/Advent8bit)
* [A general purpose text adventure group (Spanish)](https://t.me/RetroAventuras)

###### Supported Languages

Despite DAAD does only support English and Spanish, some limited support for Portuguese, French and German has been added to DAAD Ready. You can skip this section if you are creating Spanish or Eng-lish games.

Limited means that although you will be able to create games which use the special characters of those languages as "ô" "ß" or "õ", the interpreter below will always be the English one (for German and French) and the Spanish one (for Portuguese), so player won’t be able to provide orders with those characters, and parsing will be made thinking the language is English/Spanish. That also influences object listing:

* If an object description starts with "a" or "some" in English games, when added to a message like "You take the \_." that underscore will be replaced by the object text without the arti-cle. That way an object named "a lamp" will be display "You take the lamp." when taken, instead of "You take the a lamp."
* If an object description stars with "una", "un", "una", "unos" in a Spanish game, that definite article would be replaced by the indefinite one, so "Tomas \_." would work, so "una lámpara" will show "Tomas la lámpara." when taken.

Sadly, those are very language specific behaviors, so for German, French and Portuguese articles are neither removed nor modified, as those languages articles do not match the removal/replacement rules. Thus, a reasonable approach has been taken for each language: in French and Portuguese, the message will use the indefinite article, and in German, messages like those will not show the object text, and will just say "Taken.".

Other languages may be used with DAAD, in a similar way than Portu-guese, French or German are. Please join some of the Telegram groups above if you need help using DAAD Ready with your own language. Please notice these are the special characters supported by DAAD, other than English ones:

```
ª ¡ ¿ á é í ó ú ñ Ñ ç Ç ü Ü à ã â ä è ê ë ì î ï ò ô ö õ ù û Ý
Á É Í Ó Ú Â Ê Î Ô Û À È Ì Ò Ù Ä Ë Ï Ö Ü ý Ý þ Þ å Å ð Ð ø Ø ß
```

If your language uses special symbols other than these, still it can be done, provided it still uses roman alphabet in general.

#### How to use DAAD Ready

Here is a summary of the major points needed to produce a game:

* To begin with, click on any of the BAT files (ZXESXDOS.BAT for instance). You will be asked which language the game will be using, then an empty game will be compiled, and you would be asked to press enter to continue, do it, and then just close the emulator launched after you press any key.
* Well, once you have run any of the BAT files, you will find a TEST.DSF file in your DAAD Ready folder. That one is your source file. Start editing your source file with your preferred text editor. Make sure encoding is correct (ISO 8859-1) unless you are making an English game (if so, encoding does not matter).
* You can make changes in the source file by knowing how to develop in DAAD language/format, check further into this document.
* Each BAT file run compiles the game for the specified target (i.e. C64.BAT for Commodore 64) and tests the game. Please notice there will be a pause before launching the emulator, you could press CTRL+C in order to cancel emulation being launched if you just wanted to know if your changes were ok, but not testing.
* Also, notice in that pause, sometimes you get instructions about what to do when emulators are launched, cause sometimes you must do something after the emulator runs (game does not start automatically).
* Finally, you should know the game you are testing it is stored at the RELEASE folder, where you can find a folder per target. You could find the D64, DSK or whatever file in there once you want to release the game.

|  |  |
| --- | --- |
| AMIGA.BAT | Commodore Amiga |
| ATARIST.BAT | Atari ST |
| C64.BAT | Commodore 64 |
| CPC.BAT | Amstrad CPC |
| CPLUS4.BAT | Commodore Plus/4 |
| MSDOS.BAT | MSDOS |
| MSX1.BAT | MSX 1 |
| MSX2.BAT | MSX 2 |
| ZX48TAPE.BAT | ZX Spectrum 48k (no graphics but loading screen) |
| ZX128TAPE.BAT | ZX Spectrum 128k (tape) |
| ZX128PLUS3.BAT | ZX Spectrum Plus 3 (disk) |
| ZXESXDOS.BAT | For Spectrum 48K or above with DivMMC/DivIDE interface. |
| ZXUNO.BAT | For ZX-Uno |
| PCW.BAT | Amstrad PCW |
| HTML.BAT | HTML/Javascript |
| WINDOWS.BAT | Windows |
| WINDOWS\_EXPERIMENTAL.BAT | Windows Experimental |
| MACOS\_EXPERIMENTAL.BAT | MacOS Experimental |
| OLDMSDOS.BAT | MSDOS (old version, no graphics) |
| OLDPLUS3.BAT | ZX Spectrum Plus 3 (uses original interpreter) |

###### Other Details

* Once you have selected a language your DAAD Ready setup will consider the game you are making is in that language. In case you want to create a different game at the same time, just unzip the DAAD Ready zip file in some other folder, and work with that game there. I know, is like using a sledgehammer to crack a nut, but for the time being, that is the way to do it.
* Visual Studio Code is a recommended editor as it has an addon made by Chris Ainsley which provides syntax highlighting, check for DAAD in the addons option of the editor.

#### DAAD Programming

Please notice that apart of this detailed information, there is an online tutorial made by Uto. You can find it by clicking [this link](https://medium.com/%40uto_dev/a-daad-tutorial-for-beginners-1-b2568ec4df05).

###### Overview

When the game interpreter starts, it will run the information you entered in the TEST.DSF file. If you have had a look at those files, you may have found several sections to define objects, locations, etc. and at the end the processes. The processes are the DAAD programming code, which will determine how the game works. When a game starts, process 0 is run first ("/PRO 0" section).

Please notice TEST.DSF file already contain code that makes DAAD behave very much like PAW, so if you ever used PAW, it would be easy for you to adapt.

###### Layout windows

DAAD allows you to define windows within the screen. A window is a box within the screen, which starts at some coordinates and has
a width and a height. Inside the window you can write text, draw graphics, or whatever, and DAAD will take care of not letting the
text or graphics to go outside the window. When a window is active, by default everything you write or draw goes to that window.
There are 8 windows available in DAAD, numbered from 0 to 7. When the game starts, all windows are full screen (overlapping).

With DAAD Ready, window #0 is used for graphics, and it is placed at the top of the screen, in the first 12 rows of characters.
Text is placed in window #1, starting at line 14 (we leave one blank), and until the end of the screen. Both windows have maximum width.

Obviously, this can be changed, and there are specific actions to manage windows, but DAAD Ready uses this simple and very common layout in text adventures.

In 8 bit machines, graphics are directly supported by the new interpreters. See the [Multimedia Manual](multimedia_en.html) for more details on how graphics
are affected by windows or not (depends on target).

###### The Flags

The flags are like containers that can hold a value, you can change the value and check later, so you know something has happened. For instance you can set a flag to value 1 when an evil troll has run away due to some player action, and then you can check if that action has been performed when you want to cross a bridge where the evil troll was, and determine if he run away or not.

There are 256 flags available in DAAD, so you have plenty of them for your puzzles, despite some of them have an internal meaning. For instance, flag 38, also known as "fPlayer", holds the number of the location where the player is right now, so if you change it, you are moving the player somewhere else, but also you can use it to check if the player is at a given location or not, in order to determine if he can perform some action (i.e. sleep in the bedroom, but not in the bathroom). System flags, as they are named, are explained later in this section.

###### The processes workflow

When the game starts, the interpreter performs initialization just once, which implies all
flags are set to 0 and the screen is cleared. Notice that is why the player always starts
at location 0, because flag 38 (fPlayer) is set to 0.

**Start**

The interpreter calls process 0, when it exits that process it calls the operating system or restarts the computer.
Normally this will only happen if the player performs an end of game action and says he does not want to play again.

Figure 1 describes how the processes table is executed. Note that if process 0 ends, it does not jump to process 1,
to jump from one table to another there is a specific order "PROCESS" that jumps to another table. When that table ends,
it returns to the position from which it was called.

![Figure 1](res/Image1.png)

DAAD will search in each enrty (they start with the character ">") until the process ends.
Assuming there is some entry, it will check that the verb and name of these correspond with
the verb and name of the order given by the player, which will have been extracted by a PARSE order in the process.

Using word "\_" in the verb or name of an entry means that it will be executed regardless of the verb or name of the order given by the player.
Thus, an entry "> \_ \_" is run regardless of what the player wrote, while an entry "> OPEN \_" is run if the player asked to open something
(OPEN DOOR, OPEN BOX, etc.), and an entry "> \_ KEY" is run if the player asked to do anything with the key (LOOK KEY, LEAVE KEY, TAKE KEY, etc.)

Example of an entry:

```
> VERB NOUN
Condact1
Condact2
...
CondactN
```

Example o a double entry which is the same as adding two entries with the same condactos (generally used for sentences that do the same like PUSH BUTTON and CALL LIFT):

```
> VERB1 NOUN1
> VERB2 NOUN2
Condact1
Condact2
...
CondactN
```

Once we see an entry that matches the verb and name of the order given by the player, we will execute the condacts of that entry one by one, from top to bottom.

When a condact is a condition, we will check if it is fulfilled or not. If it is fulfilled, we will execute the next condact of the same entry, and so on until
we finish the condacts of that entry or a condition fails. If a condition fails, we will not execute the rest of the condacts of that entry, and we will
evaluate the verb and name of the next entry. If condition is fullfilled, we will execute the next condact of the same entry, and so on until we finish
that entry or another condition fails.

###### First steps

DAAD programming requires learning a series of orders called condacts, which we will see in the next section, but first we are going to try to see some practical examples, following the logic seen in the previous section.

First let's imagine the player can press a button, and when he does, a key appears on the floor. Let's imagine start by adding the ket to OBJ section:

`/1 252 1 _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ KEY _`

And to OTX section:

`/1 "a key"`

Also, vocabulary (VOC section):

```
KEY 100 noun
BUTTON 101 noun
PUSH 120 verb
```

Notes: the election of 100 and 101 as the number representing the key and button words is arbitrary, you can choose any that is not being used by another noun, because if you repeat the same, both names would be considered synonyms, which we do not want. Likewise, the value 252 as the starting location in the OBJ section, indicates that the object is not created, it does not exist when the game starts.

Well, as we want the player to be able to press the button, we must add an entry in the PRO 5 section. There goes what we call "response table". We put the following at the end of that section (just before /PRO 6):

```
> PUSH BUTTON
AT 1
ISAT 1 252
MESSAGE "You press the button and a key appear on the floor."
CREATE 1
DONE
```

With this, we are telling DAAD that if the player writes "PUSH BUTTON", we are in location 1 and object 1 (the key) is in location 252 (not created), write on screen the message "You press the button and a key appear on the floor", create object 1 (the key) in location 1, and end process 5 (DONE).

If we run the BAT file now, we will see that when we write "PUSH BUTTON" we get the message, but the key does not appear because the condition ISANT 1 252 is not fulfilled, because the key is already created in location 1. We will see the "You can't do that" message in place.

To avoid that, let's add another entry to process 5:

```
> PUSH BUTTON
AT 1
ISNOTAT 1 252
MESSAGE "You press the button but nothing happens."
DONE
```

This second entry makes that if we are in location 1, but the key has already been created (it is not in 252), we get the other message and nothing else happens.

Adding details

Location descriptions usually have a fixed description, but sometimes we would like to change something during the game. Let's go to the LTX section and change the line of location 1 to:

`/1 "You have entered the dungeon, you see a button."`

We can now see the button when entering location1, and so have a reason to press it. But let's go further. What happens when we press the button?

We will be adding the following at the beginning of the PRO 3 section (just after /PRO 3). Process 3 is run just after printing the location description, so it's ideal to add details to the location text:

```
> _ _
AT 1
ISNOTAT 1 252
MES "The button is pressed."
```

If you look at the entry, you will see that it does not have a verb or a name, so it will be executed regardless of what the player writes. It will only be executed if we are in location 1 and the
key has already been created (it is not in 252). It will show the message "The button is pressed" and nothing else.

Events

Sometimes thingds happen in the game that are not related to the player's actions. For instance a lightning falls, a river overflows, a fairy appears, a troll attacks us, etc. These are called events, and they are executed in process 4, which is run every turn.

For instance, let's say in location 2 there is a troll, which will attack us after 3 turns, threatening us before. We can do the following in PRO 4:

```
>_ _
AT 2
EQ 254 2
MES "The troll attacks you with his mace. You're dead."
END
>_ _
AT 2
EQ 254 1
LET 254 2
MES "The troll threatens you with his mace."
> _ _
AT 2
ZERO 254
LET 254 1
MES "The troll looks at you threateningly."
```

We are using a flag, #254, to count turns. By default all flags are 0, so flag 254 will be 0 at the beginning. That is why when entering process 4 (each turn is run) the entry that will be executed is the third one,
because the condition ZERO 254 is fulfilled (flag 254 is 0). In that entry, we change flag 254 to 1, and show the message "A troll looks at you threateningly".

One turn later, flag 254 will be 1, so the second entry will be executed, because the condition EQ 254 1 is fulfilled (flag 254 is 1). In that entry, we change flag 254 to 2, and show the message "The troll threatens you with his mace".

Finally, one more turn later, flag 254 will be 2, so the first entry will be executed, because the condition EQ 254 2 is fulfilled (flag 254 is 2). In that entry, we show the message "The troll attacks you with his mace. You're dead", and end the game (END).

###### CondActs

There now follows a detailed description of each CondAct that may be included in an entry. They are divided into groups according to the subject they deal with in DAAD; such as flags, objects, etc. and give some hints as to a possible use.

For those used to the PAW systems. Be careful! Several actions have been removed, while others have changed in function. Make sure, you check on their function within DAAD. E.g. TURN & SCORE are deleted - they must now be soft coded. Also, TIMEOUT, PROMPT and GRAPHIC are gone as 'HASAT fTimeout', 'LET fPrompt x' and 'LET fGFlags expression' will do the same.

Several abbreviations are used in the descriptions as follows:

* locno. is a valid location number.
* locno+ is a valid location number or 252 or "\_" (meaning not-created), 253 or "WORN", 254 or "CARRIED" and 255 or "HERE" (which is converted into the current location of the player).
* mesno. is a valid message.
* sysno. is a valid system message.
* flagno. is any flag (0 to 255).
* procno. is a valid sub-process number.
* word is word of the required type, which is present in the vocabulary, or "\_" which ensures no-word - not an any match as normal.
* value is a value from 0 to 255.

###### Indirection

The first parameter on most condActs can use indirection. This is indicated by placing at sign (@) before the first parameter of a CondAct. This will cause not the number itself to be used, but the contents of the flag corresponding to that number. Only the first parameter may be indirected but this provides a powerful facility. Although not all commands can indirect, most that have a flag, object, or number as the first parameter can be indirected.

E.g. 'MESSAGE @100' will print the message number that is given by the value in Flag 100 - as opposed to 'MESSAGE 100' which would print message 100!

###### Conditions

Conditions which deal with the location of the player

AT locno

Succeeds if the current location is the same as locno.

NOTAT locno

Succeeds if the current location is different to locno.

ATGT locno

Succeeds if the current location is greater than locno.

ATLT locno

Succeeds if the current location is less than locno.

Conditions which deal with the current location of an object

PRESENT objno.

Succeeds if Object objno. is carried, worn or at the current location.

ABSENT objno.

Succeeds if Object objno. is not carried, not worn and not at the current location.

WORN objno.Succeeds if object objno. is worn.

NOTWORN objno.

Succeeds if Object objno. is not worn.

CARRIED objno.

Succeeds if Object objno. is carried.

NOTCARR objno.

Succeeds if Object objno. is not carried.

ISAT objno. locno+

Succeeds if Object objno. is at Location locno.

ISNOTAT objno. locno+

Succeeds if Object objno. is not at Location locno.

Conditions which deal with the value and comparison of flags

ZERO flagno.

Succeeds if flag flagno. is set to zero.

NOTZERO flagno.

Succeeds if flag flagno. is not set to zero.

EQ flagno. value

Succeeds if flag flagno. is equal to value.

NOTEQ flagno. value

Succeeds if flag flagno. is not equal to value.

GT flagno. value

Succeeds if flag flagno. is greater than value.

LT flagno. value

Succeeds if flag flagno. is less than value.

SAME flagno1. flagno2.

Succeeds if flag flagno1 has the same value as flag flagno2.

NOTSAME flagno1. flagno2.

Succeeds if flag flagno1 does not have the same value as flag flagno2.

BIGGER flagno1. flagno2.

Succeeds if flag flagno1 is larger than flag flagno2.

SMALLER flagno1. flagno2.

Succeeds if flag flagno1 is smaller than flag flagno2. It is not actually needed as reversing the parameters of BIGGER would do the same, but it may make programs more readable and indirection can be used with 'either' flag by using the appropriate condition.

Conditions to check for an extended logical sentence

It is best to use these conditions only if the specific word (or absence of word using "\_") is really needed. Doing that allows greater flexibility in the commands
understood by the game. That is, although you can force player to write "UNLOCK DOOR WITH KEY" don’t do it unless the game script really requires the player to specify
he wants to unlock the door with the key and not with something else, because doing it with the key is not the most logical thing to do. Just allow "UNLOCK DOOR"
to work while key is carried.

ADJECT1 word

Succeeds if the first noun's adjective in the current LS is word.

ADVERB word

Succeeds if the adverb in the current LS is word.

PREP word

Succeeds if the preposition in the current LS is word.

NOUN2 word

Succeeds if the second noun in the current LS is word.

ADJECT2 word

Succeeds if the second noun's adjective in the current LS is word.

Conditions for random occurrences

You could use it to provide a chance of a tree falling on the player during a lightning strike or a bridge collapsing etc. Do not abuse this facility, always allow a player a way of preventing the problem, such as rubber boots for the lightning, or similar.

CHANCE percent

Succeeds if percent is less than or equal to a random number in the range 1-100 (inclusive). Thus, a CHANCE 50 condition would allow DAAD to look at the next CondAct only if the random number generated was between 1 and 50, a 50% chance of success.

Conditions providing a boolean TRUE/FALSE for subprocess calls

ISDONE

Succeeds if the last table ended by exiting after executing at least one Action. This is useful to test for a single succeed/fail boolean value from a Sub-Process. A DONE action will cause the 'done' condition, as will any condact causing exit, or falling off the end of the table - assuming at least one CondAct (other than NOTDONE) was done.

See also ISNDONE and [NOTDONE](#notdone) actions.

ISNDONE

Succeeds if the last table ended without doing anything or with a NOTDONE action.

Conditions for object attributes

HASAT/HASNAT value

This condacts checks the object attribute specified by value. 0-15 are the object attributes for the current referenced object. This is the use most of you will give to the condact, but HASAT/HASNAT are actually more powerful, but to properly understand it you will have to know about handling flags bitwise. If you think you understand that, please continue reading, if not, just continue to the next condact:

The truth is HASAT and HASNAT do not check object attributes, they actually check bits in some flags. It happens that HASAT 0-7 checks bits in flag 59, and 8-15 checks bits in flag 58. As flags 58 and 59 containt a copy of the attributes of the currently referenced, HASAT seems to check the attributes, but it’s actually checking the flags.

And here comes what it’s really interesting : as HASAT parameter can have a value from 0 to 255, as you can imagine, next 8 values for HASAT/HASNAT 16-23 check bits of flag 57, and next 8 values (24-31), bits of flag 56, and so on until values 248-255 check bits of flag 28, which happens not to be a system flag, but a general purpose one.

So, for instance, as bit 7 of flag 57 determines if the currently referenced obejct is wearable, HASAT 23 will succeed of the currently referenced object is wearable.

When you see the System Flags table further on this Will make more sense, but see some simples of whan can be done:

| Symbol | Flag | Attr. Number | Description |
| --- | --- | --- | --- |
| WEARABLE | 57-Bit7 | 23 | Current object is wearable |
| CONTAINER | 56-Bit7 | 31 | Current object is a container |
| LISTED | 53-Bit7 | 55 | If objects listed by LISTOBJ |
| TIMEOUT | 49-Bit7 | 87 | If Timeout last frame |
| GMODE | 29-Bit7 | 247 | Graphics available |
| MOUSE | 29-bit0 | 240 | Mouse available |
| CUSTOM | 28-Bit7 | 255 | Bit 7 of flag 28, user flag |

As a further example, The TIMEOUT condition of PAW is implemented in DAAD by

`HASAT TIMEOUT`

That also allows a simulation of "NOTTIMEOUT", that can be done with:

`HASNAT TIMEOUT`

HASAT/HASNAT can check bits from flags 28 to 59, a total of 32 flags.

Conditions to interact with the player

INKEY

Is a condition which will be satisfied if the player is pressing
a key. In 16Bit machines Flags Key1 and Key2 (60 & 61) will be a
standard IBM ASCII code pair. On 8-bit only Key1 will be valid,
and the code will be machine specific.

QUIT

SM12 ("Are you sure?") is printed and the input routine
called. Will succeed if the player replies with a word which
starts with the first letter of SM30 ("Y") to the prompt. If not
then the remainder of the entry is discarded and the next entry
is carried out.

###### Actions

Actions to deal with the manipulation of object positions

GET objno.

If Object objno. is worn or carried, SM25 ("I already have the \_.") is printed and actions NEWTEXT & DONE are performed.

If Object objno. is not at the current location, SM26 ("There isn't one of those here.") is printed and actions NEWTEXT & DONE
are performed.

If the total weight of the objects carried and worn by the player plus Object objno. would exceed the maximum conveyable weight (Flag 52) then SM43 ("The \_ weighs too much for me.") is printed and actions NEWTEXT & DONE are performed.

If the maximum number of objects is being carried (Flag 1 is greater than, or the same as, Flag 37), SM27 ("I can't carry any
more things.") is printed and actions NEWTEXT & DONE are performed. In addition, any current DOALL loop is cancelled.

Otherwise the position of Object objno. is changed to carried, Flag 1 is incremented and SM36 ("I now have the \_.") is printed.

DROP objno.

If Object objno. is worn then SM24 ("I can't. I'm wearing the
\_.") is printed and actions NEWTEXT & DONE are performed.

If Object objno. is at the current location (but neither worn nor
carried), SM49 ("I don't have the \_.") is printed and actions
NEWTEXT & DONE are performed.

If Object objno. is not at the current location then SM28 ("I
don't have one of those.") is printed and actions NEWTEXT & DONE
are performed.

Otherwise the position of Object objno. is changed to the current
location, Flag 1 is decremented and SM39 ("I've dropped the \_.")
is printed.

WEAR objno.

If Object objno. is at the current location (but not carried or
worn) SM49 ("I don't have the \_.") is printed and actions NEWTEXT
& DONE are performed.

If Object objno. is worn, SM29 ("I'm already wearing the \_.") is
printed and actions NEWTEXT & DONE are performed.

If Object objno. is not carried, SM28 ("I don't have one of
those.") is printed and actions NEWTEXT & DONE are performed.

If Object objno. is not wearable (as specified in the object
definition section) then SM40 ("I can't wear the \_.") is printed
and actions NEWTEXT & DONE are performed.

Otherwise the position of Object objno. is changed to worn, Flag
1 is decremented and SM37 ("I'm now wearing the \_.") is printed.

REMOVE objno.

If Object objno. is carried or at the current location (but not
worn) then SM50 ("I'm not wearing the \_.") is printed and actions
NEWTEXT & DONE are performed.

If Object objno. is not at the current location, SM23 ("I'm not
wearing one of those.") is printed and actions NEWTEXT & DONE are
performed.

If Object objno. is not wearable (and thus removable) then SM41
("I can't remove the \_.") is printed and actions NEWTEXT & DONE
are performed.

If the maximum number of objects is being carried (Flag 1 is
greater than, or the same as, Flag 37), SM42 ("I can't remove the
\_. My hands are full.") is printed and actions NEWTEXT & DONE
are performed.

Otherwise the position of Object objno. is changed to carried.
Flag 1 is incremented and SM38 ("I've removed the \_.") printed.

CREATE objno.

The position of Object objno. is changed to the current location
and Flag 1 is decremented if the object was carried.

DESTROY objno.

The position of Object objno. is changed to not created (252) and Flag
1 is decremented if the object was carried.

SWAP objno1. objno2.

The positions of the two objects are exchanged. Flag 1 is not
adjusted. The currently referenced object is set to be Object
objno 2.

PLACE objno. locno+

The position of Object objno. is changed to Location locno. Flag
1 is decremented if the object was carried. It is incremented if
the object is placed at location 254 (carried).

PUTO locno+

The position of the currently referenced object (i.e. that object
whose number is given in flag 51), is changed to be Location
locno. Flag 54 remains its old location. Flag 1 is decremented if
the object was carried. It is incremented if the object is placed
at location 254 (carried).

PUTIN objno. locno+

If Object objno. is worn then SM24 ("I can't. I'm wearing the
\_.") is printed and actions NEWTEXT & DONE are performed.

If Object objno. is at the current location (but neither worn nor
carried), SM49 ("I don't have the \_.") is printed and actions
NEWTEXT & DONE are performed.

If Object objno. is not at the current location, but not carried,
then SM28 ("I don't have one of those.") is printed and actions
NEWTEXT & DONE are performed.

Otherwise the position of Object objno. is changed to Location
locno. Flag 1 is decremented and SM44 ("The \_ is in the"), a
description of Object locno. and SM51 (".") is printed.

TAKEOUT objno. locno+

If Object objno. is worn or carried, SM25 ("I already have the
\_.") is printed and actions NEWTEXT & DONE are performed.

If Object objno. is at the current location, SM45 ("The \_ isn't
in the"), a description of Object locno. and SM51 (".") is
printed and actions NEWTEXT & DONE are performed.

If Object objno. is not at the current location and not at
Location locno. then SM52 ("There isn't one of those in the"), a
description of Object locno. and SM51 (".") is printed and
actions NEWTEXT & DONE are performed.

If Object locno. is not carried or worn, and the total weight of
the objects carried and worn by the player plus Object objno.
would exceed the maximum conveyable weight (Flag 52) then SM43
("The \_ weighs too much for me.") is printed and actions NEWTEXT
& DONE are performed.

If the maximum number of objects is being carried (Flag 1 is
greater than, or the same as, Flag 37), SM27 ("I can't carry any
more things.") is printed and actions NEWTEXT & DONE are
performed. In addition, any current DOALL loop is cancelled.

Otherwise the position of Object objno. is changed to carried,
Flag 1 is incremented and SM36 ("I now have the \_.") is printed.

Note: No check is made, by either PUTIN or TAKEOUT, that Object
locno. is present. This must be carried out by yourself if
required.

DROPALL

All objects which are carried or worn are created at the current
location (i.e. all objects are dropped) and Flag 1 is set to 0.
This is included for compatibility with older writing systems.
Note that a DOALL 254 will carry out a true DROP ALL, taking care
of any special actions included.

---

The next six actions are automatic versions of GET, DROP, WEAR,
REMOVE, PUTIN and TAKEOUT. They are automatic in that instead of
needing to specify the object number, they each convert
Noun (Adjective)1 into the currently referenced object - by
searching the object definition section. The search is for an
object which is at one of several locations in descending order
of priority - see individual descriptions. This search against
priority allows DAAD to 'know' which object is implied if more
than one object with the same Noun description (when the player
has not specified an adjective) exists, at the current location,
carried or worn - and in the container in the case of TAKEOUT.

AUTOG

A search for the object number represented by Noun (Adjective)1 is
made in the object definition section in order of location
priority; here, carried, worn. i.e. The player is more likely to
be trying to GET an object that is at the current location than
one that is carried or worn. If an object is found its number is
passed to the GET action. Otherwise if there is an object in
existence anywhere in the game or if Noun1 was not in the
vocabulary then SM26 ("There isn't one of those here.") is
printed. Else SM8 ("I can't do that.") is printed (i.e. It is not
a valid object but does exist in the game). Either way actions
NEWTEXT & DONE are performed

AUTOD

A search for the object number represented by Noun (Adjective)1 is
made in the object definition section in order of location
priority; carried, worn, here. i.e. The player is more likely to
be trying to DROP a carried object than one that is worn or here.
If an object is found its number is passed to the DROP action.
Otherwise if there is an object in existence anywhere in the game
or if Noun1 was not in the vocabulary then SM28 ("I don't have
one of those.") is printed. Else SM8 ("I can't do that.") is
printed (i.e. It is not a valid object but does exist in the
game). Either way actions NEWTEXT & DONE are performed.

AUTOW

A search for the object number represented by Noun (Adjective)1 is
made in the object definition section in order of location
priority; carried, worn, here. i.e. The player is more likely to
be trying to WEAR a carried object than one that is worn or here.
If an object is found its number is passed to the WEAR action.
Otherwise if there is an object in existence anywhere in the game
or if Noun1 was not in the vocabulary then SM28 ("I don't have
one of those.") is printed. Else SM8 ("I can't do that.") is
printed (i.e. It is not a valid object but does exist in the
game). Either way actions NEWTEXT & DONE are performed.

AUTOR

A search for the object number represented by Noun (Adjective)1 is
made in the object definition section in order of location
priority; worn, carried, here. i.e. The player is more likely to
be trying to REMOVE a worn object than one that is carried or
here. If an object is found its number is passed to the REMOVE
action. Otherwise if there is an object in existence anywhere in
the game or if Noun1 was not in the vocabulary then SM23 ("I'm
not wearing one of those.") is printed. Else SM8 ("I can't do
that.") is printed (i.e. It is not a valid object but does exist
in the game). Either way actions NEWTEXT & DONE are performed.

AUTOP locno.

A search for the object number represented by Noun (Adjective)1 is
made in the object definition section in order of location
priority; carried, worn, here. i.e. The player is more likely to
be trying to PUT a carried object inside another than one that is
worn or here. If an object is found its number is passed to the
PUTIN action. Otherwise if there is an object in existence
anywhere in the game or if Noun1 was not in the vocabulary then
SM28 ("I don't have one of those.") is printed. Else SM8 ("I
can't do that.") is printed (i.e. It is not a valid object but
does exist in the game). Either way actions NEWTEXT & DONE are
performed.

AUTOT locno.

A search for the object number represented by Noun (Adjective)1 is
made in the object definition section in order of location
priority: in container, carried, worn, here. i.e. The player is
more likely to be trying to get an object out of a container
which is in there than one that is carried, worn or
here. If an object is found its number is passed to the TAKEOUT
action. Otherwise if there is an object in existence anywhere in
the game or if Noun1 was not in the vocabulary then SM52 ("There
isn't one of those in the"), a description of Object locno. and
SM51 (".") is printed. Else SM8 ("I can't do that.") is printed
(i.e. It is not a valid object but does exist in the game).
Either way actions NEWTEXT & DONE are performed.

Note: No check is made, by either AUTOP or AUTOT, that Object
locno. is present. This must be carried out by you - if
required.

COPYOO objno1. objno2.

The position of Object objno 2 is set to be the same as the
position of Object Objno 1. The currently referenced object is set
to be Object objno 2.

RESET

This Action bears no resemblance to the one with the same name in
PAW. It has the pure function of placing all objects at the
position given in the Object start table. It also sets the
relevant flags dealing with no of objects carried etc.

Some actions to interchange flag and objects data

COPYOF objno. flagno.

The position of Object objno. is copied into Flag flagno. This
could be used to examine the location of an object in a
comparison with another flag value. e.g.

```
COPYOF 1 110
SAME 110 fPlayer
```

could be used to check that object 1 was at the same location as
the player - although ISAT 1 255 would be better!

COPYFO flagno. objno.

The position of Object objno. is set to be the contents of Flag
flagno. An attempt to copy from a flag containing 255 will result
in a runtime error. Setting an object to an invalid location
will still be accepted as it presents no danger to the operation
of DAAD.

WHATO

A search for the object number represented by Noun (Adjective)1 is
made in the object definition section in order of location
priority; carried, worn, here. This is because it is assumed any
use of WHATO will be related to carried objects rather than any
that are worn or here. If an object is found its number is placed
in flag 51, along with the standard current object parameters in
flags 54-57. This allows you to create other auto actions (the
tutorial gives an example of this for dropping objects in the
tree).

SETCO objno.

Sets the currently referenced object to objno.

WEIGH objno. flagno.

The true weight of Object objno. is calculated (i.e. if it is a
container, any objects inside have their weight added - do not
forget that nested containers stop adding their contents after
ten levels) and the value is placed in Flag flagno. This will
have a maximum value of 255 which will not be exceeded. If Object
objno. is a container of zero weight, Flag flagno. will be
cleared as objects in zero weight containers, also weigh zero!

Actions to manipulate the flags

SET flagno.

Flag flagno. is set to 255.

CLEAR flagno.

Flag flagno. is cleared to 0.

LET flagno. value

Flag flagno. is set to value.

PLUS flagno. value

Flag flagno. is increased by value. If the result exceeds 255
the flag is set to 255.

MINUS flagno. value

Flag flagno. is decreased by value. If the result is negative
the flag is set to 0.

ADD flagno1. flagno2.

Flag flagno 2 has the contents of Flag flagno 1 added to it. If the result exceeds 255 the flag is set to 255.

SUB flagno1. flagno2.

Flag flagno 2 has the contents of Flag flagno 1 subtracted from it. If the result is negative the flag is set to 0.

COPYFF flagno1. flagno2.

The contents of Flag flagno 1 is copied to Flag flagno 2.

COPYBF flagno1. flagno2.

Same as COPYFF but the source and destination are reversed, so
that indirection can be used. This will hopefully be replaced by
a comprehensive system of dual parameter redirection for COPY
actions in the future.

RANDOM flagno.

Flag flagno. is set to a number from the Pseudo-random sequence
from 1 to 100. This could be useful to allow random decisions to
be made in a more flexible way than with the CHANCE condition.

MOVE flagno.

This is a very powerful action designed to manipulate PSI's. It
allows the current LS Verb to be used to scan the connections
section for the location given in Flag flagno. If the Verb is
found then Flag flagno. is changed to be the location number
associated with it, and the next condact is considered. If the
verb is not found, or the original location number was invalid,
then DAAD considers the next entry in the table - if present.

This feature could be used to provide characters with random
movement in valid directions; by setting the LS Verb to a random
movement word and allowing MOVE to decide if the character can go
that way. Note that any special movements which are dealt with in
Response for the player, must be dealt with separately for a PSI
as well.

Actions to manipulate the flags dealing with the player

GOTO locno.

Changes the current location to locno. This effectively sets flag
38 to the value locno.

WEIGHT flagno.

Calculates the true weight of all objects carried and worn by the
player (i.e. any containers will have the weight of their
content added up to a maximum of 255), this value is then placed
in Flag flagno. This would be useful to ensure the player was
not carrying too much weight to cross a bridge without it
collapsing etc.

ABILITY value1 value2

This sets Flag 37, the maximum number of objects conveyable, to
value 1 and Flag 52, the maximum weight of objects the player
may carry and wear at any one time (or their strength), to be value 2.

No checks are made to ensure that the player is not already
carrying more than the maximum. GET and so on, which check the
values, will still work correctly and prevent the player carrying
any more objects, even if you set the value lower than that which
is already carried!

Actions to manipulate the flags for screen mode, format, etc.

MODE option

Allows the current window to have its operation flags changed.
To calculate the number to use for the option just add
the numbers shown next to each item to achieve the required
combination.

* 1 - Use the alternative character set. (A permanent #g, see escape sequences below)
* 2 - SM32 ("More...") will not appear when the window fills.

e.g. MODE 3 stops the 'More...' prompt and causes all characters
to be translated to the 128-256 range.

INPUT stream option

The 'stream' parameter will set the bulk of input to come from
the given window/stream. A value of 0 for 'stream' will not use
the graphics stream as might be expected, but instead causes
input to come from the current stream when the input occurs.

Options:

* 1 - Clear window after input.
* 2 - Reprint input line in current stream when complete.
* 4 - Reprint current text of input after a timeout.

TIME duration option

Allows input to be set to 'timeout' after a specific duration in
1 second intervals, i.e. the Process 2 table will be called again
if the player types nothing for the specified period. This
action alters flags 48 & 49. 'option' allows this to also occur
on ANYKEY and the "More..." prompt. In order to calculate the
number to use for the option just add the numbers shown next to
each item to achieve the required combination.

* 1 - While waiting for first character of Input only.
* 2 - While waiting for the key on the "More..." prompt.
* 4 - While waiting for the key on the ANYKEY action.

e.g. TIME 5 6 (option = 2+4) will allow 5 seconds of inactivity
on behalf of the player on input, ANYKEY or "More..." and between
each key press. Whereas TIME 5 3 (option = 1+2) allows it only on
the first character of input and on "More...".

TIME 0 0 will stop timeouts (default).

Actions to deal with screen output and control

WINDOW window

Selects window (0-7) as current print output stream.

WINAT line col

Sets current window to start at given line and column. Clipping
height and width to fit available screen.

WINSIZE height width

Sets current window size to given height and width. Clipping as
needed to fit available screen.

CENTRE

Will ensure the current window is centered for the current column
width of the screen. (Does not affect line position).

CLS

Clears the current window.

SAVEAT

Saves print position for current window. This allows
you to maintain the print position for example while printing
elsewhere in the window. You should consider using a separate
window for most tasks. This may find use in the creation of a
new input line or in animation sequences...

BACKAT

Restores print position.

PAPER colour
Sets current window paper colour according to the lookup table given in
the graphics editors, or as a palette number for machines with a
palette.
INK colour
Sets current window ink colour according to the lookup table given in
the graphics editors, or as a palette number for machines with a
palette.
BORDER colour

Sets main screen border colour - this is machine specific.

PRINTAT line col

Sets current print position to given point if in current window.
If not, then print position becomes top left of window.

TAB col

Sets current print position to given column on current line.

MES mesno.

Prints Message mesno.

MES "string"

Prints message between quotes.

Please notice you can either user double or single quotes for the string but use the same for starting and ending.

```
MES "Hello World!"
MES 'I'll be there at 3 o'clock.'
MES "John says "Hello"."
```

MESSAGE mesno.

Prints Message mesno. After printing either message, it prints a new line.

MESSAGE mesno. or MESSAGE "string"

Prints message between quotes. After printing either message, it prints a new line.

Please notice you can either user double or single quotes for the string but use the same for starting and ending.

SYSMESS sysno.

Prints System Message sysno.

SYSMESS "string"

Prints message between quotes.

DESC locno.

Prints the text for location locno. without a NEWLINE.

DESC "string"

Prints the text between quotes.

SPACE

Will simply print a space to the current output stream. Shorter
than MES Space!

NEWLINE

Prints a carriage return/line feed.

So what's the difference between DESC/MES/SYSMESS/MESSAGE when used with a string? Basically, the compiler adds internally that new message to the corresponding table,
so MES/MESSAGE will add that message to MTX table (user messages), SYSMESS will add it to STX table (system messages) and DESC to LTX table (Locations). In the end it
doesn't really matter, but take in mind each table has a limit of 256 messages, so you can use SYSMESS or DESC if you run out of space in MTX table.

The truth is the new DRC compiler already does that for you, so you can always use MES/MESSAGE and forget about it, the compiler will automatically put the message in the table where there is still some space.

Actions to deal with the printing of flag values on the screen

PRINT flagno.

The value of Flag flagno. is displayed without
leading or trailing spaces. This is a very useful action. Say
flag 100 contained the number of coins carried by the player,
then an entry in a process table of:

```
MES "You have "
PRINT 100
MESSAGE " gold coins."
```

could be used to display this to the player.

DPRINT flagno

Will print the contents of flagno and flagno+1 as a two byte
number.

i.e. a number in the range 0-65535 generated as: (flagno+1) \* 256 + (flagno)

DPRINT 255 is meaningless and will produce a random result.

Actions dealing with listing objects on the screen.

They are controlled/set by the value of flag 53 as described in the chapter on objects.

LISTOBJ

If any objects are present, then SM1 ("I can also see:") is
printed, followed by a list of all objects present at the current
location. If there are no objects then nothing (as in null, not
the word!) is printed.

LISTAT locno+.

If any objects are present, then they are listed. Otherwise SM53
("nothing.") is printed - note that you will usually have to
precede this action with a message along the lines of "In the bag
is:" etc.

>Actions to load or save the current state of the game

SAVE opt
SAVE opt
This action saves the current game position on disc or tape. SM60
("Type in name of file.") is printed and the input routine is
called to get the filename from the player. If the supplied
filename is not acceptable SM59 ("File name error.") is printed -
this is not checked on 8-bit machines, the file name is MADE
acceptable!

Option parameter ask DAAD to do one of the following, but some targets may ignore it and
have just tape or just disc support.

* 0 - Ask user tape or disc
* 1 - Save to TAPE (i.e. does not ask "Disc or Tape?")
* 2 - Save to DISC (ditto)

Please notice the option is just a suggestion, some targets may ignore it and have just tape or just disc support.

LOAD opt

This action loads a game position from disc or tape. A filename
is obtained in the same way as for SAVE. A variety of errors may
appear on each machine if the file is not found or suffers a load
error. Usually 'I/O Error'. The next action is carried out only
if the load is successful. Otherwise a system clear, GOTO 0,
RESTART is carried out.

* 0 - Normal action
* 1 - Save to TAPE (i.e. does not ask "Disc or Tape?")
* 2 - Save to DISC (ditto)

Option parameter ask DAAD to do one of the following, but some targets may ignore it and
have just tape or just disc support.

RAMSAVE

In a similar way to SAVE this action saves all the information
relevant to the game in progress not onto disc but into a memory
buffer. This buffer is of course volatile and will be destroyed
when the machine is turned off which should be made clear to the
player. The next action is always carried out.

RAMLOAD flagno.

This action is the counterpart of RAMSAVE and allows the saved
buffer to be restored. The parameter specifies the last flag to
be reloaded which can be used to preserve values over a restore,
for example, an entry of:

```
> RAMLO _
COPYFF 30 255
RAMLOAD 254
COPYFF 255 30
RESTART
```

could be used to maintain the current score, so that the player
can not use RAMSAVE/LOAD as an easy option for achieving 100%!

Note 1: The RAM actions could be used to implement an OOPS
command that is common on other systems to take back the previous
move; by creating an entry in the main loop which does an
automatic RAMSAVE every time the player enters a move.

Note 2: These four actions allow the next CondAct to be carried
out. They should normally always be followed by a RESTART or
describe in order that the game state is restored to an identical
position.

Actions to allow the game to be paused

ANYKEY

SM16 ("Press any key to continue") is printed and the keyboard is
scanned until a key is pressed or until the timeout duration has
elapsed if enabled.

PAUSE value

Pauses for value/50 secs. However, if value is zero then the
pause is for 256/50 secs.

Actions to deal with control of the parser

PARSE n
The parameter 'n' controls which level of string indentation is
to be searched. Now only two are supported by the interpreters
so only the values 0 and 1 are valid.

* 0 - Parse the main input line for the next LS.
* 1 - Parse any string (phrase enclosed in quotes [""]) that was contained in the last LS extracted.

Mode 0 is the primary method for converting the current input line
of the player to a logical sentence (LS). The detailed
description of the parser provides further details. The command
will extract the next LS from the current input line.

Mode 1 was designed to deal with speech to PSIs. Any string (i.e.
a further phrase enclosed in quotes [""]) that was present in the
players current phrase is converted into a LS - overwriting the
existing LS formed originally for that phrase.

If no phrase is present at level 0 then the input line is called
preceded with a random prompt - this gets a new input line from
the player automatically when required, removing the problem of
handling multiple phrases by the programmer. At level 1 and above
the next CondAct is carried out. This occurs at all levels if the
LS is invalid. Note that DAAD will look at the next condact in
what can be considered a fail situation - this is different to
what you might expect. Otherwise the next entry is considered
with the new LS of the speech made to the PSI. Because it
overwrites the current LS it must be used carefully - this is
also, the reason for doing the next CondAct in a fail situation,
think about it!

If you are using a text-based command structure you will need at
least one PARSE 0 CondAct in the main loop somewhere.

e.g. the minimum process 0 without an initialization might be: -

```
> _ _
PARSE 0
MESSAGE "I don't understand"
REDO

> _ _
PROCESS x ; Deal with any commands
REDO
```

To use it to speak to a PSI there will be two or more calling
entries (in process x of the above example) which will be similar
to:

```
> SAY name
SAME pos 38 ;Are they here?
PROCESS y ;Decode speech…
DONE ;LS destroyed so always DONE.

> SAY name
MESSAGE z ;"They are not here!"
DONE
```

With a "PROCESS y" similar to:

```
> _ _
PARSE ;Always do this entry
MESSAGE x ;"They don't understand"
DONE

> word word
CondAct list ;Any phrases PSI understands

> _ _
MESSAGE x ;as above or different message
```

NEWTEXT

Forces the loss of any remaining phrases on the current input
line. You would use this to prevent the player continuing without
a fresh input should something go badly for his situation. e.g.
the GET action carries out a NEWTEXT if it fails to get the
required object for any reason, to prevent disaster with a
sentence such as:

GET SWORD AND KILL ORC WITH IT

as attacking the ORC without the sword may be dangerous!

SYNONYM verb noun

Substitutes the given verb and noun in the LS. Nullword (Usually
'\_') can be used to suppress substitution for one or the other
- or both I suppose!

e.g. MATCH ON SYNONYM LIGHT MATCH

```
> STRIKE MATCH
SYNONYM LIGHT _

> LIGHT MATCH
; Actions...
```

will switch the LS into a standard format for several different
entries. Allowing only one to deal with the actual actions.

Actions which deal with jumping, looping and subroutine control

PROCESS procno.

This powerful action transfers the attention of DAAD to the specified Process table number. Note that it is a true subroutine call and any exit from the new table (e.g. DONE, OK etc.) will return control to the condact which follows the calling PROCESS action. A sub-process can call (nest) further process' to a depth of 10 at which point a runtime error will be generated.

REDO

Will restart the currently executing table.

DOALL locno+

Another powerful action which allows the implementation of an
'ALL' type command.

* 1 - An attempt is made to find an object at Location locno.
  If this is unsuccessful the DOALL is cancelled and action DONE is performed.
* 2 - The object number is converted into the LS Noun1 (and
  Adjective1 if present) by reference to the object
  definition section. If Noun (Adjective)1 matches
  Noun (Adjective)2 then a return is made to step 1. This
  implements the "Verb ALL EXCEPT object" facility of the
  parser.
* 3 - The next condact and/or entry in the table is then
  considered. This effectively converts a phrase of "Verb
  All" into "Verb object" which is then processed by the
  table as if the player had typed it in.
* 4 - When an attempt is made to exit the current table, if
  the DOALL is still active (i.e. has not been cancelled
  by an action) then the attention of DAAD is returned to
  the DOALL as from step 1; with the object search
  continuing from the next highest object number to that
  just considered.

The main ramification of the search method through the object
definition section is objects which have the same Noun (Adjective) description
(where the game works out which object is referred to by its presence) must be checked
for in ascending order of object number, or one of them may be missed.

Use of DOALL to implement things like OPEN ALL must account for
the fact that doors are often flags only and would have to be
made into objects if they were to be included in a DOALL.

The DOS interpreter supports an extended DOALL mode where DOALLs can be nested if
they are in different processes (i.e. while in DOALL, a call to PROCESS is made,
and that process has another DOALL). To enable that, you have to call the
interpreter with -NDOALL (nested doall) parameter. It’s not likely that can be used
often as it’s not supported by other interpreters, but it’s worth knowing.

SKIP distance | label

where distance is -128 to 128, or to the specified label.

Will move the current entry in a table back or forward. Parameter -1 means restart current entry, -2 the previous one, etc. It also can jump forward where 0 is next entry, 1 the one after, etc.
There is no error checking so it should be possible to jump out of the table (what would be fatal).

Skip can also accept as a parameter a local symbol. This is a symbol preceded by a $ sign. They are local to each process table and will not affect any global symbols used. The big advantage is that they can forward reference.

This is implemented with rear patching. The symbol is defined by placing it in the source file immediately before the entry it refers to:

e.g.

```
$backloop
> _ _
PRINT Flag
MINUS Flag 1
NOTZERO Flag
SKIP $backloop

> _ _
ZERO Error
SKIP $Forward

> _ _
EXIT 0

$Forward

> _ _
…
```

Actions which completely exit Response/Process execution

RESTART

Will cancel any DOALL loop, any sub-process calls and make a jump
to execute process 0 again from the start.

END

SM13 ("Would you like to play again?") is printed and the input
routine called. Any DOALL loop and sub-process calls are
cancelled. If the reply does not start with the first character
of SM31 a jump is made to Initialize. Otherwise the player is
returned to the operating system - by doing the command EXIT 0.

EXIT value

If value is 0 then will return directly to the operating system.
Any value other than 0 will restart the whole game. Note that
unlike RESTART which only restarts processing, this will clear
and reset windows etc. The non-zero numbers specify a part number to
jump to on AUTOLOAD versions. Only the PCW supports this feature now.
It will probably be added to PC as part of the HYPERCARD work. So
if you intend using it as a reset ensure you use your PART number
as the non-zero value!

Exit table actions

DONE

This action jumps to the end of the process table and flags to
DAAD that an action has been carried out. i.e. no more condacts
or entries are considered. A return will thus be made to the
previous calling process table, or to the start point of any
active DOALL loop.

NOTDONE

This action jumps to the end of the process table and flags DAAD
that #no# action has been carried out. i.e. no more condacts or entries
are considered. A return will thus be made to the previous calling process
table or to the start point of any active DOALL loop. This will
cause DAAD to print one of the "I can't" messages if needed. i.e. if no
other action is carried out and no entry is present in the connections
section for the current Verb.
OK

SM15 ("OK") is printed and action DONE is performed.

Action to deal with sound

MOUSE value value

Only available in MSDOS and HTML. Does nothing in other targets.

The second value indicates an action to perform with the mouse. The first is a parameter for that action.

| Action | Parametet 1 | Parameter 2 |
| --- | --- | --- |
| Reset mouse (DOS only) | Any | 0 |
| Show pointer | Any | 1 |
| Show pointer | Any | 2 |
| Read position: the flag in the first parameter is the first in a list of flags. That flag will contain the state of the buttons, the next flag will contain the X position divided by 8 (in characters), and the next the Y position divided by 8 (the row). In the next flag the X position divided by 6 (narrow columns) will be stored. | flagno | 3 |
| Read position(fine) : Similar to the previous one, but in the flag + 1 the X position divided by 2 will be stored if we are in VGA mode, or divided by 4 if it is SVGA, and in the flag + 2 the Y position divided by two in SVGA mode, and not divided if it is VGA. | flagno | 4 |
| Modify pointer (DOS only): load the file with PTR extension defined by first parameter. If first parameter is 1, it loads 001.PTR | Pointer number | 5 |
| Changes hotspot coord X value (DOS only): originally the hotspot in the pointer is at x=0, y=0, that is the upper left corner of the opinter. If you change the pointer maybe you would like to move the hotspot. For instance, if the pointer is a cross, you may want to put it at 5,5, which is the center of the 9x9 area. | X | 6 |
| Changes hotspot coord Y value (DOS only): same as before, but for Y value. | Y | 7 |

* The flag that contains the current status of the buttons will contain a 1 if the left button is pressed, a 2 for the right and a 4 for the center. If there is more than one
  pressed the values are added. For example, left and right at the same time, the value will be 3.
* PTR files are 81 bytes long, 9x9, each byte represents a pixel in a 9x9 square. Color 0 is transparent.

For instance, this code would allow you to use the mouse in DAAD:

```
MOUSE 0 0 ; Resets mouse
MOUSE 0 1 ; Shows pointer
MOUSE 100 4; Reads fine position in flags 100, 101 and 102
```

There are specific symbols in the compiler that allow us to not remember the action numbers. See the symbols in the appendices.

`BEEP value value`

Being first value the duration 1/50th of a second (i.e. 50 = 1 second), and the second value the frequency.

For a list of frequencies see the XBEEP condact further in this manual.

Actions to call external DAAD Routines

There are four actions used to call an external routine to DAAD: SFX, GFX, EXTERN and CALL. Despite they are powerful, they are also complicated, and EXTERN is used internally by DAAD Ready, so we will not detail them in the DAAD Ready manual. Please refer to original DAAD manual.

Actions to implement the primary graphics handling

Much as in the previous section, PICTURE and DISPLAY are used mainly for vector graphics, and DAAD Ready is not using them. It may affect you when creating games with graphics for DOS, PCW, Amiga and ST, but once again, we suggest you check DAAD original manual.

`SFX value value`

SFX is explained in depth in the [multimedia manual](multimedia_en.html).

`GFX value value`

X is explained in depth in the [multimedia manual](multimedia_en.html).

`PICTURE picno`

Checks whether the graphic whose number is picno, exists. If so, the next condact is executed, if not, it goes to the next entry. Also, in targets where there is RAM enough to keep a copy of graphics (in general, machines with more than 64K),
it loads the graphic in RAM, but doesn't paint it on the screen.
DISPLAY value

If value is 0, the graphic previously selected by PICTURE is painted on screen. If PICTURE could load it to RAM previously, it will be painted from RAM, otherwise, it will be loaded from disk at that time.

If the value is not 0 and the graphic is not a subroutine, then the graphic area is cleared.

###### Maluva Condacts

Maluva was a DAAD extension included with DAAD Ready, but it's not an extension anymore. In lieu, the some of the condacts provided by Maluva are now part of the interpreters. These condacts
are not available in the Spectrum 48K target, Amiga and ST.

`XMES "string"`

Writes the string in quotes. Note that this condact allows additional text beyond those in the message tables and the MES, MESSAGE, SYSMESS, and DESC condacts, so your game can display a lot more text. XMES has the following limitations:

* Each message may not exceed 511 characters.
* All messages used by XMES and XMESSAGE must not exceed 64K once compressed.
* These xmessages are read from disk on demand, so depending on the reading device, response time may be slow. Use them with care.
* This also means that if you're using split-mode graphics (see below), visual glitches or screen blackening may occur while reading from disk. If you must use XMES in a long adventure, avoid split-mode if possible.

`XMESSSAGE "string"`

Same as XMES but adds a NEWLINE at the end.

`XSPLITSCR value`

For machines that support split screen (currently Amstrad CPC and Commodore 64), this condact defines how it's done.

Split screen means having a different graphics mode at the top of the screen (where images appear) and at the bottom (where text is shown).

This is what "value" means for each machine:

| Machine | value | Top mode | Bottom mode |
| --- | --- | --- | --- |
| CPC | 0 | CPC Mode 1 | CPC Mode 1 |
| CPC | 1 | CPC Mode 0 | CPC Mode 1 |
| CPC | 2 | CPC Mode 2 | CPC Mode 1 |
| C64 | 0 | C64 HiRes | C64 HiRes |
| C64 | 1 | C64 Multicolor | C64 Hires |

The screen always split at scanline 96, that is unavoidable.

`XUNDONE`

XUNDONE changes the internal "done" status. Every time DAAD executes an action, the interpreter sets the internal "done" flag. That means ISDONE or ISNDONE will succeed or not depending on this status.

Note that unlike NOTDONE, XUNDONE just clears the status without jumping to the end of the current process.

Sometimes, even when an action executes in a process, we don’t want it to mark the "done" status. A clear case is using SYNONYM: although it’s technically an action, from the player’s perspective, it does nothing.

```
> TURN ON LIGHT
SYNONYM PRESS SWITCH
```

If you later use this entry and the switch was already pressed, it will fail:

```
> PRESS SWITCH
ZERO fSwitchPressed
MESSAGE "You turn on the light."
SET fSwitchPressed
DONE
```

However, since DAAD thinks something was already "done" because of the SYNONYM, instead of a "I don’t understand," you get:

```
What now?
> PRESS SWITCH
What now?
>
```

In such cases where something was "done" but you don’t want to mark the internal "done" flag, use XUNDONE.

Note that there’s no XDONE condact to set "done," because you can use any condact that does nothing, like COPYFF 100 100.

###### Meta-Condacts

A meta-condact isn’t a real condact in the sense that DAAD doesn’t know it exists. It’s actually a compiler trick that allows writing some things in a more convenient and human-readable way.

`XPLAY "string"`

DAAD can make sounds with the BEEP condact, which is handy for a beep, but not ideal for melodies—even short ones.

XPLAY lets you specify a melody in a format closer to music notation, which the compiler converts into a series of BEEPs and PAUSEs (for silences), which DAAD understands.

Only one channel is supported.

The expected string is a subset of the MML format (see below).

The MML Format

| Symbol and meaning |
| --- |
| [A-G][#:sharp][num:duration][.:dot] The dot increases duration |
| L = Note duration [1-64] (1=whole note, 2=half note, 3=third note, ..., default is 4) |
| R = Rest [1-64] |
| N = Note pitch [0-96] |
| O = Octave [1-8] (default: 4) |
| T = Tempo [32-255] (quarter notes per minute, default is 120) |
| V = Volume [0-15] (default: 8) |
| < = Lower octave |
| > = Raise octave |

This string plays Boccherini's minuet:

`XPLAY "T110S3M5000O5G16F#16G16A16G8O4G4B4O5D8D8C8C4C16O4B16O5C16D16C8O4D4A4O5C8C8O4B8B4O5G4.E16D8C#8C#8C#8G4.E16D8C#8C#8C#8G4.E16F#8D8O4B8O5G8E4.D32E32D4"`

XDATA

The XDATA meta-condact serves to shorten a series of LETs. The first value is a flag number, and the rest are values to assign to that flag and the following ones.

For example:

`XDATA "100,2,8"`

becomes:

```
LET 100 2
LET 101 8
```

It’s very useful when you want to organize certain data in flags and makes it easier to write lots of LETs. For example, in an RPG you might have CON, INT, and DEX for 8 types of enemies starting at flag 100. You could write:

`XDATA "100, 2,8,9, 3,3,7, 6,10,4, 5,1,40, 6,2,3, 104,5,4, 8,5,12"`

And this would be compiled to:

```
LET 100 2
LET 101 8
LET 102 9
LET 103 3
LET 104 3
LET 105 7
LET 106 6
LET 107 10
LET 108 4
LET 109 5
LET 110 1
LET 111 40
LET 112 6
LET 113 2
LET 114 3
LET 115 104
LET 116 5
LET 117 4
LET 118 8
LET 119 5
LET 120 12
```

As you can see, you separate the values by comma, but you can also leave empty space to separate data visually.

###### DAAD for Quill/Paw developers

This is a list of notes for people that used Gilsoft PAWS or The Quill

The processes

* 0 -> Do not change it
* 1 -> Do not change it
* 2 -> Do not change it
* 3 -> This works like PAW's process 1
* 4 -> This works like PAW's process 2
* 5 -> This works like PAW's response table
* 6 -> Initialization process, you can show some messages here, or pictures, initilalize flags, etc. if you add them at the end of process etc.

Objects

DAAD allows object attributes, which means each object has 16 values that can be active (1) or inactive (0). You can see that in the /OBJ section. Using HASNAT and HASAT condActs,
you can check if a given object has a given value for some attribute. For instance, you can define that a specific attribute means the object is sharp,
then you can make puzzle that can be solved with any object having the "sharp" attribute.

Indirection

It has been explained above, and it was not supported by PAW nor Quill.

Message printing

Rather than DAAD, this is an advantage of DRC, the new DAAD compiler. As you have seen in the description of MESSAGE, MES, SYSMESS and DESC, you can now just make MESSAGE have a string as parameter, so you do not have to bother with message numbers.

Also, ther is the XMESSAGE condAct, that also expects a string as parameter, and will give you 64K more of messages for longer adventures. Please notice xmessages are limited by memory used (64K) and not by number of messages.

Also is DRC the one that handles international character printing and takes care about repeated messages, so if you write same message twice, only one is used internally

###### Errors

Although we do as much checking as we can in the compiler, there are a few errors that cannot be detected until runtime.

The interpreters can throw up several types of error. Usually in a little window at the top left on 8 bit and centered on 16 bits, but if during a SAVE they will be printed in the current input stream as they do not cause the game to restart.

The errors are:

* I/O Error : Something wrong happened while loading or saving a file.
* BREAK : Player pressed break
* Error n: This is a machine specific error, please refer to that specific machine manual to find what error number n is.
* Game Error n:
  + 0 - Invalid object number
  + 1 - Illegal assignment to HERE (Flag 38)
  + 2 - Attempt to set object to loc 255
  + 3 - Limit reached on PROCESS calls
  + 4 - Attempt to nest DOALL
  + 5 - Illegal CondAct (corrupt or old db!)
  + 6 - Invalid process call
  + 7 - Invalid message number
  + 8 - Invalid PICTURE (drawstring only)

###### The parser

The parser works by scanning an input line (up to 125 characters)
for words which are in the vocabulary, extracting 'Phrases' which
it can turn into Logical sentences.

When a phrase has been extracted, the Response and Connections
tables are scanned to see if the Logical Sentence is recognized.
If not then system message 8 ("I can't do that") or system message 7
("I can't go in that direction") will be displayed depending on the Verb value (i.e. if
less than 14 then system message 7 will be used) and a new text input is requested. A
new text input will also be requested if an action fails in some way (e.g. an object too heavy) or
if the writer forces it with a NEWTEXT action. The results might otherwise be catastrophic for the player.
e.g. GET AXE AND ATTACK TROLL, if you do not have the axe you wouldn't really want to tackle the Troll!

If the LS is successfully executed, then another phrase is extracted, or new text requested if there is no more text in the
buffer.

Phrases are separated by conjugations ("AND" & "THEN" usually) and by any punctuation.

In English, a Pronoun ("IT" usually) can be used to refer to the
Noun/Adjective used in the previous Phrase - even if this was a
separate input. Nouns with word values less than 50 are Proper
Nouns and will not affect the Pronoun. Spanish implementes same
feature with pronominal suffixes like -lo, -la , -los, -las.

The Logical Sentence format is as follows: -

`(Adverb)Verb(Adjective1(Noun1))(preposition)(Adjective2(Noun2))`

where bracketed types are optional. i.e. the minimum phrase is a
Verb (or a Conversion Noun - a Noun with a word value <20 - which
if no Verb is found in a phrase will be converted into a Verb
e.g. NORTH). If the Verb is omitted, then the LS will assume the
previously used Verb is required. i.e. GET SWORD AND SHIELD will
work correctly! The current 'IT' (pronoun) will become the first
Noun in a list like this. I.e. 'IT' would be replaced with SWORD in
the example. It (if you will excuse the pun) will not change
until a different Verb (or conversion Noun) is used.

Note that the phrase does not strictly have to be typed in by the
player in this format. As an example:

```
GET THE SMALL SWORD QUICKLY
QUICKLY GET THE SMALL SWORD
QUICKLY THE SMALL SWORD GET
```

are all equivalent phrases producing the same LS. Although the
third version is rather dubious English.

A true sentence could be: -

```
GET ALL. OPEN THE DOOR AND GO SOUTH THEN GET THE BUCKET AND
LOOK IN IT.
```

which will become five LS’s: -

```
GET ALL
OPEN DOOR (because THE is not in the vocabulary)
SOUTH (because GO is not in the vocabulary)
GET BUCKET
LOOK BUCKET (from IT) IN (preposition)
```

Note that DOALL will not generate the object described by
Noun (Adjective)2 of the Logical sentence. This provides a simple
method of implementing EXCEPT. e.g. GET ALL EXCEPT THE FISH, it
has the side effect of not allowing PUT ALL EXCEPT THE FISH IN
THE BUCKET, as this has three nouns!

Spanish

If a Verb is less than 5 letters you will need to include the lo, la, los and las versions in the vocabulary. Obviously if it is four
letters you only need 'l' ending as a synonym (TOMA -> TOMAL) if it is three letters you need 'lo' and 'la' (PON -> PONLO, PONLA)
synonyms and if it is one or two letters you need 'lo','la','los' & 'las'! (DA -> DALE, DALOS, DALA, DALAS)

If you have a plural noun in the game which changes its stress
then you need to include the stressed and unstressed.

The Spanish Parser deals with NOUNS, PRONOUNS and ADJECTIVES differently to the English. Specifically, it assumes that adjectives FOLLOW nouns,
and does not deal with compound nouns. A compound noun is where an object is described by two nouns such as
PARK BENCH. where the player may use either or both words to describe the item. E.g. GET BENCH, GET PARK, GET PARK BENCH.
PARK and BENCH would probably be synonyms and the problem does not arise until you use a second noun. E.g. PAINT PARK
BENCH WITH BRUSH. Here BRUSH should be NOUN2, but the parser would assume PARK was NOUN1 and BENCH was NOUN2.
The English parser deals with this situation, but it is complicated to do for Spanish.

Other languages

Despite DAAD it is only supporting English and Spanish, DAAD Ready has a limited support for other languages as Portuguese,
French and German. That means, DAAD Ready is able to print special characters for those two languages, as "õ" or "ß",
but doesn’t make any change in the parser, so the Portuguese games will be using the Spanish interpreter and the German
and French games will be using the English interpreter. Please bear that in mind if creating games for those languages.

There is no support for other languages right now, but DAAD Ready supports
printing characters from other languages, basically most of the characters
in the LATIN-1 alphabet are printable.

###### System Flags

The normal flags are free for use in any way in games. But if you look at the TEST.DSF file, you will see it defines a use for every flag from 0-63 - the 'system' flags. It also defines symbolic names for the system flags. Although DAAD does not reference all of these (only those shown below) they may be treated specially by future upgrades so treat them with care.

The best way to test the bit defined flags is to use the [HASAT](#hasat) CondAct. For example, HASAT MOUSE will be true if a mouse is present.

| Flag # | Usage/meaning |
| --- | --- |
| 0 | When nonzero indicates game is dark (see also object 0) |
| 1 | Holds quantity of objects player is carrying (but not wearing) |
| 2 to 28 | These are not actually used by the DAAD interpreters anywhere. Thus, they would be free for use in your own games, but on the other hand the TEST.DSF file included within DAAD Ready it is using flag 28 for internal functionalities, and Maluva extension is using flag 20. Also MSDOS target uses flag 21 (see below). Thus, you should not use those flags, and in fact our recommendation is, to avoid conflicts with future expansions, that you start using flag 254 for your own tasks, then 254, then 253, etc. That way you will be far from any conflict and your game may be recompiled in 20 years without problems. |
| 21 | MSDOS exclusive, used for sound control:     | Bit | Meaning | | --- | --- | | 0 | 1 = SFX engine enabled | | 2 | 1 = SFX plays in loop mode | | 4 | 1 = SFX is playing |   Please notice you can write to this flag, but will have no effect, except for loop bit, wich in case of being changed will affect the loop. |
| 29 | Bit 0 - Mouse present (16 -it only). |
| 30 | Score flag - not actually used directly by DAAD but its traditional! |
| 31/32 | (LSB/MSB) holds number of turns player has taken (actually, this is the number of phrases extracted from the players input). |
| 33 | holds the Verb for the current logical sentence |
| 34 | holds the first Noun for the current logical sentence |
| 35 | holds the Adjective for the current logical sentence |
| 36 | holds the Adverb for the current logical sentence |
| 37 | holds maximum number of objects conveyable (initially 4) Set using ABILITY action |
| 38 | holds current location of player |
| 39/40 | Unused |
| 41 | Gives stream number for input to use. 0 means current stream. Used Modulo 8. I.e. 8 is considered as 0! |
| 42 | holds prompt to use a system message number - (0 selects one of four randomly) |
| 43 | holds the Preposition in the current logical sentence |
| 44 | holds the second Noun in the current logical sentence |
| 45 | holds the Adjective for the second Noun in the current logical sentence |
| 46 | holds the current pronoun ("IT" usually) Noun |
| 47 | holds the current pronoun ("IT" usually) Adjective |
| 48 | holds Timeout duration required |
| 49 | holds Timeout Control flags. Set bitwise:  | Bit | Meaning | | --- | --- | | 7 | Set if timeout occurred last frame | | 6 | Set if data available for recall (not of use to writer) | | 5 | Set this to cause auto recall of input buffer after timeout | | 4 | Set this to print input in current stream after edit | | 3 | Set this to clear input window | | 2 | Set this so timeout can occur on ANYKEY | | 1 | Set this so timeout can occur on "More..." | | 0 | Set this so timeout can occur at start of input only | |
| 50 | holds object number for DOALL loop. i.e. value following DOALL |
| 51 | holds last object referenced by GET/DROP/WEAR/WHATO etc. |
| 52 | holds players strength (maximum weight of objects carried and worn - initially 10). Set with ABILITY action. |
| 53 | holds object print flags, Set bitwise:  | Bit | Meaning | | --- | --- | | 7 | Set if any object printed as part of LISTOBJ or LISTAT | | 6 | Set this to cause continuous object listing. i.e. LET 53 64 will make DAAD list objects on the same line forming a valid sentence. | |
| 54 | holds the present location of the currently referenced object |
| 55 | holds the weight of the currently referenced object |
| 56 | is 128 if the currently referenced object is a container. |
| 57 | is 128 if the currently referenced object is wearable |
| 58/59 | are the currently referenced objects user attributes |
| 60/61 | are the Key flags which give the key code returned after an INKEY condition succeeds. Flag 61 is only relevant on IBM, ST and AMIGA where it is used to provide the extended codes when a cursor or function key is pressed. In this case Flag 60 is zero and Flag 61 contains the IBM extended code. |
| 62 | on ST and PC gives the absolute screen mode in use on the machine. This allows checks to be made as to size of screen etc, but to determine if you are in graphics mode see Flag 29. On the ST 0 means lo-res and 1 med-res. On the PC 4 means CGA, 7 means mono character only, 13 is EGA or VGA and +128 (Bit 7 is set) in VGA to indicate you have palette switching. |
| 63 | defines the currently active window. Note that this is a copy so changing its value will not affect the DAAD system. Important: some interpreters don't update this flag. |
| 64-254 | Available for your own use |

###### The Source File

The source file consists of several inter-related sections
describing the adventure. They usually correspond with the areas
found in the database.

###### Sections

The Control (CTL) section

This section is obsolete and not used anymore. There should be an underscore character in there, but it is just for historical reasons.

The Vocabulary (VOC) section

Each entry in this section contains a word (or the first five
characters of a word), a word value and a word type. Words with
the same word value and type are called synonyms.

The System Messages (STX) section

This section contains the messages used by the Interpreter which
are numbered from 0 upwards. The description of the Interpreter
shows when these messages are used. In addition, extra messages
can be inserted by the writer to provide messages for the game if
required.

The Message Text (MTX) section

This section contains the text of any messages which are needed for the adventure. The messages are numbered from 0 upwards.
With DAAD Ready you don’t really need to add messages here like in the old PAWS or original DAAD, you can just write things like this in
the processes, and the compiler will take care of creating a message for you. It will even find if you are using the
same text twice or more, and use same message in that case:

`MESSAGE "Hello World!"`

Still, the table may be used because you want to have some messages with some specific number, to be used with indirection (i.e. MESSAGE @100, which will print the message whose number is at flag 100).

The Object Text (OTX) section

This section, which has an entry for each object, contains the text which is printed when an object is described. An object is anything in the adventure which may be manipulated, and objects are numbered from 0 upwards. Object 0 is assumed by the Interpreter to be a source of light.

The Location Text (LTX) section

This section, which has an entry for each location, contains the text which is printed when a location is described. The entries are numbered from 0 upwards and location 0 is the location at
which the adventure starts.

The Connections (CON) section

This section has an entry for each location and each entry may
either be empty (null) or contain several movements. A movement consists
of a Verb (or conversion Noun) from the vocabulary followed by a location number.
This means that any Verb (or conversion Noun) with that word value causes
movement to that location. A typical entry could be: -

```
SOUTH 6
EAST 7
LEAVE 6
NORTH 5
```

which means that SOUTH or LEAVE or their synonyms cause movement
to location 6, EAST or its synonyms to location 7 and NORTH or
its synonyms to location 5.

Note 1. When the adventure is being played it is only the LS Verb
which will cause movement.

Note 2. If a movement is performed by an entry in the Response
table using the GOTO action, then it may not be needed in
the Connections table, unless that entry is required for
a PSI who can move unconditionally.

The Object Definition (OBJ) section

This section has an entry for each object which specifies:

* The object’s number
* The location at which the object is situated at the beginning of the adventure.
* The object’s weight (0-63).
* Whether the object is a container.
* Whether the object can be worn/removed.
* The state of the other object attributes
* The noun and adjective associated with the object.

The Process tables (PRO) section

This section forms the heart of the source file providing the
main game control. Each table consists of several entries. Each entry
contains the Verb and Noun for the LS the entry is to deal
with followed by any number of condActs. When the adventure is played if there is an entry
in the table which matches the Verb and Noun1 of the LS entered then the condActs are performed.
The condActs that may be present and the effect that they have is fully specified in
the description of the Interpreter. The LS can of course, be set using a method other than the PARSE
action. This would allow the creation of a menu system, multiple-choice entry etc.

Process table 0

This contains the main control of a DAAD program. It is entered
after initialization with the current LS empty. It will normally
consist of '> \_ \_' word entries and some form of looping.

Process 1 (and upwards)

These are optional and define sub-processes that can be
referenced using the PROCESS action.

Processes bundled with DAAD Ready

Please notice the DAAD Ready TEST.DSF file already contains more processes, to simulate the way of working of Gilsoft's PAW. Thus, Process 0, 1, 2 and 6
are internal to build that, but process 3 is much like process 1 in PAW, process 4 is much like process 2, and process 5 just like the response table.

###### Escape Characters

While printing message, you can use some specific characters that will print not just what you they look like, but something in place. Please look at this table to understand what is printed:

| Escape code | Description |
| --- | --- |
| \_ | It is replaced by the current referenced object description, if an indefinite article is found, it’s removed. That allows system messages be like "You take the \_.", so object "a lamp" gets "You take the lamp." as associated message. |
| @ | Same as the underscore, but the article has its first letter uppercased. Only works for Spanish interpreter. |
| #b or #s | Prints a blank space |
| #k | When this character is printed, the game is paused and waits for a key to be pressed. |
| #n or \n | Prints a carriage return |
| #g | Prints nothing, but from this moment on, every text printed will be using the alternative font. So, for instance #gt will print the character "t" of the alternative font |
| #t | Opposite to #g, starts using again the default character set. |
| #e | Prints the euro sign (€). |

###### Preprocessor commands

Please notice this section is not for newbies, so do not worry if you don’t get a word. You can even skip it if starting with DAAD once you have seen the #define and #ifdef definitions.

All preprocessor commands are preceded by a hash or gate character ('#'). They must occupy a line of their own. Generally, they can be placed almost anywhere, although it may not be appropriate or useful!

#define symbol expression

Define the case sensitive label to have the value given by expression. This can consist of other symbols, numbers, and operations between quotes. The symbols can usually be used anywhere a number is required along with fixed numbers and the operators. This does not apply to IF statements which accept a single symbol only.

```
#define hours 24
#define days 2
#define weekendHours "days*hours"
#define noon "hours/2"
```

Note that as the compiler is single pass you must define all
symbols before you use them. This does not apply to local symbols
(those preceeded with a $) as detailed elsewhere.

Please notice #define is there for two main reasons:

1) Being able to refer things by name instead of number, for instance if object 2 is a lamp, then you can make a define and reference it like this:

```
#define oLamp 2

CARRIED oLamp
DESTROY oLamp
```

Instead of using this old approach:

```
CARRIED 2
DESTROY 2
```

It also increases code readability if you must come back some day and fix something you wrote months or years ago.

2) To use with "ifdef" (see below).

```
#ifdef "symbol"
{#else}
#endif
```

This group of commands will occur together. Note that the #else is
optional. Any lines following the #ifdef (up until the next #else or #endif) will be included in the compilation only if the symbol
has a non zero (TRUE) value. If you use #ifndef instead of #ifdef, then it happens if FALSE.

```
#ifdef "zx"
MESSAGE "It’s hard to do that with a Spectrum"
#else
MESSAGE "It’s hard to do that without a Spectrum"
#endif
```

If you add that to the code, when the target is "ZX" (compiling for any ZX Spectrum target) players will get the first message, otherwise they will get the second one.

Although this is a simple example, you can use that to create different code that compiles depending on whether symbols have been defined or not.

#ifdef "symbol"

The code between ifdef and #endif will be compiled only if symbol is defined.

You can also use #else to say which code to be included if symbol is not defined.

#ifndef "symbol"

Same as above, but the code inside is included only if symbol is not defined.

You can also use #else with ifndef, so

```
#ifdef "X"
XXXX
#else
YYYY
#endif
```

is basically the same as:

```
#ifndef "X"
YYYY
#else
XXXX
#endif
```

`#include "filespec"`

Will switch the compiler to use the specified source file. At the
end of the file the compiler returns to the line after the
#include in the original source file. Includes can't be nested.

`#echo "text"`

Output text to the console during compilation - usually to
indicate titles and the inclusion of a file with conditional
options.

e.g.

```
#ifdef "PC"
#echo Including IBM Display handler
#include \LIB\PCDISP.DSF
#endif
```

`#incbin "filespec"`

Will include a memory image file at the current position in the
database. This allows just about any type of data to be included
in the database segment.

`#defb "expression"`

Will include the indicated byte value(s) in the database at the
current address. For example:

```
#DEFB 1
#defb PSIFLAG-1
```

`#defw "expression"`

Will include the indicated word value(s) in the database at the
current address. For example:

```
#defw 6578
#defw IOADDR+4
```

#defb/w commands may be used in preference to INCBIN if you need
to include only a few values or ones that are calculated from
symbol values.

`#dbaddr symbol`

Will give symbol the current address in the database. This can be
sed for CALL, #userptr or #defw commands.

`#userptr n`

Where n is from 0-9.

This command is designed to overcome the forward reference
limitation of the single pass compiler. It places the current
database address in one of ten vectors whose position is fixed at
the start of the database. Thus, an external routine can locate
inserted data in the database by looking at the fixed vector.

`#extern "filespec"`

`#sfx "filespec"`

`#gfx "filespec"`

For 8 bit only. These three commands are like #USERPTR, but
the value stored by them is copied by the interpreter on database
load to the corresponding EXTERN vector. Note that SFX already
has a default effect of writing to the sound chip registers. So
any routine using this vector will replace this function. The
EXTERN.SCE file uses EXTERN to achieve the sound so that the SFX
command continues to function.

Now any EXTERN or SFX commands will be directed to your routine.

The interpreters recognize 3 external vectors. The third is for a
50Hz interrupt: -

`#int "filespec"`

For 8 bit machines only. Any routine placed on this vector will
be called 50 times a second for the entire period that the game
is running. Thus, there is no command in the DAAD language to call
the routine. The Z80 ones save only the AF and HL registers (as
HL is given as your address), so be sure to save any other
registers you use. The 6502 interpreters save the entire
processor state.

If you include the filename in the #EXTERN, #SFX or #INT commands
then it will cause the given file to be included in the database
- as if the command was followed by #INCBIN.

#### Appendix

###### Appendix A - The character set

Throughout the whole DAAD system the same character set of 256
characters is used.

DAAD Ready is using some of the chars in the alternative charset to represent international character other than the original Spanish ones already supported.

DAAD Ready allows you to change the font for your game:

If you want to change your game font, you just must change the files at the ASSETS/CHARSET folder. These are the files:

* AD8x6.CHR is for Spectrum, MSX, Amstrad PCW, Amiga, AtariST and OLDMSDOS.
* AD8x8.CHR is used by Amstrad CPC
* C64bold.CHR is used by C64 and Plus/4.
* MSDOS.FNT is used for MSDOS.

To modify the font, you can use GCS application at TOOLS\GCS.ZX-Paintbursh can also open them. If you save the font from GCS, save it for 8 bit, even if you want to use it for 16 bit targets, with the only exception of MSDOS.FNT, tha you must save in SINTAC format.

Please consider this:

* AD8x6.CHR font is shown as a 6 pixels wide font, so you must avoid using three rightmost columns. The two rightmost ones will not be even shown, the third one you better keep empty to avoid characters appear to close from each other.
* AD8x8.CHR font is shown using 8 pixels wide font. Despite that, it is the same font, so text in CPC is shown with more gaps between characters. You can modify the font to take two more columns, try not to use last column for the same reason said above.
* C64bold.CHR for C64 is a bold font, because CRT TVs connected to C64 via RF connection have problems showing thin letters in a way that is readable.

Please take in mind if you redefine characters in the alternative font used for languages other than English, you won’t be able to use the for that language, but if you need to redefine the characters, you can always use one of the character you are not using (i.e. "ñ" if your game is in German or "õ" if your game is in Spanish).

###### Appendix B - Daad Ready customizations

DAAD Ready saves some settings in the CONFIG.BAT file, which is loaded by all the other .BAT files (one per target). Also, DAAD Ready checks if CUSTOM.BAT file exists, and if so, loads it after CONFIG.BAT. That allows third-party tools, as Adventuron, integrate with DAAD Ready and modify values in CONFIG.BAT file.

Also, the bat files for each machine have several points where some external files are called if they exist, check the BAT files for more information.

###### Appendix C - Supported languages

Although DAAD only supported Spanish and English originally, limited support for Portuguese and German has been added. You can
skip this section if you are going to create a game in English or Spanish.

"Limited" means that although you will be able to create games that display text using characters from those languages like "ß" or "õ", the underlying interpreter will always be English (for German or French) or Spanish (for Portuguese), so the player will not be able to give orders to the game that include those characters, and the analysis of the orders will think it is English or Spanish, with the consequences that may have. It also affects the list of objects:

* In English games, when the description of an object is added to a message like "You take the \_.", the underscore is replaced by the object text without the article. So, when that message is shown for an object called "a lamp", it will show "You take the lamp." instead of "You take the a lamp."
* In Spanish games, the same happens, but the article is kept, so the message "Tomas \_." will show "Tomas la lámpara." instead of "Tomas lámpara."

Sadly, this behavior is very specific to the language, so games in German or French will not have their articles modified, because they do not match the expected ones (also, German has
three genres). As a consequence, a reasonable solution has been taken for those languages: in Portuguese, the messages will use the indefinite article, and in German or French the object
name is avoided in the response and answers with a simple "OK, taken."

Other languages can be used with DAAD Ready in a similar way to Portuguese, French and German. Please join the Telegram groups if you want to create support for other languages. These are the characters that DAAD Ready supports apart from English, if they match your language in general it will be possible:

```
ª ¡ ¿ á é í ó ú ñ Ñ ç Ç ü Ü à ã â ä è ê ë ì î ï ò ô ö õ ù û Ý
Á É Í Ó Ú Â Ê Î Ô Û À È Ì Ò Ù Ä Ë Ï Ö Ü ý Ý þ Þ å Å ð Ð ø Ø ß
```

If your language uses other special symbols, it is probably still viable, as long as it uses the Roman alphabet.

###### Appendix D - Symbols

DAAD Ready provides a series of values that you can use in your code, so you don't have to calculate them, and even in some cases, it will provide you with a different value depending on the .BAT you use,
so you won't have to put something different depending on the machine you work with. For example, there is a symbol called "NOT\_CREATED" that
is worth 252. You can remember to put 252 in your code, or put NOT\_CREATED instead, which will be easier to
remember (ISAT oKey NOT\_CREATED instead of ISAT 7 252)

This is the list of symbols that DAAD Ready gives you by default, apart from those you can create with #define:

* COLS : Number of text columns in the target machine.
* ROWS : Number of text rows in the target machine.
* NOT\_CREATED : 252. It is the value used to indicate that an object has not been created.
* WORN : 253. It is the value used to indicate that an object is worn.
* CARRIED : 254. It is the value used to indicate that an object is carried.
* HERE : 255. Some condacts allow using this value to compare with the current location, instead of 255.
* DSTRINGS: although of little use in DAAD Ready, if this symbol is defined, it indicates that the machine originally supported vector graphics.
* LAST\_OBJECT: the number of the last object in the object list.
* LAST\_LOCATION: the number of the last location in the location list.
* NUM\_OBJECTS: the number of objects in the game.
* NUM\_LOCATIONS: the number of locations in the game.
* NUM\_CARRIED: the number of objects the player carries at the start of the game.
* NUM\_WORN: the number of objects the player wears at the start of the game.
* BIT8: This symbol is defined if the target machine is 8 bit.
* BIT16: This symbol is defined if the target machine is 16 bit. Note that "Window Experimental" and "MacOS Experimental" are compiled as if they were Atari ST games, so it will have the BIT16 symbol. The HTML, however, does not have it (nor the 8 bit one).
* YEARHIGH: the two digits of the current year.
* YEARLOW: the lower two digits of the current year.
* MONTH: the current month.
* DAY: the current day. These four together allow you to save in your code the date it was generated:
  ```
  > HELP _
  LET 100 YEARHIGH
  LET 101 YEARLOW
  LET 102 MONTH
  LET 103 DAY
  MES "Game written on "
  PRINT 103
  MES "/"
  PRINT 102
  MES "/"
  PRINT 100
  PRINT 101
  NEWLINE
  DONE
  ```
* Also, a symbol is defined with the target machine: ZX, CPC, C64, CP4, MSX, MSX2, PCW, AMIGA, ST, PC or HTML.
* Also, the subtarget of some machines is added as a symbol, preceded by the text "MODE\_". For example, for ZX, in addition to the "ZX" symbol, "MODE\_PLUS3", "MODE\_ESXDOS", "MODE\_NEXT", or "MODE\_UNO" will be generated according to the case.
* There are also a few symbols made to be used wiTH SFX condact in MSDOS target, to make easier to remember how to play multimedia. These are the Symbols:

  | Symbol | Value |
  | --- | --- |
  | PLAYSFX | 1 |
  | PLAYSFXL | 2 |
  | PLAYSFXF | 3 |
  | PLAYSFXFL | 4 |
  | STOPSFX | 5 |
  | PLAYDRO | 6 |
  | PLAYDROL | 7 |
  | STOPDRO | 8 |
  | PLAYFLI | 9 |
  | FPLAYFLIL | 10 |

  For instance, you can enter "SFX 100 PLAYDRO" instead of "SFX 100 6".
* Similarly, the actions of the second parameter of the MOUSE condact can be used with symbols:

  | Symbol | Value |
  | --- | --- |
  | RESETMS | 0 |
  | SHOWMS | 1 |
  | HIDEMS | 2 |
  | GETMS | 3 |
  | GETFINEMS | 4 |
  | POINTERMS | 5 |
  | DELTAXMS | 6 |
  | DELTAYMS | 7 |

  For instance, you can enter "MOUSE 100 GETMS" instead of "MOUSE 100 3".
* Finally, all the words in the vocabulary are added as symbols, added with the prefix "\_VOC\_". So, if the verb LEAVE is number 80, the symbol \_VOC\_LEAVE will be 80.

Obviously, using these symbols may not be necessary in many adventures, but some can be very convenient. For example, if we have a code that looks at all the objects to see if we carry them, we can make it look from object 0 to 15, because when we write the code the last object is 15, but if we add more objects, the code will not work. Instead, we can use the LAST\_OBJECT symbol, which will always have the value of the last object, and so the code will always work.

###### Appendix E - Greetings

* Tim Gilberts, from Gilsoft/Infinite Imaginations, for creating DAAD (among others)
* Graeme Yeandle and Huw-Hammer Powel, as co-authors of the previous engines together with Tim (PAWS, The Quill, SWAN)
* Andrés Samudio, from Aventuras AD, for allowing public and free distribution of DAAD
* Richard Wilson, for creating WinAPE. Despite is not used by DAAD Ready Anymore, it was used for a long time.
* César Hernández Bañó, for creating ZEsarUX, and for his great help improving ZEsarUX debugger, which was absolutely helpful for Spectrum Next and ZX-Uno targets
* Mochilote, for creating CPCDiskXP
* Attila Grósz, for creating Yape
* Natalia Pujol, for the new MSX2 interpreter
* Imre Szell, for the new Plus/4 interpreter
* Marcin Skoczylas, for creating C64Debugger
* Habi, for creating CP/M Box, PCW Emulator
* To all those working in dosbox, VICE64, OpenMSX, etc.
* To Javier San José, for GCS font editor
* To John Newbigin, for DD for Windows
* To Amstrad and Locomotive, for allowing using their ROMS in Spectrum and CPC emulators.
* To Juan José Torres, for the DAAD logo.
* To Chris Ainsley, for his ideas, and for Visual Studio Code addon.
* To Sergio Chico (Chronomantic), author of the fantastic Spectrum 128K interpreter and utilities.
* To José Luis Cebrián, author of the new Windows and MacOS interpreters (among others)
* To Cesar Nicolás Gonzalez, author of CPCE
* To Mark Adler, John Bush, Karl Davis, Harald Denker, Jean-Michel Dubois,Jean-loup Gailly, Hunter Goatley, Ed Gordon, Ian Gorman, Chris Herborth, Dirk Haase, Greg Hartwig, Robert Heath, Jonathan Hudson, Paul Kienitz, David Kirschbaum, Johnny Lee, Onno van der Linden, Igor Mandrichenko, Steve P. Miller, Sergio Monesi, Keith Owens, George Petrov, Greg Roelofs, Kai Uwe Rommel, Steve Salisbury, Dave Smith, Steven M. Schweda, Christian Spieler, Cosmin Truta, Antoine Verheijen, Paul von Behren, Rich Wales and Mike White, authors of ZIP utility.
* To Zorg, author of MSA Converter
* To PiCiJi, author of Denise
* To Einar Saukas & Urusergi, for ZX0
* To Nicholas Campbell, for the AZERTY and drive B patches for CPC

###### Appendix F - Licenses

DAAD Ready contains software of a lot of parts and has been built by Uto (@utodev), which is author of Maluva (DAAD extension AKA extern) and DRC (New DAAD compiler). All software used is either free or open source with free distribution, and some special cases where license was not specified, permission has been granted by authors.

* DRC and Maluva are (C) Uto and are both subject to their respective licenses. See more information and source code [here](https://github.com/daad-adventure-writer/DRC/wiki#LICENSE).
* ZEsarUX is (C) César Hernández and it is subject to its own license, please see license file at TOOLS/ZEsarUX folder. Also find more information and source code at [Github repository](https://github.com/chernandezba/ZEsarUX/).
* WinAPE is (C) Richard Wilson. WinAPE is Freeware.
* OpenMSX is (C) several authors. Find more information at [OpenMSX website](https://openmsx.org/).
* C64DEbugger is (C) Marcin Skoczylas, although is also using some of the Vice64 code. If you like it and use it consider donating beer to* Denise is (C) PiCiJi, ant it's under GPL v3 license.
  * Yape is (C) Attila Grósz and its freeware. Please find more information at [Yape website](http://yape.plus4.net).
  * DOSBOX is (C) Peter "Qbix" Veenstra, Sjoerd "Harekiet" van der Berg, Tommy "fanskapet" Frössman, Ulf "Finster" Wohlers. Find source code and info at [dosbox.com](https://www.dosbox.com/).
  * CP/M Box (PCW emulator) is (C) Habisoft. Find more information at [Habisoft website](http://www.habisoft.com/pcw/index_uk.asp).
  * MSX2DAAD is (C) NataliaPC, find more information and source code at [its repository](https://github.com/nataliapc/msx2daad/wiki).
  * CPCDiskXP is (C) Mochilote. Get more information at [CPCMania website](http://www.cpcmania.com/news.htm).
  * C1541 is part of Vice64. Find more information at [Vice website](https://vice-emu.sourceforge.io/).
  * dsktool is (C) Ricardo Bittencourt, Tony Cruise and NataliaPC. Find source code and lastest info at [its repository](https://github.com/nataliapc/MSX_devs/tree/master/dsktool).
  * PHP is (C) several users. Please find more information at [php.net](https://www.php.net/).
  * DD for windows is (c) John Newbigin and it's licensed under the GPL v2. Find more information at [Chami website](http://www.chami.com/html-kit/).
  * GCS is (C) Javier San José and Uto, and license it's pending to define.
  * Several ROM images supplied with CPCE and ZEsarUX are still (C) to Amstrad Plc and Locomotive Software. Amstrad and Locomotive have given permission for these ROM images to be distributed with CPC emulators, but retains the (C).
  * WinUAE is (C) Bernd Schmidt, Toni Wilen, Richard Drummond, Mustafa 'GnoStiC' TUFAN, Frode Solheim and Rupert Hausberger , and is subject to GNU GPL license.
  * Hatari is (C)Thomas Hut, Nicolas Pomarède, Eero Tamminen and others, and is subject to GNU GPL license, although it contains a library (DLL) with a more restrictive license, whose restrictivemess only affects distribution together with paid software, which is not the case with DAAD Ready. Please notice anyway that it’s not legal to distribute your game together with our game if it’s not free (as in free beer).
  * ZXDAAD128 and its related tools are GPL and (C) Sergio Chico, DRB128 is (C)UtoDev and Sergio Chico.
  * ADG is (C) Jose Luis Cebrián and is released under the MIT license. Please check [its repository](https://github.com/jlcebrian/ADP) for details.
  * ZIP is © Mark Adler, John Bush, Karl Davis, Harald Denker, Jean-Michel Dubois,Jean-loup Gailly, Hunter Goatley, Ed Gordon, Ian Gorman, Chris Herborth, Dirk Haase, Greg Hartwig, Robert Heath, Jonathan Hudson, Paul Kienitz, David Kirschbaum, Johnny Lee, Onno van der Linden, Igor Mandrichenko, Steve P. Miller, Sergio Monesi, Keith Owens, George Petrov, Greg Roelofs, Kai Uwe Rommel, Steve Salisbury, Dave Smith, Steven M. Schweda, Christian Spieler, Cosmin Truta, Antoine Verheijen, Paul von Behren, Rich Wales and Mike White, and its license is includ-ed in the LICENSE.TXT file at TOOLS/ZIP folder.
  * MSA Converter is (C) Zorg
  * ZX0 is (C) Einar Saukas & Urusergi
