;===============================================================================
; Clear screen5 lines
; In:  HL  Start line
;      DE  Num. lines to clear
; Out: -
; Chg: A C HL
;
.include "vdp.s"

_clearSC5lines::
clearSC5lines::
        call    .clearsc5_ldir
        pop     af
        pop     hl
        pop     de
        push    de
        push    hl
        push    af
        ld      (BLT_DY), hl
        ld      (BLT_NY), de
        jr      .clearsc5_call
_clearSC5::
clearSC5::
        call    .clearsc5_ldir
.clearsc5_call:
        ld      hl,#BLT_SX
        call    fastVCopy
        ret
.clearsc5_ldir:
        ld      hl, #.clearData
        ld      de, #BLT_SX
        ld      bc, #15
        ldir
        ret
.clearData:
        .dw    0        ; SX (not used)
        .dw    0        ; SY (not used)
        .dw    0        ; DX
        .dw    0        ; DY
        .dw    256      ; NX
        .dw    212      ; NY
        .db    0        ; COL
        .db    0        ; ARG
        .db    CMD_HMMV ; CMD
