; void copyVRam2CmdData(uint32_t vram_src, uint16_t length);
;
; Reads `length` bytes from VRAM (starting at A+DE) and writes them to VDP
; port #9B (HMMC streaming). Caller MUST have dispatched an HMMC command via
; fastVCopy() previously; the HMMC operation persists during this routine.
;
; In:  A+DE = VRAM source address (24-bit)
;      BC   = length
;

	.globl setVDP_Read

	.area _CODE

_copyVRam2CmdData::
		push ix
		ld   ix,#4
		add  ix,sp

		ld   e,0(ix)
		ld   d,1(ix)
		ld   a,2(ix)		; A+DE = VRAM src
		ld   c,4(ix)
		ld   b,5(ix)		; BC = length
		pop  ix

		push bc
		call setVDP_Read	; A+DE → VDP read pointer
		pop  bc

		call setIndirectRegisterPointer	; Point to R44 (HMMC data port)

		ld   d, b			; D = blocks of 256 bytes
		ld   b, c			; B = remainder

		xor  a				; Any remainder?
		cp   b
		jr   z, .cv_block
	.cv_rest:
		in   a,(0x98)
		out  (0x9B),a
		djnz .cv_rest
		xor  a				; Any 256-byte blocks?
		cp   d
		ret  z
	.cv_block:
		in   a,(0x98)
		out  (0x9B),a
		djnz .cv_block
		dec  d
		jr   nz, .cv_block
		ret
