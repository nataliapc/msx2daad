# DAAD Condacts Reference

Condacts are the bytecode instructions of the DAAD virtual machine. There are **128 condacts** (0-127), divided into **conditions** (test something, fail = skip to next entry) and **actions** (do something).

## Condact Encoding

Each condact in bytecode is:
- **Byte 0**: `[indirection_bit(7)][condact_number(6:0)]`
- **Byte 1**: first argument (if applicable)
- **Byte 2**: second argument (if applicable)

Indirection (`@`): When bit 7 is set, the first argument is treated as a flag number and dereferenced: `value = flags[argument]`.

## Complete Condact Table

| # | Name | Args | Type | Description |
|---|------|:----:|:----:|-------------|
| 0 | AT | 1 | C | True if player at location `locno` |
| 1 | NOTAT | 1 | C | True if player NOT at `locno` |
| 2 | ATGT | 1 | C | True if player location > `locno` |
| 3 | ATLT | 1 | C | True if player location < `locno` |
| 4 | PRESENT | 1 | C | True if object is carried, worn, or at player's location |
| 5 | ABSENT | 1 | C | True if object is NOT present |
| 6 | WORN | 1 | C | True if object is worn |
| 7 | NOTWORN | 1 | C | True if object is NOT worn |
| 8 | CARRIED | 1 | C | True if object is carried |
| 9 | NOTCARR | 1 | C | True if object is NOT carried |
| 10 | CHANCE | 1 | C | True with `percent`% probability (1-100) |
| 11 | ZERO | 1 | C | True if flag is zero |
| 12 | NOTZERO | 1 | C | True if flag is not zero |
| 13 | EQ | 2 | C | True if `flag == value` |
| 14 | GT | 2 | C | True if `flag > value` |
| 15 | LT | 2 | C | True if `flag < value` |
| 16 | ADJECT1 | 1 | C | True if 1st noun adjective matches `word` |
| 17 | ADVERB | 1 | C | True if adverb matches `word` |
| 18 | SFX | 2 | A | Write `value` to sound chip register `reg` |
| 19 | DESC | 1 | A | Print location description (no newline) |
| 20 | QUIT | 0 | C | Print SM12 ("Are you sure?"), true if player confirms |
| 21 | END | 0 | A | Print SM13 ("Play again?"), end or restart game |
| 22 | DONE | 0 | A | Exit process table (action was performed) |
| 23 | OK | 0 | A | Print SM15 ("OK"), then DONE |
| 24 | ANYKEY | 0 | A | Print SM16 ("Press any key"), wait for keypress |
| 25 | SAVE | 1 | A | Save game to disk |
| 26 | LOAD | 1 | A | Load game from disk |
| 27 | DPRINT | 1 | A | Print 16-bit value from `flag` and `flag+1` |
| 28 | DISPLAY | 1 | A | If 0: show buffered picture. If non-0: clear window area |
| 29 | CLS | 0 | A | Clear current window |
| 30 | DROPALL | 0 | A | Drop all carried/worn objects at current location |
| 31 | AUTOG | 0 | A | Auto-GET: find object by Noun1, priority: here>carried>worn |
| 32 | AUTOD | 0 | A | Auto-DROP: find object by Noun1, priority: carried>worn>here |
| 33 | AUTOW | 0 | A | Auto-WEAR: find object by Noun1, priority: carried>worn>here |
| 34 | AUTOR | 0 | A | Auto-REMOVE: find object by Noun1, priority: worn>carried>here |
| 35 | PAUSE | 1 | A | Pause for `value/50` seconds (0 = 256/50 sec) |
| 36 | SYNONYM | 2 | A | Replace verb and/or noun in logical sentence |
| 37 | GOTO | 1 | A | Set player location (flag 38) to `locno` |
| 38 | MESSAGE | 1 | A | Print user message + newline |
| 39 | REMOVE | 1 | A | Remove (unwear) object; checks worn/carried/wearable |
| 40 | GET | 1 | A | Pick up object; checks location/weight/capacity |
| 41 | DROP | 1 | A | Drop object; checks carried/worn |
| 42 | WEAR | 1 | A | Wear object; checks carried/wearable |
| 43 | DESTROY | 1 | A | Set object location to NOT_CREATED (252) |
| 44 | CREATE | 1 | A | Set object location to player's current location |
| 45 | SWAP | 2 | A | Exchange locations of two objects |
| 46 | PLACE | 2 | A | Place object at specific location |
| 47 | SET | 1 | A | Set flag to 255 |
| 48 | CLEAR | 1 | A | Set flag to 0 |
| 49 | PLUS | 2 | A | `flag += value` (saturate at 255) |
| 50 | MINUS | 2 | A | `flag -= value` (floor at 0) |
| 51 | LET | 2 | A | `flag = value` |
| 52 | NEWLINE | 0 | A | Print carriage return / newline |
| 53 | PRINT | 1 | A | Print flag value as decimal (no spaces) |
| 54 | SYSMESS | 1 | A | Print system message |
| 55 | ISAT | 2 | C | True if object is at location (255 = player's location) |
| 56 | SETCO | 1 | A | Set currently referenced object to `objno` |
| 57 | SPACE | 0 | A | Print a space character |
| 58 | HASAT | 1 | C | True if attribute bit is set (see HASAT reference) |
| 59 | HASNAT | 1 | C | True if attribute bit is NOT set |
| 60 | LISTOBJ | 0 | A | List objects at current location (prefixed by SM1) |
| 61 | EXTERN | 2 | A | Call external routine (MALUVA extensions) |
| 62 | RAMSAVE | 0 | A | Save game state to RAM buffer |
| 63 | RAMLOAD | 1 | A | Load game state from RAM buffer (restore up to `flagno`) |
| 64 | BEEP | 2 | A | Play tone: `length` (1/50s), `tone` (see tone table) |
| 65 | PAPER | 1 | A | Set background color |
| 66 | INK | 1 | A | Set text color |
| 67 | BORDER | 1 | A | Set border color |
| 68 | PREP | 1 | C | True if preposition matches `word` |
| 69 | NOUN2 | 1 | C | True if second noun matches `word` |
| 70 | ADJECT2 | 1 | C | True if second noun's adjective matches `word` |
| 71 | ADD | 2 | A | `flag2 += flag1` (saturate at 255) |
| 72 | SUB | 2 | A | `flag2 -= flag1` (floor at 0) |
| 73 | PARSE | 1 | A | Parse input: 0 = next sentence, 1 = quoted string |
| 74 | LISTAT | 1 | A | List objects at `locno` (SM53 if empty) |
| 75 | PROCESS | 1 | A | Call sub-process (max 10 nesting levels) |
| 76 | SAME | 2 | C | True if `flag1 == flag2` |
| 77 | MES | 1 | A | Print user message (no newline) |
| 78 | WINDOW | 1 | A | Select active window (0-7) |
| 79 | NOTEQ | 2 | C | True if `flag != value` |
| 80 | NOTSAME | 2 | C | True if `flag1 != flag2` |
| 81 | MODE | 1 | A | Set window mode: bit0=force upper charset, bit1=disable "More..." |
| 82 | WINAT | 2 | A | Set window position (line, col) |
| 83 | TIME | 2 | A | Set timeout: `duration` (seconds), `option` (bitmask) |
| 84 | PICTURE | 1 | A | Load picture into buffer (next condact runs only if found) |
| 85 | DOALL | 1 | A | Iterate all objects at `locno` (see DOALL section) |
| 86 | MOUSE | 1 | A | Mouse handler (skeleton) |
| 87 | GFX | 2 | A | Graphics extension: `param`, `routine` (see GFX table) |
| 88 | ISNOTAT | 2 | C | True if object is NOT at location |
| 89 | WEIGH | 2 | A | Copy true weight of object (incl. contents) to flag |
| 90 | PUTIN | 2 | A | Put carried object into container location |
| 91 | TAKEOUT | 2 | A | Take object from container location |
| 92 | NEWTEXT | 0 | A | Discard remaining input phrases |
| 93 | ABILITY | 2 | A | Set max carry (flag 37) and strength (flag 52) |
| 94 | WEIGHT | 1 | A | Copy total carried+worn weight to flag |
| 95 | RANDOM | 1 | A | Set flag to random number 1-100 |
| 96 | INPUT | 2 | A | Configure input stream and options |
| 97 | SAVEAT | 0 | A | Save cursor position for current window |
| 98 | BACKAT | 0 | A | Restore saved cursor position |
| 99 | PRINTAT | 2 | A | Set cursor to (line, col) in current window |
| 100 | WHATO | 0 | A | Find object matching Noun1, set as current object |
| 101 | CALL | 1 | A | Call machine code at address |
| 102 | PUTO | 1 | A | Move current object (flag 51) to location |
| 103 | NOTDONE | 0 | A | Exit process table (NO action performed) |
| 104 | AUTOP | 1 | A | Auto-PUTIN: find by Noun1, priority: carried>worn>here |
| 105 | AUTOT | 1 | A | Auto-TAKEOUT: find by Noun1, priority: container>carried>worn>here |
| 106 | MOVE | 1 | A | Move player via connection table using current verb |
| 107 | WINSIZE | 2 | A | Set window size (height, width) |
| 108 | REDO | 0 | A | Restart current process table from first entry |
| 109 | CENTRE | 0 | A | Center current window horizontally |
| 110 | EXIT | 1 | A | 0 = quit to OS; non-0 = restart with part number |
| 111 | INKEY | 0 | C | True if key pressed; key code stored in flag 60 |
| 112 | BIGGER | 2 | C | True if `flag1 > flag2` |
| 113 | SMALLER | 2 | C | True if `flag1 < flag2` |
| 114 | ISDONE | 0 | C | True if last sub-process performed an action |
| 115 | ISNDONE | 0 | C | True if last sub-process did nothing |
| 116 | SKIP | 1 | A | Skip `distance` entries in table (-128 to +128) |
| 117 | RESTART | 0 | A | Cancel all processes, restart from Process 0 |
| 118 | TAB | 1 | A | Set cursor column |
| 119 | COPYOF | 2 | A | Copy object location to flag |
| 120 | *(unused)* | - | - | Reserved |
| 121 | COPYOO | 2 | A | Copy object1's location to object2 |
| 122 | *(unused)* | - | - | Reserved |
| 123 | COPYFO | 2 | A | Set object location from flag value |
| 124 | *(unused)* | - | - | Reserved |
| 125 | COPYFF | 2 | A | Copy flag1 to flag2 |
| 126 | COPYBF | 2 | A | Copy flag2 to flag1 (reversed, for indirection use) |
| 127 | RESET | 0 | A | Reset all objects to initial positions |

**Type**: C = condition, A = action.

---

## GFX Routines (condact 87)

| Routine | Description |
|---------|-------------|
| 0 | Copy back buffer -> physical screen |
| 1 | Copy physical screen -> back buffer |
| 2 | Swap physical and back buffers |
| 3 | Graphics write to physical screen |
| 4 | Graphics write to back buffer |
| 5 | Clear physical screen |
| 6 | Clear back buffer |
| 7 | Text write to physical (Atari ST only) |
| 8 | Text write to back (Atari ST only) |
| 9 | Set palette: param = flag offset of 4-byte block (index, R, G, B; 0-255 range) |
| 10 | Read palette: param = flag offset of 4-byte block |

---

## BEEP Tone Table (condact 64)

Duration is in 1/50ths of a second. Tone uses this mapping:

```
Oct  C   C#  D   D#  E   F   F#  G   G#  A   A#  B
#1   48  50  52  54  56  58  60  62  64  66  68  70
#2   72  74  76  78  80  82  84  86  88  90  92  94
#3   96  98  100 102 104 106 108 110 112 114 116 118
#4   120 122 124 126 128 130 132 134 136 138 140 142
#5   144 146 148 150 152 154 156 158 160 162 164 166
#6   168 170 172 174 176 178 180 182 184 186 188 190
#7   192 194 196 198 200 202 204 206 208 210 212 214
#8   216 218 220 222 224 226 228 230 232 234 236 238
```

Standard durations: 200=whole, 100=half, 50=quarter, 25=eighth, 12=sixteenth.

---

## DOALL Mechanism

`DOALL locno` iterates over all objects at `locno`:

1. Find next object at `locno` (ascending order from last found)
2. Skip if object's noun+adjective matches Noun2+Adjective2 (implements "ALL EXCEPT X")
3. Set Noun1/Adjective1 from found object's vocabulary IDs
4. Execute remaining condacts/entries in the table
5. On DONE, loop back to step 1
6. When no more objects found, perform DONE

DOALL cannot be nested (runtime error). Cancelled by GET/TAKEOUT capacity failures.

---

## HASAT/HASNAT Bit Reference

These condacts test individual bits within flags. Formula: `flag = param / 8`, `bit = param % 8`.

Object extended attributes use values 0-15. Special system values:

| Value | Flag.Bit | Meaning |
|-------|----------|---------|
| 23 | 57.7 | Current object is wearable |
| 31 | 56.7 | Current object is a container |
| 55 | 53.7 | Objects were listed by LISTOBJ |
| 87 | 49.7 | Timeout occurred last frame |
| 240 | 29.0 | Mouse available |
| 247 | 29.7 | Graphics mode available |

---

## AUTO- Search Priority

| Condact | Search order |
|---------|-------------|
| AUTOG | here -> carried -> worn |
| AUTOD | carried -> worn -> here |
| AUTOW | carried -> worn -> here |
| AUTOR | worn -> carried -> here |
| AUTOP | carried -> worn -> here |
| AUTOT | container -> carried -> worn -> here |
