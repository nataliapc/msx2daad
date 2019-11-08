; void copyToVRAM(uint16_t source, uint32_t vram, uint16_t size);

	.globl setVDP_Write

_copyToVRAM::
		push ix
		ld   ix,#4
		add  ix,sp

		ld   l,0(ix)
		ld   h,1(ix)
		ld   e,2(ix)
		ld   d,3(ix)
		ld   a,4(ix)
		ld   c,6(ix)
		ld   b,7(ix)
        pop  ix

copyToVRAM::	; source:HL vram:A+DE size:BC
		push  de
		push  bc
		call  setVDP_Write	; A+DE
		pop   bc
		pop   de

		ld   d,b
		ld   b,c
		ld   c,#0x98

	c2v_loop0:
		outi
		jp   nz,c2v_loop0
		xor  a
		cp   d
		jr   z,c2v_end

	c2v_loop1:
		otir
		dec  d
		jp   nz,c2v_loop1

	c2v_end:
		ret
