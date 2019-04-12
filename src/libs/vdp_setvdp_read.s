;===============================================================================
; Set VDP address counter to read/write from address A+DE (17-bit)
; SSRRRRRRRRCCCCCCC     SCREEN 5:  S=Screen  R=Row  C=Column
; ADDDDDDDDEEEEEEEE
; In:  A+DE    VRAM address
;
_setVDP_Read::
        pop     af
        pop     bc
        pop     de
        push    de
        push    bc
        push    af
        ld      a,c
setVDP_Read::
        ld      c,#0
        jr      .jmp1
        
_setVDP_Write::
        pop     af
        pop     bc
        pop     de
        push    de
        push    bc
        push    af
        ld      a,c
setVDP_Write::
        ld      c,#64
.jmp1:
        rlc     d
        rla
        rlc     d
        rla
        srl     d
        srl     d
        di
        out     (0x99), a
        ld      a,#(14+128)
        out     (0x99), a
        ld      a,e
        nop
        out     (0x99),a
        ld      a,d
        or      c
        ei
        out     (0x99),a
        ret
