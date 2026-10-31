# AGENTS.md — MSX2DAAD

> DAAD text-adventure interpreter for MSX2/MSX2+, written in C and Z80 assembly.
> Compiled with SDCC 4.5.0 running inside Docker. Target: MSX-DOS `.com` executables.

## Self-maintenance rule

Any change to the repository that affects build commands, project structure, conventions, or toolchain **must** be reflected in this file — either as an update to existing content or as a new entry. Do not let `AGENTS.md` drift from reality.

---

## Build & toolchain

- **Compiler**: SDCC 4.5.0 (`sdasz80`, `sdar`, `sdcc`) — runs inside Docker via `docker run`.
- **Docker image**: `nataliapc/sdcc:4.5.0`. The Makefile defines `DOCKER_RUN` which mounts the **project root** (`msx2_daad/`) at `/src` with working directory set to the relevant subdirectory. Change `SDCC_VER` in the Makefile to use other SDCC versions.
- **Calling convention**: SDCC 4.5.0 uses `__sdcccall(1)` by default — 8-bit return in `A`, 16-bit in `DE`, `uint32_t` in `E:D:L:H`. All `__naked` functions in `src/libs/` have been rewritten for this convention (see PRP007).
- **Post-link**: `hex2bin` converts `.ihx` → `.com`; binary is copied to `dsk/`.
- **Host tools**: `gcc` (for `bin/testdaad`), `php` (for `bin/precomp.php`, `bin/imgwizard.php`), `python3` with Pillow and the `zx0` compressor (for `bin/img9990.py`, GFX9000 images).
- Docker must be running before any `make` target. No `docker-compose` is used — the Makefile calls `docker run` directly.
- **Native SDCC 4.6.0** (e.g. Homebrew) also builds the program and all unit tests by overriding the tools: `make AS=sdasz80 AR=sdar CC=sdcc HEX2BIN=<hex2bin>`. Two things keep it working:
    - Prototypes in `include/daad_platform_api.h` must not be declared `inline`. SDCC 4.6.0 enforces C99 inline semantics (error 328) in every translation unit that includes the header; the platform `.c` files keep `inline` on their definitions.
    - SDCC 4.6.0 (z80) hits a fatal internal error (`gen.c` line 1334) when `x > 0`, with `x` an extern `uint8_t`, is passed as a `bool` argument followed by a pointer argument (e.g. `ASSERT(counter > 0, "msg")`). Write `x != 0` instead.

### Key commands

```bash
make clean all          # Full rebuild of msx2daad.com (default: test/debug, Spanish, MSX-DOS1, Screen 8)
make test               # Launch openMSX emulator with dsk/ as disk A
make precomp            # Analyze dsk/DAAD.DDB → regenerate include/daad_defines.h (strips unused condacts)
make package            # Build all 16 release variants (EN/ES × SC5/6/7/8/10/12 + transcript + GFX9000)
make EN_G9K             # Build one variant (here the GFX9000 one in English) into package/
make disk               # Generate game.dsk and game.rom from dsk/ contents
make bin/testdaad       # Compile the host-side integration test tool (GCC)
```

### Unit tests

```bash
cd unitTests && make all    # Compile all 11 test .com binaries (via Docker)
cd unitTests && make test   # Run tests in openMSX (machine: turbor), then print summary
cd unitTests && make clean  # Remove obj/, dsk/*.com, dsk/tests.txt
```

`unitTests/Makefile` uses Docker (same `DOCKER_RUN` pattern as the main Makefile). No local SDCC install required.

---

## Compile-time configuration

Almost everything is selected via `-D` flags in `CXXFLAGS`. The Makefile default is:

```
-DLANG_ES -DMSX2 -DDAADV3 \
-DCOMPRESSOR_RAW -DCOMPRESSOR_RLE -DCOMPRESSOR_PLETTER -DCOMPRESSOR_ZX0 \
-D_TEST -D_TRANSCRIPT -D_DEBUG -D_VERBOSE -D_VERBOSE2
```

| Flag | Purpose |
|------|---------|
| `-DMSX2` / `-DCPM` / `-DPC_TXT` | Target platform (mutually exclusive) |
| `-DMSXDOS1` | MSX-DOS version (MSXDOS2 removed — FCB/DOS1 API only) |
| `-DLANG_EN` / `-DLANG_ES` | Language (English or Spanish) |
| `-DSCREEN=5\|6\|7\|8\|10\|12\|16` | MSX screen mode (default 8). `16` is the GFX9000 (V9990) B1 BD16 mode: defines `V9990`, undefines `COMPRESSOR_PLETTER` |
| `-DFONTWIDTH=6\|8` | Font width in pixels |
| `-DDAADV3` | DDB V3 format support |
| `-DCOMPRESSOR_RAW` / `-DCOMPRESSOR_RLE` / `-DCOMPRESSOR_PLETTER` / `-DCOMPRESSOR_ZX0` | Enable each V9938CmdData decompressor in the gfxPictureShow dispatch (PRP025). Defining a subset trims the unused decoders from the .com. |
| `-DTEST` | Enables error details + openMSX debug device output |
| `-DDEBUG` | Debug output |
| `-DVERBOSE` / `-DVERBOSE2` | Condact execution tracing |
| `-DTRANSCRIPT` | Write game text to `TRANSCR.TXT` |
| `-DUNIT_TESTS` | Unit test build (used in `unitTests/`) |
| `-DVERSION=X.Y.Z` | Version string for release builds |

Override at invocation: `make CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_EN -DSCREEN=5" clean all`

The Makefile defines `COMPFLAGS` and `PKGFLAGS` macros that share the compressor/font/version flags between the default build and the 16 `package` targets — change them in one place.

---

## Project structure

```
src/msx2daad.c              # Entry point: main()
src/daad_condacts.c         # All 128 DAAD condacts (dispatch table + implementations)
src/daad_global_vars.c      # Global state definitions
src/daad_platform_msx2.c    # MSX2 platform layer (GFX, SFX, filesystem, keyboard)
src/daad_platform_pctxt.c   # PC text-mode platform (host debugging)
src/daad_platform_cpm.c     # CP/M platform
src/heap.c                  # Linear bump allocator (custom malloc/free for Z80)
src/crt0msx_msxdos_advanced.s  # CRT0 startup (Z80 asm, code at 0x0180)
src/daad/                   # Engine split into testable translation units
    daad_condacts.c         #   (symlink/copy, same as src/daad_condacts.c)
    daad_getObjectWeight.c  #   getObjectWeight() — pure logic, no platform calls
    daad_getToken.c         #   getToken() — DDB token decompression
    daad_global_vars.c      #   Global state definitions
    daad_init.c             #   initDAAD(), initFlags(), initObjects()
    daad_mainLoop.c         #   mainLoop() — game loop entry point
    daad_msg.c              #   Message extraction and printing
    daad_objects.c          #   getObjectId(), referencedObject()
    daad_parser_sentences.c #   parser(), populateLogicalSentence(), etc.
    daad_print.c            #   printChar(), printMsg(), printOutMsg(), etc.
    daad_prompt.c           #   prompt() — user input handler
    daad_transcript.c       #   transcript_flush(), transcript_char()
    daad_utils.c            #   errorCode(), waitForTimeout()
src/libs/                   # Low-level libraries → compiled into dos.lib, vdp.lib, gfx9000.lib, utils.lib
    dos_*.c / dos_*.s       #   MSX-DOS file operations (FCB/DOS1 API only)
    vdp_*.c / vdp_*.s       #   VDP (TMS9938) BitBlt, palette, screen ops
    g9k_*.c / g9k_*.s       #   GFX9000 (V9990): detection, B1 BD16 set-up, command engine, picture buffer, ZX0
    utils_*.c / utils_*.s   #   Decompression (Pletter/RLE/ZX7), string ops, mouse

include/                    # Headers
    daad.h                  #   Core types: DDB_Header, PROCstack, Window, Object, flags[256]
    daad_platform_api.h     #   Platform selector (#ifdef → includes correct platform header)
    daad_condacts.h         #   Condact function declarations + NUM_PROCS
    daad_defines.h          #   Auto-generated by precomp.php (condact exclusion #defines)
    dos.h, vdp.h, heap.h   #   Library APIs
    gfx9000.h               #   GFX9000 (V9990) ports, registers, command packet, VRAM layout, g9k_* API
    msx_const.h             #   MSX BIOS constants (~1700 lines)
    sdcc_compat.h           #   SDCC 4.5.0 compatibility helpers

unitTests/                  # Unit test suite — 11 .com binaries, 323 tests
    Makefile                #   Dockerized build; produces .com → dsk/
    dsk/                    #   Test disk image: AUTOEXEC.BAT + .com files + tests.txt output
    src/
        condacts_stubs.h/c  #   Shared stubs for condact tests (globals + platform mocks)
        daad_stubs.h/c      #   Shared stubs for daad engine tests
        tests_dos.c         #   23 tests for dos.lib
        tests_condacts1.c   #   36 tests: location + object location condacts
        tests_condacts2.c   #   61 tests: flag/parser/misc conditions
        tests_condacts3.c   #   62 tests: object manipulation condacts
        tests_condacts4.c   #   44 tests: object data + flag ops + movement
        tests_condacts5.c   #   22 tests: window + display condacts
        tests_condacts6.c   #   17 tests: I/O + control flow condacts
        tests_daad_getObjectWeight.c  # 5 tests: weight logic incl. containers
        tests_daad_objects.c          # 8 tests: getObjectId + referencedObject
        tests_daad_getToken.c         # 3 tests: DDB token decompression
        tests_daad_init.c             # 6 tests: initObjects + initFlags

docs/                       # DAAD official documentation (markdown)
    DAAD_Manual_1991.md     #   Original manual — definitive condact spec (DC compiler, [] indirection)
    DAAD_Manual_2018.md     #   2018 revision (tools/distribution focus)
    DAAD_Ready_Documentation_V2.md  # DAAD Ready v2 — condact reference for DRC compiler (@ indirection)
    Flags_for_Quill_PAWS_SWAN_and_DAAD.md  # System flag reference

bin/                        # Tools: imgwizard.php, img9990.py (GFX9000 .IMG), precomp.php, testdaad.c, dsktool, dsk2rom
dsk/                        # Runtime disk image contents (DAAD.DDB, fonts, images, MSX-DOS)
emulation/                  # openMSX Tcl scripts (boot.tcl, info_daad.tcl)
plan/                       # PRP planning documents (PRP001–PRP026)
```

---

## Architecture notes

- **Platform abstraction**: `include/daad_platform_api.h` selects the platform header based on `#ifdef MSX2|CPM|PC_TXT`. Each platform implements the same API (`gfx*`, `sfx*`, filesystem, keyboard). When adding platform-dependent code, implement it in all three platform files.
- **Condact dispatch**: `daad_condacts.c` has a `condactList[]` function pointer array. Each condact is a `void` function (e.g., `do_AT()`, `do_GET()`). Condacts can be excluded at compile time via `#ifndef` guards generated by `precomp.php` into `include/daad_defines.h`.
- **Memory model**: Code starts at `0x0180`, data at `0`. Custom CRT0 (no standard C library). Custom linear bump allocator in `heap.c` — no realloc, no free list.
- **Image format**: Custom `.IMx` chunked format. Created by `bin/imgwizard.php`. Two format generations:
    - **V1 (legacy `c`/`cl`/`s` commands, DEPRECATED)**: chunks PALETTE/RAW/RLE/PLETTER + control chunks (RESET/CLS/SKIP/PAUSE).
    - **V2 (`cx`/`cxl` commands, current)**: INFO + V9938Cmd + V9938CmdData chunks streamed directly to V9938 HMMC/LMMC via port #9B (PRP022). Compressors per V9938CmdData chunk: RAW (0), RLE (1), PLETTER (2), ZX0 (3). Decompressors stream into a VRAM scratch zone at `0x1D400` (`VRAM_DECOMP_SCRATCH`, capped at 11264 bytes uncomp per chunk).
    - **Transparency**: `cx[l] --transparent-color=N` produces a 2-pass LMMC|AND + LMMC|OR sequence (mask + image buffers) for SC5/6/7/8 (PRP024). SC10 (`.SCA`) supported via the YJK+YAE bit-A flag: paletted-visible pixels use `mask=0x07, image=byte&0xF8` to preserve dest's bits 2-0 (chroma sharing of the 4-byte YJK group); YJK source pixels are auto-preserved (PRP026). SC12 (pure YJK) remains unsupported.
    - **GFX9000 (`SCREEN=16`, `.IMG`, screen type `'G'`)**: created by `bin/img9990.py` (`image`, `font`, `list`). INFO (pixel type 5 = BD16, palette type 3 = GRB555, chipset 4 = V9990) + optional FIXEDIMG + V9990Cmd (type 22: 21-byte packets for R#32-R#52, one LMMC over the rectangle) + V9990CmdData (type 23: same layout as V9938CmdData; RAW, RLE or ZX0, at most 32768 uncompressed bytes per chunk). The interpreter does not send the LMMC: the data chunks are decoded through P#0 into a linear VRAM buffer at `0x60000` (`G9K_VRAM_SCRATCH`) and the rectangle is drawn with one BMXL. No Pletter decoder for the V9990.
        - **Clipping**: a picture without FIXEDIMG is a floating DAAD picture and is clipped to the window current at DISPLAY (right and bottom; a window reaching the last column or line extends to the screen edge, and CLS of such a window clears up to the edge too, so nothing drawn there is left behind). Lower is one BMXL with a smaller NY; narrower is one BMXL per line, as the buffer keeps the full width. FONT.IMG is not clipped (`g9kLoadingFont`).
        - **FIXEDIMG with window**: the extra header may be 8 bytes, `{ offsetX, offsetY, winX, winY, winW, winH }` (window in characters); the current window then becomes that rectangle, as a fixed DAAD picture does (`img9990.py image --fixed X,Y --window C,R,W,H`). The 4 byte form is unchanged.
        - **Palette**: an optional chunk type 1 in the legacy layout `{ 1, 32, 32 }` + 16 GRB555 words (`--palette`) loads the colour table used by INK, PAPER and the rest, as the palette of a fixed DAAD picture does. The pixels themselves are direct colour.
        - **Fonts**: `img9990.py font --gchars PNG` adds the graphic charset (256x32, glyphs from code 16) as a second LMMC 264 lines below the text one, where `gfxPutChPixels()` reads it after `\g` (VRAM line 476).
- **GFX9000 (V9990) build**: `SCREEN=16` keeps the V9938 code paths and VRAM layout (font at Y=212, TEMP at 244, back screen at 256, graphic charset at 476) in an image space of 256x1024 pixels, 2 bytes each (GRB555, bit 15 = YS). `bitBlt()`, `waitVDPready()` and `setVPage()` are macros to V9990 versions in `daad_platform_msx2.c`: colours are EGA indexes looked up in a GRB555 table (GFX 9/10 change and read it), V9938 command and logical codes are translated to V9990 LMMV/LMMM. Every command writes all 16 bits (write mask 0xFFFF), because transparency compares the whole word with 0 and VRAM holds garbage in bit 15 after power-on; only NOT uses mask 0x7FFF. The CRT0 does not clear uninitialised data: initialise statics explicitly (see `g9kPicturePending`). The MSX VDP keeps the MSX-DOS text mode; `checkPlatformSystem()` dies without a GFX9000. `flags[fScMode]` is `32|4|2`.
- **`#ifdef` heavy**: Nearly every source file uses conditional compilation for platform, screen mode, language, and debug features. Be careful with changes — test the specific variant you're modifying.
- **MSX-DOS1 only**: `MSXDOS2` support was removed in PRP007. All filesystem calls use FCB-based DOS1 API.

---

## Code style

- **Indentation**: Tabs, 4-space width (`.editorconfig`).
- **Line endings**: LF.
- **Charset**: UTF-8 for `.c`, `.h`, `.s`, `.asm`.
- **SDCC-specific C**: Uses `__asm`/`__endasm` inline assembly, `volatile __at()` for MMIO. This is not standard C — do not refactor to remove SDCC-specific constructs.
- **Naming**: Platform files: `daad_platform_<name>.c/.h`. Library files: `<category>_<function>.c/.s` (e.g., `dos_open.c`, `vdp_clear.s`).

---

## Testing

### Unit tests (primary)

**11 binaries, 323 tests, run on openMSX (turbor machine):**

| Binary | Tests | Coverage |
|--------|-------|----------|
| `dos.com` | 23 | dos.lib file I/O (MSX-DOS1 FCB API) |
| `condact1–6.com` | 242 | All 128 condacts (292 OK, 31 TODO, 0 FAIL) |
| `objwght.com` | 5 | `getObjectWeight()` incl. container + magic bag |
| `daadobjs.com` | 8 | `getObjectId()`, `referencedObject()` |
| `gettoken.com` | 3 | `getToken()` DDB token decompression |
| `initobj.com` | 6 | `initObjects()`, `initFlags()` |

Results are written to `unitTests/dsk/tests.txt`. The Makefile prints a summary after openMSX exits:
```
OK: 292 / 323  |  FAIL: 0 / 323  |  TODO: 31 / 323
```

**Stubs architecture**: Two independent stubs pairs:
- `condacts_stubs.h/c` — for condact tests; links real `daad_objects.rel` + `daad_getObjectWeight.rel`
- `daad_stubs.h/c` — for engine function tests; no `daad_condacts.rel` (avoids circular deps)

All tests validated against `docs/DAAD_Manual_1991.md`, `docs/Flags_for_Quill_PAWS_SWAN_and_DAAD.md`, `docs/DAAD_Ready_Documentation_V2.md`, and `wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md`. Each test includes a reference to the manual line that justifies the expected behaviour.

### Integration tests

`bin/testdaad` (host C program) communicates with openMSX via pipes using script files with `>` (send) and `<` (expect) directives. Requires `-DTEST` build.

### Manual testing

`make test` boots openMSX with `dsk/` as disk A. The emulator runs `AUTOEXEC.BAT` which launches `msx2daad`.

There is no CI pipeline. All testing is local.

---

## Operational gotchas

- `hex2bin` must be installed on the host (not in Docker).
- `openMSX` must be installed for `make test` and unit test execution.
- `precomp.php` requires `php` CLI on the host.
- The `dsk/` directory is the emulated disk — changes to runtime files (DDB, fonts, images) go there.
- `include/daad_defines.h` is auto-generated — do not edit manually; run `make precomp` after changing `dsk/DAAD.DDB`.
- Release builds (`make package`) produce 14 `.com` variants in `package/` — this takes a long time.
- The current version is `1.5.1` (set in `Makefile` line 23).
- `unitTests/dsk/tests.txt` uses `\n\r` line endings (MSX output). Grep patterns must NOT use `^` anchor for lines after the first — use bare patterns like `### TODO:` not `^### TODO:`.

---

## Known bugs / limitations

- **Convertible nouns**: with no verb, a noun up to `LAST_CONVERTIBLE_NOUN` (19) is copied to the verb and stays the noun, for PARSE 0 only (PCDAAD `parser.pas`). It used to be moved, which left Noun1 empty and broke entries such as `_ I`.
- **DOALL with no objects** ends the table as DONE/NOTDONE do and leaves `checkEntry` alone, so a calling process goes on after its PROCESS (it used to abort the caller's entry).
- **Line wrap**: `printChar()` wraps as soon as the last column is written, so text ending exactly at the right edge of a window one line high scrolls it away. Games that right-align a status line must stop one column short.

- **`AUTOT` container search** was broken until PRP008: `getObjectId(LOC_CONTAINER=256)` always failed because `256 < numObjDsc` (uint8) is always false. Fixed in `src/daad/daad_objects.c` — now checks `objects[i].location < numObjDsc` instead.
- **`PAUSE`, `END`, `EXIT 0`** are not unit-testable: PAUSE loops on `getTime()` (always 0 in stubs), END/EXIT 0 call `die()` which terminates the process.
- **Spanish enclitic pronouns** (PRP016): implemented. `parser()` detects `-LO/-LA/-LOS/-LAS` suffixes on matched verbs and injects a `[SYNTH_PRONOUN_ID, PRONOUN]` token (`SYNTH_PRONOUN_ID=1`, defined in `include/daad.h`). `populateLogicalSentence` processes it via the PRONOUN branch (PRP015), filling `fNoun1`/`fAdject1` from flags 46/47. Respects `F53_NOPRONOUN` (V3). **Limitation**: verbos ≤ 4 chars require vocabulary synonyms per DAAD 1991 manual §4.4.1 — the DAAD 5-char truncation must match a vocab entry before enclitic detection can fire.
- **`DOALL`** requires a real process table to test meaningfully.
- **`MOVE`** requires Verbs/Connections DDB tables to be mocked.

---

## DAAD knowledge sources

When working on interpreter logic, condact behaviour, DDB format, flags, or adventure authoring, consult these sources in order of authority:

### Primary spec (definitive)
| File | Content |
|------|---------|
| `docs/DAAD_Manual_1991.md` | Original DAAD manual — authoritative spec for all 128 condacts, system messages, flag semantics, object model, process tables. Uses DC compiler syntax: indirection with `[param]` |
| `docs/Flags_for_Quill_PAWS_SWAN_and_DAAD.md` | Complete flag reference (0–255): name, initial value, purpose for every system flag |
| `docs/DAAD_Manual_2018.md` | 2018 revision by Tim Gilberts & Stefan Vogt — tool workflows, platform notes, known issues; condact spec unchanged from 1991 |

### DRC compiler documentation
| File | Content |
|------|---------|
| `docs/DAAD_Ready_Documentation_V2.md` | DAAD Ready v2 — condact and system reference for the **DRC compiler** (the compiler used by this project). Uses `@param` indirection syntax. Mostly aligned with the 1991 manual, but has known discrepancies — see `DAAD_DISCREPANCIAS.md` for the full analysis. Key differences: `PUTIN`/`TAKEOUT` use `locno+` instead of `locno.`; `AUTOP`/`AUTOT` include the `locno` argument (unlike the wiki). Prefer `docs/DAAD_Manual_1991.md` for authoritative condact semantics; prefer this file for DRC-specific syntax and the `@` indirection form |

### MSX2-specific wiki
| File | Content |
|------|---------|
| `wiki/MSX2DAAD-Wiki:-DAAD-Condacts:-a-quick-reference.md` | Quick reference for all condacts with MSX2 notes; use to cross-check 1991 manual |
| `wiki/MSX2DAAD-Wiki:-Understanding-DSF-file-format.md` | DDB binary format: header layout, section offsets, object attributes, token encoding |
| `wiki/MSX2DAAD-Wiki:-About-DAAD.md` | Project overview and architecture |
| `wiki/MSX2DAAD-Wiki:-Using-DRC.md` | DRC compiler usage |
| `wiki/MSX2DAAD-Wiki:-MSX2DAAD features-and-limits.md` | MSX2-specific extensions and limits |

### Repositories

| Name | Content |
|------|---------|
| [PCDAAD](https://github.com/Utodev/PCDAAD) | PC/MSDOS engine interpreter (VGA 256 colours), reference implementation for V3 DDB format, and GFX/SFX behaviour |

### Agent skill
- **`daad-system`** (`.agents/skills/daad-system/SKILL.md`): Structured technical reference — DDB binary format, all 128 condacts, system flags, object model, DSF source format, DRC compiler, MALUVA extensions, engine architecture. Load this skill for any task involving interpreter logic, condact behaviour, DDB parsing, or adventure source code.
