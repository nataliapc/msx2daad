; -----------------------------------------------------------------------------
; GFX9000 (V9990) command engine helpers.
;
; void g9kWaitCmd();
;     Wait until the command engine is idle (P#5 bit 0 CE = 0).
;
; void g9kSendCmd(const G9K_CMD *cmd) __sdcccall(1);
;     In: HL = 21-byte packet for R#32..R#52. Waits for the previous command,
;     writes the packet (R#52 OP starts it) and then waits a little for CE to
;     rise, so that a fast CPU (R800) does not rewrite the registers of a
;     command that has not started yet.
;
; void g9kSetReg(uint8_t reg, uint8_t value) __sdcccall(1);
;     In: A = register, L = value.
; -----------------------------------------------------------------------------

	.area _CODE

_g9kWaitCmd::
	in   a,(#0x65)			; P#5 status
	rrca					; CE -> carry
	jr   c,_g9kWaitCmd
	ret

_g9kSendCmd::
	call _g9kWaitCmd
	ld   a,#32				; R#32, write increment on
	out  (#0x64),a
	ld   bc,#0x1563			; B = 21 bytes, C = P#3 register data
	otir
	ld   b,#16
.g9k_taken:
	in   a,(#0x65)
	rrca
	ret  c					; CE set: the command has started
	djnz .g9k_taken
	ret						; already finished (short command)

_g9kSetReg::
	out  (#0x64),a			; P#4 register select
	ld   a,l
	out  (#0x63),a			; P#3 register data
	ret
