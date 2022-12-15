;===============================================================================
; void bitBlt(uint16_t sx, uint16_t sy, uint16_t dx, uint16_t dy, uint16_t nx, uint16_t ny, uint8_t col, uint8_t arg, uint8_t cmd);
;
.include "vdp.s"

_bitBlt::
		ld      hl,#2
		add     hl,sp

		jp      fastVCopy
