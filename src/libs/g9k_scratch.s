; -----------------------------------------------------------------------------
; GFX9000 (V9990) picture buffer: the pixel bytes of an LMMC rectangle are
; written to a linear VRAM zone (G9K_VRAM_SCRATCH) through P#0, and the
; rectangle is then drawn with one BMXL (linear VRAM -> rectangle). The CPU
; never waits for TR and never copies the data twice.
;
; The VRAM write pointer runs on by itself from g9kScratchBegin() to the
; last chunk; g9kScrPos keeps the same position (19 bits) for the ZX0 back
; references, and the caller adds every chunk with g9kScratchAdvance().
;
; void g9kScratchBegin();
; void g9kScratchAdvance(uint16_t length) __sdcccall(1);     HL = length
; void g9kScratchRaw(const uint8_t *src, uint16_t length);   HL = src, DE = length
; void g9kScratchRLE(const uint8_t *src);                    HL = RLE stream
;     RLE: first byte is the mark; then  mark,0 = end;  mark,n,value = n times
;     value (n 1-255);  any other byte = literal.
; -----------------------------------------------------------------------------

	.globl _g9kWaitCmd

	.area _CODE

SCRATCH_PAGE = 0x06			; G9K_VRAM_SCRATCH >> 16

_g9kScratchBegin::
	call _g9kWaitCmd		; a BMXL of the previous picture may still read the buffer
	ld   hl,#0
	ld   (#_g9kScrPos),hl
	ld   a,#SCRATCH_PAGE
	ld   (#_g9kScrPos+2),a
	xor  a					; R#0, write increment on
	out  (#0x64),a
	out  (#0x63),a			; R#0 address bits 7-0
	out  (#0x63),a			; R#1 address bits 15-8
	ld   a,#SCRATCH_PAGE
	out  (#0x63),a			; R#2 address bits 18-16 (pointer update)
	ret

_g9kScratchAdvance::
	ld   de,(#_g9kScrPos)
	add  hl,de
	ld   (#_g9kScrPos),hl
	ret  nc
	ld   hl,#_g9kScrPos+2
	inc  (hl)
	ret

_g9kScratchRaw::
	ld   a,d
	or   e
	ret  z
	ld   c,#0x60			; P#0 VRAM data
.g9k_raw_loop:
	outi
	dec  de
	ld   a,d
	or   e
	jr   nz,.g9k_raw_loop
	ret

_g9kScratchRLE::
	ld   e,(hl)				; E = mark
	inc  hl
	ld   c,#0x60
.g9k_rle_loop:
	ld   a,(hl)
	cp   e
	jr   z,.g9k_rle_token
	outi					; literal
	jr   .g9k_rle_loop
.g9k_rle_token:
	inc  hl
	ld   a,(hl)				; count (0 = end)
	inc  hl
	or   a
	ret  z
	ld   b,a
	ld   a,(hl)				; value
	inc  hl
.g9k_rle_rep:
	out  (#0x60),a
	djnz .g9k_rle_rep
	jr   .g9k_rle_loop


	.area _DATA

_g9kScrPos::
	.ds  3					; VRAM address of the next scratch byte (bits 18-0)
