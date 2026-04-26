; void copyCmdData(uint8_t *src, uint16_t length) __sdcccall(1);
;
; Sends `length` bytes from RAM to VDP port #9B (HMMC streaming).
; Caller MUST have dispatched an HMMC command via fastVCopy() previously.
;
; In:  HL = src
;      DE = length
;

	.area _CODE

_copyCmdData::
copyCmdData::			; HL=src, DE=length
		ld   a, d
		or   e
		ret  z			; No data to send?

		call setIndirectRegisterPointer	; Point to R44 (HMMC data port)

		ld   b, e		; B = remainder
		ld   c, #0x9B

		xor  a			; Any remainder?
		cp   b
		jr   z, .ccd_loop1
	.ccd_loop0:
		otir			; Send remainder bytes
		cp   d			; Any 256-byte blocks?
		ret  z
	.ccd_loop1:
		otir			; Send 256-byte block
		dec  d
		jr   nz, .ccd_loop1
		ret

; ---------------------------------------------------------------
; Point R#17 (Indirect Register Pointer) to R#44 (CLR) with
; AII=1 (auto-increment OFF), so every byte sent to port #9B is
; written to R#44 (HMMC data byte). After fastVCopy, R#17 was
; left at 47 with AII=0 by the 15-byte OUTI burst.
; ---------------------------------------------------------------
_setIndirectRegisterPointer::
setIndirectRegisterPointer::
		ld   a, #44 | #0x80	; R#17 data: target=R44, AII=1 (no auto-inc)
		di
		out  (#0x99), a
		ld   a, #17 | #0x80	; select R#17
		ei
		out  (#0x99), a
		ret
