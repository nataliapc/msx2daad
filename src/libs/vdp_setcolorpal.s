;===============================================================================
; In: A        Color index to change
;     DE       Color definition D:-----GGG E:-RRR-BBB
;
.include "vdp.s"

_setColorPal::
        push ix
        ld ix,#4
        add ix,sp

        ld a,0(ix)
        ld e,1(ix)
        ld d,2(ix)
        pop ix

setColorPal::
        ld      b,#0                    ; set pointer to zero.
        ld      c,#0x99
        di
        out     (c), b
        ld      a, #(16+128)
        ei
        out     (c), a
        ld      c, #0x9A
        out     (c), e
        out     (c), d
        ret
