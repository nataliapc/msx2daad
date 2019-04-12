;===============================================================================
; In:  A    Foreground color
;      E    Background color
;      E    Border color
; Out: -
; Chg: A DE HL
;
.include "vdp.s"

_setColor::
        push    ix
        ld      ix,#4
        add     ix,sp

        ld      a,0(ix)
        ld      e,1(ix)
        ld      d,2(ix)
        pop     ix

setColor::
        ld      hl,#FORCLR
        ld      (hl),a
        inc     hl
        ld      (hl),e
        inc     hl
        ld      (hl),d
        ret
