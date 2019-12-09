;===============================================================================
; In:  A: page view to be activated [0..3]
; Out: -
; Chg: A BC HL DE
;
.include "vdp.s"

_setVPage::
		pop  af
		pop  bc
		push bc
		push af
		ld   a,c

setVPage::
		sla  a
		sla  a
		sla  a
		sla  a
		sla  a
		or   #0b00011111
		ld   b,a
		ld   c,#2
		jp   setRegVDP
