;===============================================================================
; Clear screen7 lines
; In:  HL  Start line
;      DE  Num. lines to clear
; Out: -
; Chg: A C HL
;
.include "vdp.s"

_clearSC7lines::
clearSC7lines::
        call    .clearsc7_ldir
        pop     af
        pop     hl
        pop     de
        push    de
        push    hl
        push    af
        ld      (BLT_DY), hl
        ld      (BLT_NY), de
        jr      .clearsc7_call
_clearSC7::
clearSC7::
        call    .clearsc7_ldir
.clearsc7_call:
        ld      hl,#BLT_SX
        call    fastVCopy
        ret
.clearsc7_ldir:
        ld      hl, #.clearData7
        ld      de, #BLT_SX
        ld      bc, #15
        ldir
        ret
.clearData7:
        .dw    0        ; SX (not used)
        .dw    0        ; SY (not used)
        .dw    0        ; DX
        .dw    0        ; DY
        .dw    512      ; NX
        .dw    212      ; NY
        .db    0        ; COL
        .db    0        ; ARG
        .db    CMD_HMMV ; CMD
