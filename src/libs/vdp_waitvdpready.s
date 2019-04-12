;===============================================================================
; This routine waits until the VDP is done copying
; In:  -
; Out: -
; Chg: A
;
.include "vdp.s"

_waitVDPready::
waitVDPready::
        ld      a,#2
        di
        out     (0x99),a    ; select s#2
        ld      a,#(15+128)
        out     (0x99),a
        in      a,(0x99)
        rra
        ld      a,#0        ; back to s#0, enable ints
        out     (0x99),a
        ld      a,#(15+128)
        ei
        out     (0x99),a    ; loop if vdp not ready (CE)
        jp      c, waitVDPready
        ret
