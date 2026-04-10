---
name: daad-system
description: "Technical reference for the DAAD (Disenador de Aventuras AD) text adventure authoring system. Use when working with DAAD interpreters, compilers, game databases, or adventure source code. Covers: DDB binary file format, all 128 condacts, system flags (0-63), object model, DSF source format, DRC compiler, MALUVA extensions, engine architecture (parser, process tables, game loop, indirection, windows, save/load). Use for: (1) implementing or modifying a DAAD interpreter, (2) writing or debugging DAAD adventures in DSF, (3) understanding DDB binary layout, (4) looking up condact behavior or system flag semantics, (5) working with DRC compiler or MALUVA extensions."
---

# DAAD System Reference

DAAD (Disenador de Aventuras AD) is a multi-platform text adventure authoring system created by Tim Gilberts in 1988 for the Spanish company Aventuras AD. It evolved from The Quill -> PAW -> SWAN -> DAAD. Adventures are written in DSF source files, compiled to DDB binary databases by DRC, and executed by platform-specific interpreters.

Supported platforms: ZX Spectrum, Amstrad CPC, C64, MSX, MSX2, PCW, Atari ST, Amiga, PC (DOS).

## Reference Files

Load the appropriate reference based on the task:

### DDB Binary Format
**File**: [references/ddb-format.md](references/ddb-format.md)
- 34-byte header structure (all fields and offsets)
- Data section layout (vocabulary, texts, processes, objects, connections)
- Token-based text compression scheme
- Character encoding (`255 - char`), inline escapes, article modification
- Save file format

### Condacts (all 128)
**File**: [references/condacts.md](references/condacts.md)
- Complete condact table: number, name, args, type (condition/action), description
- GFX routines table (routines 0-10)
- BEEP tone table (8 octaves)
- DOALL mechanism
- HASAT/HASNAT bit reference (object attributes + system flag bits)
- AUTO- condact search priorities

### Flags and Objects
**File**: [references/flags-and-objects.md](references/flags-and-objects.md)
- All 64 system flags (0-63) with names and meanings
- Timeout control bitmask (flag 49)
- Referenced object update (flags 51-59)
- Object structure (6 bytes: location, weight, container, wearable, extended attrs, noun, adjective)
- Special location values (252=not_created, 253=worn, 254=carried)
- Object 0 convention (light source)
- Complete system messages table (SM0-SM60)

### DSF Source Format
**File**: [references/dsf-source-format.md](references/dsf-source-format.md)
- All DSF sections: /CTL, /VOC, /STX, /MTX, /OTX, /LTX, /CON, /OBJ, /PRO, /END
- Vocabulary rules (word types, truncation, synonyms, noun-as-verb)
- Object definition format
- Process entry format and condact sequences
- Standard process architecture (PRO 0-6 roles)
- DRC `#define` symbols and inline messages

### DRC Compiler
**File**: [references/drc-compiler.md](references/drc-compiler.md)
- DRF (front-end) and DRB (back-end) usage
- All compiler directives (`#define`, `#ifdef`, `#extern`, `#include`, etc.)
- Auto-defined symbols per target
- DRC optimizations vs `#classic` mode
- Differences from classic DC (SCE format)
- Related tools (DRT, EAAD, SC2DAAD, DAADMAKER, etc.)

### MALUVA Extensions
**File**: [references/maluva-extensions.md](references/maluva-extensions.md)
- XPICTURE, XSAVE, XLOAD, XMESSAGE/XMES, XBEEP, XPLAY (MML), XPART
- XSPLITSCR, XUNDONE
- Error reporting (flag 20)
- EXTERN equivalents for classic DC compiler

### Engine Architecture
**File**: [references/engine-architecture.md](references/engine-architecture.md)
- Game startup sequence and main loop
- Process call stack (10 levels max)
- Parser and logical sentence system (compound sentences, Noun2 resolution)
- Indirection mechanism (`@` prefix)
- Window system (8 windows, "More..." prompt)
- ISDONE/ISNDONE mechanism
- Darkness system (flag 0 + Object 0)
- Connection tables and MOVE
- Save/Load and RAMSAVE/RAMLOAD
- Runtime error codes
- Historical timeline (1983-2018)

## Quick Lookup

### Key Constants
- NULLWORD = 255 (wildcard in process entries)
- NOT_CREATED = 252, WORN = 253, CARRIED = 254
- Max process nesting = 10
- Max objects = 255, max locations = 252
- Flags: 256 total, 0-63 system, 64-255 user

### Most Important Flags
- Flag 0: darkness
- Flag 1: objects carried count
- Flag 33-36: current verb, noun1, adjective1, adverb
- Flag 37: max carryable objects
- Flag 38: **player location**
- Flag 51: currently referenced object
- Flag 52: max carryable weight
