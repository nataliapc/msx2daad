# DAAD Engine Architecture

## Game Loop

### Startup Sequence

```
1. Load DDB binary into memory
2. Validate header (version must be 2)
3. Convert header offsets to absolute pointers
4. Initialize token table (skip first token entry)
5. Allocate runtime structures (objects, ramsave, tmpMsg)
6. Initialize flags, windows
7. Push Process 0 onto process stack
8. Enter main engine loop
```

### Engine Loop (processPROC)

The engine is a single loop:

1. Execute condacts in current entry while conditions pass
2. Advance to next matching entry (verb+noun from flags 33, 34)
3. When process table ends (verb byte = 0), pop back to calling process
4. When stack fully unwound, infinite loop continues (game never returns from Process 0)

### Condition vs Action

- **Conditions**: Test something. If false, skip to the next entry in the table.
- **Actions**: Do something. Continue to the next condact in the same entry.

An entry is a sequence of condacts. If any condition fails, the remaining condacts are skipped and the engine moves to the next entry that matches the current verb+noun.

---

## Process Call Stack

Processes nest via the PROCESS condact up to **10 levels deep**. Each level maintains:
- Process number
- Current entry pointer
- Current condact pointer
- DOALL state (if active)
- Whether conditions are passing (continueEntry flag)

---

## Parser / Logical Sentence System

### Input Processing

1. Player types a sentence
2. Parser splits into words, looks each up in vocabulary
3. Words stored as `(id, type)` pairs in a buffer
4. Quoted strings (`"..."`) stored separately for `PARSE 1`

### Logical Sentence Extraction

The parser extracts one logical sentence and sets system flags:

| Flag | Content |
|------|---------|
| 33 (fVerb) | Verb ID |
| 34 (fNoun1) | First noun ID |
| 35 (fAdject1) | First adjective |
| 36 (fAdverb) | Adverb |
| 43 (fPrep) | Preposition |
| 44 (fNoun2) | Second noun ID |
| 45 (fAdject2) | Second adjective |
| 46-47 | Pronoun noun/adjective |

Unset slots are filled with NULLWORD (255).

### Compound Sentences

Players can type compound sentences with conjunctions: `GET SWORD AND KILL ORC`. These are split at conjunction words. After processing one sentence, the next is shifted forward.

### Second Object Resolution

When Noun2 is present, the parser immediately resolves it to an object:
- Flag 25: Object 2 number
- Flag 26: Object 2 container flag
- Flag 27: Object 2 location
- Flags 39-40: Object 2 extended attributes

### Noun-as-Verb Rule

If no verb is found but a noun with ID < 20 exists, it functions as the verb. This allows direction words (NORTH, SOUTH = noun IDs 2-13) to work as movement commands without an explicit "GO" verb.

---

## Indirection Mechanism

When the indirection bit (bit 7 of condact byte) is set, the **first argument** is treated as a flag number and dereferenced:

```
LET 10 42         ; flag[10] = 42
LET @10 42        ; flag[flag[10]] = 42 (double dereference)
```

Indirection applies **only to the first argument**. The second argument is always literal (with exceptions in specific condacts like ISAT where 255 means "HERE").

---

## Window System

### Window Structure

Each window has:
- Position: (x, y) on screen
- Size: (width, height)
- Cursor position within window
- Mode flags (bit 0: force upper charset, bit 1: disable "More...")
- Paper (background) and Ink (text) colors
- Last picture drawn (to avoid redundant redraws)

### Number of Windows

DAAD supports **8 windows** (0-7). Each starts at (0,0) with full screen dimensions.

### "More..." Mechanism

When text fills a window (printed lines >= height - 1), system message 32 ("More...") is shown and the system waits for a keypress or timeout. Disabled per-window with `MODE 2`.

---

## ISDONE / ISNDONE Mechanism

Every condact that is an **action** (not a condition) sets an internal "isDone" flag. This flag is inherited by the calling process when a sub-process returns.

- `ISDONE`: True if the last PROCESS call executed at least one action
- `ISNDONE`: True if the last PROCESS call did nothing (or explicitly called NOTDONE)

This enables sub-processes to return a boolean success/fail to the caller.

---

## Darkness System

1. Flag 0 (`fDark`) controls darkness. Non-zero = dark.
2. Object 0 is conventionally a light source.
3. When dark, the location description is replaced by SM0 ("It's too dark to see anything.").
4. If Object 0 is PRESENT (carried, worn, or at location), the player can see despite darkness.

---

## Connection Tables

Each location has a connection list: pairs of `(direction_verb, destination_location)`.

The MOVE condact scans connections for the current location (flag 38), looking for a match on the current verb (flag 33). If the verb matches a direction verb (ID < 14), the player is moved to the destination.

---

## Save / Load

### Disk Save (SAVE/LOAD)

Save file contains:
1. All 256 flags (256 bytes)
2. All object data

On LOAD failure, SM57 ("I/O Error") is printed and the game restarts.

### RAM Save (RAMSAVE/RAMLOAD)

RAM buffer (volatile) stores:
1. All 256 flags
2. All object locations

RAMLOAD parameter specifies the **last flag to restore**, allowing preservation of flags above that number (useful for keeping score across restores).

Practical use: Implement OOPS/UNDO by doing RAMSAVE at the start of each turn.

---

## Error Codes (Runtime)

| Code | Error |
|------|-------|
| 0 | Invalid object number |
| 1 | Illegal assignment to HERE (flag 38) |
| 2 | Attempt to set object to location 255 |
| 3 | PROCESS nesting limit exceeded (>10) |
| 4 | Attempt to nest DOALL |
| 5 | Illegal condact (corrupt database) |
| 6 | Invalid process number |
| 7 | Invalid message number |
| 8 | Invalid PICTURE |

In debug builds, errors show `p:v,n>c` where p=process, v=verb, n=noun, c=condact.

---

## History

- **The Quill** (1983, Gilsoft): First adventure writing system
- **PAW / Professional Adventure Writer** (Gilsoft): Evolution of The Quill
- **SWAN** (System Without A Name): Intermediate evolution
- **DAAD** (1988, Tim Gilberts for Aventuras AD): Multi-platform evolution of PAW/SWAN
- Price: >2,500,000 pesetas (~15,000 EUR)
- Used by Aventuras AD to create 6 commercial games (1989-1992)
- Supported 8 platforms: ZX Spectrum, Amstrad CPC, C64, MSX, PCW, Atari ST, Amiga, PC
- Lost after Aventuras AD closed (1993), recovered from original disks in 2014
- Tim Gilberts and Stefan Vogt completed missing parts (English interpreters) in 2017-2018
- Released to public domain; now maintained by the community

### Games Created with DAAD

| Year | Title |
|------|-------|
| 1989 | La Aventura Original |
| 1989 | Jabato |
| 1990 | Cozumel |
| 1990 | La Aventura Espacial |
| 1991 | Los Templos Sagrados |
| 1992 | Chichen Itza |
