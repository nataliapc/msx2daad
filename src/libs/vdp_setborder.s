;===============================================================================
; In:  A    Border color
; Out: -
; Chg: A
;
.include "vdp.s"

_setBorder::
		pop de
		pop bc
		push bc
		push de
		ld  a,c

setBorder::
		out (0x99),a
		ld  a,#0x87
		out (0x99),a
		ret
