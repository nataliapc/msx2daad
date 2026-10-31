; -----------------------------------------------------------------------------
; ZX0 decoder (Standard, v2 format) by Einar Saukas & Urusergi.
; Adapted to the GFX9000 (V9990) picture buffer (see g9k_scratch.s).
; Derived from utils_dzx0vram.s (the V9938 version).
;
; void g9kScratchZX0(const uint8_t *src) __sdcccall(1);
;     In: HL = ZX0 stream. Output goes through P#0 at the VRAM write pointer,
;     which must be at g9kScrPos (g9kScratchBegin and g9kScratchAdvance).
;
; The V9990 has separate read and write pointers: a back reference sets the
; read address once and streams P#0 -> P#0, while the write pointer runs on.
; DE counts the bytes written by this chunk (added once per run, not per
; byte), so a source is g9kScrPos + DE + offset, 19 bits.
; The read port goes through a one byte buffer, refilled when R#5 is written
; and after every read (openMSX V9990.cc; the ZEMMIX core does the same): a
; copy from offset -1 would read stale data, so it is done as a fill. Any
; other offset only reads bytes written in an earlier iteration.
; -----------------------------------------------------------------------------

	.globl _g9kScrPos

	.area _CODE

_g9kScratchZX0::
	ld   de,#0				; DE = bytes written by this chunk
	ld   bc,#0xffff
	ld   (#g9kzx0_offset),bc	; default offset -1
	inc  bc					; BC = 0
	ld   a,#0x80

g9kzx0_literals:
	call g9kzx0_elias		; BC = literal length
	push af
	ex   de,hl				; DE += BC (once for the run)
	add  hl,bc
	ex   de,hl
	ld   a,b
	or   c
	jr   z,g9kzx0_lit_done
g9kzx0_lit_loop:
	ld   a,(hl)
	out  (#0x60),a			; VRAM write (auto increment)
	cpi						; HL++, BC--, P/V = (BC != 0)
	jp   pe,g9kzx0_lit_loop
g9kzx0_lit_done:
	pop  af
	add  a,a				; copy from last offset or new offset?
	jr   c,g9kzx0_new_offset
	call g9kzx0_elias		; BC = length

g9kzx0_copy:
	push af
	push hl
	; read address = g9kScrPos + DE + offset (offset < 0, DE + offset >= 0)
	ld   hl,(#g9kzx0_offset)
	add  hl,de
	push de
	ld   de,(#_g9kScrPos)
	add  hl,de
	ld   a,(#_g9kScrPos+2)
	adc  a,#0
	ld   e,a
	ld   a,#3				; R#3, write increment on
	out  (#0x64),a
	ld   a,l
	out  (#0x63),a
	ld   a,h
	out  (#0x63),a
	ld   a,e
	out  (#0x63),a			; R#5: read pointer + buffer reload
	pop  de
	ex   de,hl				; DE += BC (once for the run)
	add  hl,bc
	ex   de,hl
	ld   hl,(#g9kzx0_offset)
	inc  hl
	ld   a,h
	or   l
	jr   z,g9kzx0_fill		; offset -1: repeat the previous byte
g9kzx0_copy_loop:
	in   a,(#0x60)
	out  (#0x60),a
	dec  bc
	ld   a,b
	or   c
	jr   nz,g9kzx0_copy_loop
	jr   g9kzx0_copy_done
g9kzx0_fill:
	in   a,(#0x60)
	ld   l,a
g9kzx0_fill_loop:
	ld   a,l
	out  (#0x60),a
	dec  bc
	ld   a,b
	or   c
	jr   nz,g9kzx0_fill_loop
g9kzx0_copy_done:
	pop  hl
	pop  af
	add  a,a				; copy from literals or new offset?
	jr   nc,g9kzx0_literals

g9kzx0_new_offset:
	ld   c,#0xfe			; prepare negative offset
	call g9kzx0_elias_loop	; offset MSB
	inc  c
	ret  z					; end marker
	ld   b,c
	ld   c,(hl)				; offset LSB
	inc  hl
	rr   b					; last offset bit becomes first length bit
	rr   c
	ld   (#g9kzx0_offset),bc
	ld   bc,#1
	call nc,g9kzx0_elias_backtrack
	inc  bc
	jr   g9kzx0_copy

; Elias gamma (identical to the RAM decoder)
g9kzx0_elias:
	inc  c
g9kzx0_elias_loop:
	add  a,a
	jr   nz,g9kzx0_elias_skip
	ld   a,(hl)
	inc  hl
	rla
g9kzx0_elias_skip:
	ret  c
g9kzx0_elias_backtrack:
	add  a,a
	rl   c
	rl   b
	jr   g9kzx0_elias_loop


	.area _DATA

g9kzx0_offset:
	.ds  2
