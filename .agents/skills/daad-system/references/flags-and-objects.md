# DAAD Flags and Objects

## The 256 Flags

DAAD maintains 256 byte-sized flags (`uint8_t flags[256]`). Flags 0-63 are **system flags** with defined meanings. Flags 64-255 are available for game author use.

### System Flags (0-63)

| Flag | Name | Description |
|------|------|-------------|
| 0 | fDark | Non-zero = game is dark (Object 0 is conventionally a light source) |
| 1 | fNOCarr | Count of objects currently carried (maintained by system) |
| 2 | fWork1 | System work variable |
| 3 | fWork2 / fFULL | System work variable / stack full boundary |
| 4-22 | *(stack)* | Small push/pop stack area (flags 3-23) |
| 23 | fEMPTY | Stack empty boundary |
| 24 | fStack | Stack pointer (2 bytes per push, max 10 pushes) |
| 25 | fO2Num | Second object number (resolved from Noun2) |
| 26 | fO2Con | Second object is-container flag (bit 7) |
| 27 | fO2Loc | Second object location |
| 28 | fDarkF | Dark-related flag |
| 29 | fGFlags | Graphics flags: bit 0 = mouse, bit 7 = graphics available |
| 30 | fScore | Player score (optional, author use) |
| 31 | fTurns | Turn counter low byte |
| 32 | fTurnsHi | Turn counter high byte (16-bit little-endian with flag 31) |
| 33 | fVerb | Current logical sentence verb ID |
| 34 | fNoun1 | First noun ID |
| 35 | fAdject1 | Adjective for first noun |
| 36 | fAdverb | Adverb |
| 37 | fMaxCarr | Max carryable objects (initially 4, set by ABILITY) |
| 38 | **fPlayer** | **Current player location** |
| 39-40 | fO2Att | Second object extended attributes (2 bytes) |
| 41 | fInStream | Input stream/window number (0 = current) |
| 42 | fPrompt | Prompt message number (0 = random from SM2-5) |
| 43 | fPrep | Preposition |
| 44 | fNoun2 | Second noun ID |
| 45 | fAdject2 | Second noun adjective |
| 46 | fCPNoun | Pronoun noun ("IT") |
| 47 | fCPAdject | Pronoun adjective |
| 48 | fTime | Timeout duration in seconds |
| 49 | fTIFlags | Timeout control bitmask (see below) |
| 50 | fDAObjNo | DOALL loop current object number |
| 51 | **fCONum** | **Currently referenced object number** |
| 52 | fStrength | Max carriable weight (initially 10, set by ABILITY) |
| 53 | fOFlags | Object print flags (bit 7 = objects listed by LISTOBJ) |
| 54 | fCOLoc | Current object's location |
| 55 | fCOWei | Current object's weight |
| 56 | fCOCon | 128 if current object is container |
| 57 | fCOWR | 128 if current object is wearable |
| 58-59 | fCOAtt | Current object's extended attributes (2 bytes) |
| 60 | fKey1 | Key code from INKEY |
| 61 | fKey2 | Extended key code (IBM only) |
| 62 | fScMode | Screen mode (platform-dependent) |
| 63 | fCurWin | Current window number (read-only) |

### Timeout Control Bitmask (Flag 49)

| Bit | Value | Meaning |
|-----|-------|---------|
| 0 | 1 | Timeout only on first character of input |
| 1 | 2 | Timeout on "More..." prompt |
| 2 | 4 | Timeout on ANYKEY |
| 3 | 8 | Clear window after input |
| 4 | 16 | Reprint input in current stream |
| 5 | 32 | Auto-recall of input after timeout |
| 6 | 64 | Data available for recall (system use) |
| 7 | 128 | **Timeout occurred last frame** |

### Referenced Object Update

When an object is referenced (by GET, DROP, WHATO, SETCO, etc.), flags 51-59 are updated:

| Flag | Content |
|------|---------|
| 51 | Object number |
| 54 | Object's location |
| 55 | Weight (6-bit) |
| 56 | Bit 7 = is container |
| 57 | Bit 7 = is wearable |
| 58-59 | Extended attributes (2 bytes) |

---

## Objects

### Object Structure (runtime, 6 bytes)

| Field | Size | Description |
|-------|------|-------------|
| location | 1 byte | Current location (see special values below) |
| attribs | 1 byte | bits 0-5: weight (0-63), bit 6: is_container, bit 7: is_wearable |
| extAttr | 2 bytes | 16 user-defined boolean attribute bits |
| nounId | 1 byte | Vocabulary noun ID |
| adjectiveId | 1 byte | Vocabulary adjective ID |

### Special Location Values

| Value | Name | Meaning |
|-------|------|---------|
| 0-251 | *(locations)* | Actual game location numbers |
| 252 | NOT_CREATED | Object does not exist in the game world |
| 253 | WORN | Object is being worn by the player |
| 254 | CARRIED | Object is being carried by the player |
| 255 | HERE | Pseudo-value: player's current location (used in searches, not stored) |

### Object 0 Convention

Object 0 has special significance: it is conventionally used as a **light source**. When flag 0 (fDark) is non-zero, the game is in darkness. If Object 0 is PRESENT (carried, worn, or at player's location), the player can still see.

### Object Extended Attributes (16 bits)

The 16 extended attribute bits (0-15) are user-defined. Common conventions include attributes for: edible, drinkable, poisonous, lit, openable, open, lockable, locked, etc. These are tested with HASAT/HASNAT condacts.

### Object Definition in DSF

```
;obj  starts  weight    c w  5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0    noun   adjective
/0    CARRIED   1       _ _  _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _    TORCH  _
```

Fields:
- Object number (must match /OTX index)
- Starting location: `CARRIED`, `WORN`, `NOT_CREATED`, or a location number
- Weight (0-63)
- `c` = carriable, `w` = wearable
- 16 user attribute bits (underscore = unset)
- Noun vocabulary word
- Adjective vocabulary word (underscore = none)

---

## System Messages (Required STX)

| SM# | Default Content (English) |
|-----|---------------------------|
| 0 | "It's too dark to see anything." |
| 1 | "I can also see:" |
| 2-5 | Random prompt messages ("What now?", etc.) |
| 6 | "I didn't understand." |
| 7 | "I can't go in that direction." |
| 8 | "I can't do that." |
| 9 | "I have with me:" |
| 10 | "I am wearing:" |
| 12 | "Are you sure?" |
| 13 | "Would you like another go?" |
| 15 | "OK." |
| 16 | "Press any key to continue." |
| 23 | "I'm not wearing one of those." |
| 24 | "I can't. I'm wearing the _." |
| 25 | "I already have the _." |
| 26 | "There isn't one of those here." |
| 27 | "I can't carry any more things." |
| 28 | "I don't have one of those." |
| 29 | "I'm already wearing the _." |
| 30 | "Y" (yes confirmation character) |
| 31 | "N" (no confirmation character) |
| 32 | "More..." |
| 33 | "\n>" (prompt string) |
| 36 | "I now have the _." |
| 37 | "I'm now wearing the _." |
| 38 | "I've removed the _." |
| 39 | "I've dropped the _." |
| 40 | "I can't wear the _." |
| 41 | "I can't remove the _." |
| 42 | "I can't remove the _. My hands are full." |
| 43 | "The _ weighs too much for me." |
| 44 | "The _ is in the" |
| 45 | "The _ isn't in the" |
| 46 | ", " (list separator) |
| 47 | " and " (last list separator) |
| 48 | ".\n" (list terminator) |
| 49 | "I don't have the _." |
| 50 | "I'm not wearing the _." |
| 51 | "." (sentence terminator) |
| 52 | "There isn't one of those in the" |
| 53 | "Nothing." |
| 57 | "I/O Error" |
| 59 | "File name error." |
| 60 | "Type in name of file." |
