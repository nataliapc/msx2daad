# DRC - DAAD Reborn Compiler

DRC is the modern DAAD compiler, replacing the original DOS-based DC. It is native to Linux/macOS/Windows (no DOSBOX required).

**Repository**: https://github.com/daad-adventure-writer/DRC

## Architecture

DRC has two stages:
- **DRF** (front-end): FreePascal binary. Parses `.DSF` -> generates `.json` intermediate
- **DRB** (back-end): PHP script. Takes `.json` -> generates `.DDB` binary

## Usage

### Step 1: DRF (Front-end)

```bash
drf <target> [subtarget] <file.dsf> [output.json] [symbols] [options]
```

Example:
```bash
drf MSX2 8_6 mygame.dsf
```

Parameters:
- Target: `zx`, `cpc`, `msx`, `c64`, `amiga`, `pcw`, `st`, `pc`, `msx2`
- Subtarget (MSX2): `<screen>_<fontwidth>` e.g. `8_6` = Screen 8, 6px font
- Output: defaults to `<input>.json`

Options:
- `-verbose`: Show detailed compilation info
- `-no-semantic`: Disable semantic analysis
- `-semantic-warnings`: Show warnings instead of errors for semantic issues
- `-force-normal-messages`: Force all messages to MTX (no inline optimization)

### Step 2: DRB (Back-end)

```bash
php drb.php <target> [subtarget] <language> <file.json> [output.ddb] [options]
```

Example:
```bash
php drb.php MSX2 8_6 EN mygame.json
```

Parameters:
- Language: `EN` (English) or `ES` (Spanish)
- Output: defaults to `<input>.ddb`

Options:
- `-v`: Verbose output
- `-c`: Classic mode (no optimizations, compatible with old DC)
- `-d`: Debug mode (enable DEBUG condact for ZesarUX)
- `-np`: No padding
- `-p`: Force padding
- `-ch`: C64 header
- `-3h`: +3 disk header

**Important**: Rename the output `.DDB` to `DAAD.DDB` before copying to the game disk.

## Compiler Directives

| Directive | Description |
|-----------|-------------|
| `#define SYMBOL [value]` | Define a symbol for use in condact arguments |
| `#ifdef SYMBOL` / `#ifndef SYMBOL` | Conditional compilation |
| `#else` / `#endif` | Conditional compilation blocks |
| `#include file.dsf` | Include another DSF file (no nesting) |
| `#extern "MALUVA"` | Include MALUVA extension binary |
| `#int "file.bin"` | Include interrupt handler binary |
| `#sfx` | Sound effects configuration |
| `#gfx` | Graphics configuration |
| `#incbin "file.bin"` | Include binary data |
| `#hex`, `#dw`, `#db` | Binary data inclusion helpers |
| `#classic` | Disable DRC optimizations |
| `#debug` | Enable DEBUG condact |
| `#echo "text"` | Print message at compile time |

## DRC Auto-defined Symbols

| Symbol | When |
|--------|------|
| Target name (`ZX`, `C64`, `MSX`, `MSX2`, `PC`, `AMIGA`, `ST`, `CPC`, `PCW`) | Always |
| `bit8` | 8-bit targets (ZX, CPC, MSX, C64, PCW) |
| `bit16` | 16-bit targets (PC, Amiga, ST) |
| `COLS` | Screen columns for target |
| `ROWS` | Screen rows for target |
| `LAST_OBJECT` | Highest object number |
| `LAST_LOCATION` | Highest location number |
| `NUM_OBJECTS` | Total number of objects |
| `NUM_LOCATIONS` | Total number of locations |

## DRC Optimizations

When NOT in `#classic` mode:
1. **Entry deduplication**: Synonym process entries that share identical condact code are merged (shared bytecode pointer)
2. **Auto-routing**: When MTX is full, messages can overflow to other tables
3. **Text compression**: MTX and STX are always token-compressed

## Differences from Classic DC (SCE format)

| Feature | Classic DC (SCE) | DRC (DSF) |
|---------|-----------------|-----------|
| Encoding | CP437 | ISO-8859-1 / Windows-1252 |
| Escape char | `\` | `#` |
| Indirection | `[flag]` | `@flag` |
| Inline messages | Not supported | `MESSAGE "text"` / `MES "text"` |
| Symbols | 5 chars max | Unlimited length |
| Strings | Optional quotes | Required quotes (`"..."` or `'...'`) |
| Process entries | No prefix | `>` prefix required |
| Token section | `/TOK` manual | Auto-generated |
| Conditional comp | Not supported | `#ifdef`/`#ifndef` |

## Related Tools

| Tool | Description |
|------|-------------|
| **DRT** | Token optimizer for DAAD texts (by Jose Manuel Ferrer) |
| **EAAD** | Advanced editor for DAAD sources |
| **SC2DAAD** | Image converter for MALUVA XPICTURE format |
| **DAADMAKER** | Creates .TAP/.DSK distribution files |
| **Triz2DAAD** | Converts Trizbort maps to DAAD source |
| **unPAWs** | Disassembles Spectrum PAW games to DSF format |
| **DAAD Ready** | Suite that simplifies the full build process |
