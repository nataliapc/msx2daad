# DDB Binary File Format

The `.DDB` (DAAD DataBase) is the compiled binary loaded by DAAD interpreters. It contains all game data: vocabulary, texts, processes, objects, and connections.

## Header (34 bytes)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x00 | 1 | version | DAAD version: 1 = early games (1989), **2** = all modern games |
| 0x01 | 1 | target | High nibble = machine, low nibble = language |
| 0x02 | 1 | magic | CTL character, always 95 (ASCII `_`) |
| 0x03 | 1 | numObjDsc | Number of object descriptions |
| 0x04 | 1 | numLocDsc | Number of location descriptions |
| 0x05 | 1 | numUsrMsg | Number of user messages (MTX) |
| 0x06 | 1 | numSysMsg | Number of system messages (STX) |
| 0x07 | 1 | numPrc | Number of processes |
| 0x08 | 2 | tokensPos | Offset to token table |
| 0x0A | 2 | prcLstPos | Offset to process list |
| 0x0C | 2 | objLstPos | Offset to object description text lookup |
| 0x0E | 2 | locLstPos | Offset to location description text lookup |
| 0x10 | 2 | usrMsgPos | Offset to user messages lookup |
| 0x12 | 2 | sysMsgPos | Offset to system messages lookup |
| 0x14 | 2 | conLstPos | Offset to connections lookup |
| 0x16 | 2 | vocPos | Offset to vocabulary table |
| 0x18 | 2 | objLocLst | Offset to object initial locations (1 byte/obj) |
| 0x1A | 2 | objNamePos | Offset to object noun+adjective IDs (2 bytes/obj) |
| 0x1C | 2 | objAttrPos | Offset to object weight/flags (1 byte/obj) |
| 0x1E | 2 | objExtrPos | Offset to object extended attributes (2 bytes/obj) |
| 0x20 | 2 | fileLength | Total DDB file length |

All 16-bit values are **little-endian**. Offsets are relative to the start of the DDB file.

### Target Machine Codes (high nibble of `target`)

| Code | Machine |
|------|---------|
| 0 | PC (DOS) |
| 1 | ZX Spectrum |
| 2 | Commodore 64 |
| 3 | Amstrad CPC |
| 4 | MSX |
| 5 | Atari ST |
| 6 | Amiga |
| 7 | Amstrad PCW |
| 15 | MSX2 |

### Language Codes (low nibble of `target`)

| Code | Language |
|------|----------|
| 0 | English |
| 1 | Spanish |

---

## Data Sections

### Vocabulary (at `vocPos`)

Array of 7-byte entries, terminated by first byte = 0:

| Bytes | Content |
|-------|---------|
| 0-4 | Word: first 5 characters, encoded as `255 - char`, padded with encoded spaces |
| 5 | Word ID (0-254) |
| 6 | Word type: 0=verb, 1=adverb, 2=noun, 3=adjective, 4=preposition, 5=conjunction, 6=pronoun |

### Text Sections (STX, MTX, OTX, LTX)

Each section is accessed via a **lookup list** of 16-bit offsets (one per message). Each offset points to a compressed text string within the DDB.

### Connections (at `conLstPos`)

Lookup list of 16-bit offsets, one per location. Each location's connections are pairs of `(verb_id, destination_location)` terminated by `0xFF`.

### Process List (at `prcLstPos`)

Array of `numPrc` 16-bit offsets. Each points to a process table.

### Process Table Entry (4 bytes)

| Byte | Content |
|------|---------|
| 0 | Verb to match (255 = wildcard `_`) |
| 1 | Noun to match (255 = wildcard `_`) |
| 2-3 | 16-bit offset to condact bytecode |

Process tables are terminated by verb byte = 0x00.

### Condact Bytecode

Each condact is encoded as:
- **Byte 0**: bit 7 = indirection flag; bits 0-6 = condact number (0-127)
- **Byte 1**: first argument (if condact takes 1+ args)
- **Byte 2**: second argument (if condact takes 2 args)
- Entry terminator: `0xFF`

### Object Data (spread across 4 tables)

| Table | Size/obj | Content |
|-------|----------|---------|
| `objLocLst` | 1 byte | Initial location (252=not_created, 253=worn, 254=carried) |
| `objNamePos` | 2 bytes | Noun ID + Adjective ID |
| `objAttrPos` | 1 byte | bits 0-5: weight (0-63), bit 6: is_container, bit 7: is_wearable |
| `objExtrPos` | 2 bytes | 16 user-defined attribute bits |

---

## Text Compression (Token System)

All DDB text uses the same encoding scheme.

### Character Encoding

Each byte is stored as `255 - character`. To decode: `c = 255 - byte`.

### Token References

After decoding, if bit 7 of the decoded character is set (`c >= 128`), it references token number `c & 0x7F` (0-127) in the token table.

### Token Table (at `tokensPos`)

A sequence of variable-length token strings. Each token's **last character** has bit 7 set as a terminator. To find token N, skip N tokens by counting terminator characters. The actual character is `byte & 0x7F`.

### String Terminator

A decoded character of `0x0A` (newline) terminates a text string. The encoded terminator byte is `0xF5` (255 - 10).

### Special Inline Characters

| Decoded | Escape | Meaning |
|---------|--------|---------|
| `_` | `_` | Print current object name (lowercase article) |
| `@` | `@` | Print current object name (uppercase article) |
| `\r` (13) | `#n` | Newline / carriage return |
| `\x0B` (11) | `#b` | Clear screen |
| `\x0C` (12) | `#k` | Wait for keypress |
| `\x0E` (14) | `#g` | Enable graphical charset (128-255) |
| `\x0F` (15) | `#t` | Enable text charset (0-127) |

### Extended Characters (0x10-0x1F)

Used for accented letters and symbols (Spanish/European):

```
0x10: ª   0x11: !   0x12: ?   0x13: <<  0x14: >>
0x15: a   0x16: e   0x17: i   0x18: o   0x19: u
0x1A: n   0x1B: N   0x1C: c   0x1D: C   0x1E: u   0x1F: U
```

### Article Modification

When `_` or `@` appears in text, the currently referenced object's name (flag 51) is printed with article modification:
- **Spanish**: `_` converts "Un X" -> "el X", "Una X" -> "la X"; `@` -> "El X" / "La X"
- **English**: `_`/`@` strip the article ("A X" -> "X", "An X" -> "X")

---

## Save File Format

A save file contains:
1. All 256 flags (256 bytes)
2. All object structs (numObjDsc * 6 bytes)

RAM save buffer (512 bytes) stores:
1. All 256 flags (256 bytes)
2. All object locations (up to 256 bytes)
