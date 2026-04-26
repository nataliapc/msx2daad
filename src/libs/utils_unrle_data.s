; RLE decompression to VDP port #9B (HMMC streaming).
; Adapted from utils_unrle_vram.s — same RLE format, but:
;   - Output goes to port #9B instead of #98 (no setVDP_Write needed).
;   - No VRAM address tracking (HMMC manages destination internally).
;   - skip-output token (mark+0x01+N) is NOT supported — V9938CmdData
;     never emits it because rectangles are contiguous in HMMC mode.
;
; Caller MUST have dispatched an HMMC command via fastVCopy() previously.
;
; Tokens:
;   Mark+0x00            → EOF (2 bytes)
;   Mark+N+value (N>3)   → repeated value (3 bytes)
;
; In: HL = source pointer
;

	.area _CODE


_unRLE_Data::	; void unRLE_Data(char *source);
		pop  af
		pop  hl
		push hl
		push af

unRLE_Data::	; HL = source
		call setIndirectRegisterPointer	; Point to R44 (HMMC data port)

		ld   c, #0x9B
		ld   a, (hl)		; A = mark byte
		ld   (#.urd_mark), a
		inc  hl

	.urd_loop:
		ld   a, (#.urd_mark)
	.urd_loop2:
		cp   (hl)
		jr   z, .urd_rle	; byte == mark → compressed/eof
		outi				; raw byte to #9B
		jr   .urd_loop2

	.urd_rle:
		inc  hl
		xor  a
		cp   (hl)			; mark+0x00 → EOF
		ret  z
		ld   b, (hl)		; B = repeat count
		inc  hl
		ld   a, (hl)		; A = value to repeat
		inc  hl
	.urd_rep:
		out  (0x9B), a
		djnz .urd_rep
		jr   .urd_loop

	.urd_mark:
		.ds  1
