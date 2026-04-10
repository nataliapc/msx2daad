# MALUVA Extensions

MALUVA is a DAAD extension that provides additional condacts for features not in the original DAAD system. It is loaded via `#extern "MALUVA"` in the DSF source.

**Repository**: https://github.com/daad-adventure-writer/MALUVA

## Virtual Condacts (DRC)

DRC compiles these as platform-appropriate EXTERN calls.

### XPICTURE picno

Load and display a raster graphic from disk.

```
XPICTURE 5          ; Load picture 5
XPICTURE @38        ; Load picture for current location (flag 38)
```

Equivalent to:
```
PICTURE picno
DISPLAY 0
```

Image files are named `NNN.ext` where ext is platform-specific:
- `.ZXS` (ZX Spectrum), `.NXT` (ZX Next), `.MS2` (MSX), `.CPC` (Amstrad CPC), `.64` (C64, no dot)

Images are created with the **SC2DAAD** utility.

### XSAVE opt

Save game state to disk. Replaces SAVE on platforms without native disk save.

```
XSAVE 0
```

### XLOAD opt

Load game state from disk. Replaces LOAD on platforms without native disk load.

```
XLOAD 0
```

### XMESSAGE "text" / XMES "text"

Print messages stored in an external disk file (`.XMB`), freeing space in the DDB. XMESSAGE adds a newline, XMES does not.

```
XMESSAGE "This is a long text stored externally on disk."
XMES "No newline at the end."
```

DRC generates `0.XMB` files alongside the `.DDB`. **Rename** `0.XMB` to `TEXTS.XDB` and copy to the game disk.

### XBEEP length tone

Sound on platforms where BEEP may not be natively supported. Duration in 1/50ths of second. Tone 48-238 (even values only, 8 octaves).

```
XBEEP 50 126     ; Quarter note, O4 D#
```

### XPLAY mmlString

Expands to multiple XBEEP condacts to play music using MML (Music Macro Language) notation.

```
XPLAY "T110O5G16F#16G16A16G8O4G4B4O5D8"
```

MML commands:
| Command | Description |
|---------|-------------|
| `A-G` | Notes (C D E F G A B) |
| `#`/`+` | Sharp |
| `-` | Flat |
| `n` (after note) | Length as 1/n of whole note |
| `.` | Dotted note (1.5x duration) |
| `Tn` | Tempo: n quarter notes per minute (32-255, default 120) |
| `On` | Octave (1-8, default 4) |
| `Ln` | Default note length |
| `Rn` | Rest of 1/n whole note length |
| `Nn` | Note by pitch number (0-96, 0=C1, each increment = half tone) |
| `Vn` | Volume (0-15, default 8, not always supported) |
| `<` | Decrease octave |
| `>` | Increase octave |

MML library: https://archeagemmllibrary.com/

### XPART num

For multi-part games sharing a disk. Changes which XMB file set is loaded.

```
XPART 1     ; Switch to part 1 external messages
```

### XSPLITSCR

Split-screen mode: upper half = picture area (96px), lower half = text (104px). Supported on CPC, C64, ZX-Uno.

### XUNDONE

Clear the "done" status flag without exiting the process. Useful for SYNONYM entries that shouldn't count as "done".

## MALUVA Error Reporting

On failure, MALUVA sets **bit 7 of flag 20**. Test with:
```
GT 20 127    ; True if MALUVA reported an error
```

Setting bit 0 of flag 20 (`LET 20 1`) enables "done" flag integration with MALUVA operations.

## Using MALUVA Without DRC

With the classic DC compiler, use EXTERN directly:

| Virtual condact | EXTERN equivalent |
|----------------|-------------------|
| `XPICTURE x` | `EXTERN x 0` |
| `XSAVE 0` | `EXTERN 0 1` |
| `XLOAD 0` | `EXTERN 0 2` |
| `XMES` | `EXTERN 0 3` (with setup) |
| `XPART x` | `EXTERN x 4` |
| `XBEEP l t` | `EXTERN l 5` + SFX for tone |
| `XUNDONE` | `EXTERN 0 7` |
