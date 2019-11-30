;===============================================================================
; In:  B     Value to store
;      C     Register number
; Out: -
; Chg: AF HL DE 
;
.include "vdp.s"

_setRegVDP8::
        pop     af
        pop     bc
        push    bc
        push    af
setRegVDP8::
        ld      a,c
        sub     #8
        ld      hl,#RG8SAV
        jr      .setregvdp_cont
_setRegVDP::
        pop     af
        pop     bc
        push    bc
        push    af
setRegVDP::
        ld      a,c
        ld      hl,#RG0SAV
.setregvdp_cont:
        ld      e,a
        ld      d,#0
        add     hl,de
        ld      (hl),b                  ;save copy of vdp value in system var

        ld      a,b
        di
        out     (0x99),a
        ld      a,#128
        add     c
        ei
        out     (0x99),a
        ret
