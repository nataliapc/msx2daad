;===============================================================================
; In:  A    Border color
; Out: -
; Chg: A
;
.include "vdp.s"

_setBorder::	; A = border
setBorder::
		out (0x99),a
		ld  a,#0x87
		out (0x99),a
		ret
