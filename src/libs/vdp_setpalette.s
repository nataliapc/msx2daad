;===============================================================================
; In:  HL    Palette RAM address
;
.include "vdp.s"

_setPalette::
	pop     af
	pop     hl
	push    hl
	push    af

setPalette::
        xor     a                       ; set pointer to zero.
        di
        out     (0x99), a
        ld      a, #(16+128)
        ei
        out     (0x99), a
        ld      bc, #0x209A             ; out 32 bytes ($20) to port $9A
        otir
        ret
