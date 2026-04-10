# DSF Source File Format

The `.DSF` (DAAD Source File) is the text-based source format for DAAD adventures. It is compiled by the DRC compiler into a `.DDB` binary database.

## Encoding

DSF files must be encoded in **Windows-1252** or **ISO-8859-1**. UTF-8 is NOT supported.

## Editors

- **EAAD**: Text editor specifically created for editing DAAD adventures
- **VS Code/Codium**: With the `daad-paws-sce` plugin for syntax highlighting

## File Sections

### /CTL - Control Section

Defines the null/wildcard character (always `_`):
```
/CTL
_ 95
```

### /VOC - Vocabulary

Word definitions with format: `<word> <id> <type>`

```
NORTH  2  noun
SOUTH  3  noun
GET   10  verb
TABLE 20  noun
RED   10  adjective
```

**Rules:**
- Only the first **5 characters** matter; longer words are truncated
- Word types: `verb`, `adverb`, `noun`, `adjective`, `preposition`, `conjunction`, `pronoun`
- Words with same ID and type are **synonyms**
- Verb IDs 0-13: movement directions (used by MOVE condact / connection tables)
- Noun IDs 0-19: can also function as verbs
- Noun IDs 0-49: proper nouns (cannot be replaced by "IT")
- Noun ID 20: conventionally used for ALL/EVERYTHING (for DOALL)
- Word ID 255 (`_`): NULLWORD / wildcard in process entries

### /STX - System Messages

Predefined messages used internally by the interpreter. Format: `/<number> "text"`

```
/0 "It's too dark to see anything."
/1 "I can also see:#n"
```

Escape codes (DRC uses `#` instead of `\`):
- `#n` = newline
- `#k` = wait for keypress
- `#b` = clear screen
- `#g` = enable graphical charset
- `#t` = enable text charset

### /MTX - User Messages

Custom game messages. Format: `/<number> "text"`

```
/0 "You jump the crevice and now you are at the other side."
```

Messages can be shown with `MES <number>` or `MESSAGE <number>` condacts.

DRC extension: inline messages in process sections:
```
MESSAGE "You jump the crevice and..."
MES "Text without carriage return."
```
The compiler auto-creates /MTX entries and replaces inline text with the assigned number.

### /OTX - Object Descriptions

Object name/description texts. Index must match /OBJ section.

```
/0 "a big and red key"
/1 "an old torch"
```

Convention: start descriptions with the article ("a", "an", "the" / "un", "una") for proper article modification in messages using `_` or `@`.

### /LTX - Location Descriptions

Location text descriptions. Format: `/<number> "text"`

```
/0 "You are in a dark cave. Water drips from the ceiling."
/1 "You are standing in a clearing surrounded by ancient trees."
```

### /CON - Connections

Per-location direction-to-destination pairs:

```
/0
NORTH 1
EAST  2
/1
SOUTH 0
/2
WEST  0
```

Each location must be defined even if it has no connections.

### /OBJ - Object Definitions

```
;obj  starts  weight    c w  5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0    noun   adjective
/0    CARRIED   1       _ _  _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _    TORCH  _
/1    NOT_CREATED 2     Y _  _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _    KEY    RED
```

- Starting locations: `CARRIED`, `WORN`, `NOT_CREATED`, or a number
- `c`/`w`: `Y` = yes, `_` = no (carriable/wearable)
- 16 user attribute bits: `Y` or `_`

### /PRO - Process Tables

Process definitions with verb/noun entries and condact sequences:

```
/PRO 5
> GET    TORCH    AUTOG
                  DONE

> EAT    APPLE    AT      1
                  PRESENT  objAPPLE
                  MESSAGE "It tastes good!"
                  DESTROY  objAPPLE
                  DONE

> _      _        ; wildcard entry - matches anything
                  NOTDONE
```

Entry format:
- `> VERB NOUN` header (use `_` for wildcard)
- Indented condacts below the header
- `;` for comments

### /END

Marks the end of the DSF file. Required.

---

## Standard Process Architecture

| Process | Purpose |
|---------|---------|
| PRO 0 | Main location loop: darkness check, load picture, describe location, call PRO 3, call PRO 1 |
| PRO 1 | Game loop: call PRO 4, parse input, increment turns, call PRO 5, try MOVE, print error |
| PRO 2 | Timeout/invalid input: prints "I didn't understand" or handles timeout |
| PRO 3 | List objects at current location (if not dark) |
| PRO 4 | "Each turn" process (author's per-turn logic) |
| PRO 5 | Command decoder: INVENTORY, GET ALL, DROP ALL, QUIT, SAVE, LOAD, LOOK, etc. |
| PRO 6 | Initialization: windows, flags, reset objects, set ABILITY defaults |

### Process Execution Flow

```
Game Start -> PRO 6 (init) -> PRO 0 (main loop)
                                |
                                +-> Show location (picture + description)
                                +-> PRO 3 (list objects)
                                +-> PRO 1 (game loop)
                                     |
                                     +-> PRO 4 (each turn)
                                     +-> PARSE 0 (get input)
                                     +-> Increment turns (flags 31-32)
                                     +-> PRO 5 (command decoder)
                                     +-> Try MOVE (connections)
                                     +-> Error message if nothing matched
                                     +-> Loop via REDO
```

---

## DRC `#define` Symbols

DRC supports `#define` for symbolic constants:

```
#define objTORCH  0
#define objKEY    1
#define locCAVE   0
```

These can be used anywhere an ID number is expected:
```
> GET    TORCH    AT locCAVE
                  PRESENT objTORCH
```

DRC auto-defines target symbols: `C64`, `ZX`, `MSX`, `MSX2`, `PC`, `AMIGA`, `ST`, `CPC`, `PCW`, `bit8`, `bit16`, `COLS`, `ROWS`, `LAST_OBJECT`, `LAST_LOCATION`, `NUM_OBJECTS`, `NUM_LOCATIONS`.
