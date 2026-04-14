;===============================================================================
; In: A        Color index to change
;     DE       Color definition D:-----GGG E:-RRR-BBB
;
.include "vdp.s"

_setColorPal::
	; A = colindex
	; DE = grb
setColorPal::
	ld      c,#0x99                    ; set port to 0x99
	di
	out     (c), a
	ld      a, #(16+128)
	ei
	out     (c), a
	inc     c
	out     (c), e
	out     (c), d
	ret
