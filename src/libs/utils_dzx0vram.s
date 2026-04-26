; -----------------------------------------------------------------------------
; ZX0 decoder (Standard) by Einar Saukas & Urusergi
; Adapted to V9938 VRAM destination (PRP025)
;
; Reference RAM-only decoder: src/libs/utils_dzx0.s (68 bytes)
; Reference VRAM pattern:    src/libs/utils_pletter2vram.s
;
; Differences from RAM ZX0:
;   - Two LDIRs replaced with VRAM-aware loops:
;       * literals: read from RAM, OUT to VDP port #98 (write mode).
;       * back-ref: per byte, switch VDP read mode at (DE-offset),
;                   IN from #98, switch VDP write mode at DE, OUT to #98.
;   - DE tracks the low 16 bits of a 17-bit VRAM destination address.
;   - (#zx0_page) holds the high bit (bit 16) of the VRAM dst address.
;   - Each chunk MUST fit in one 16KB VDP page (cap enforced by imgwizard:
;     CHUNK_PLETTER_MAX_UNCOMP=11264 < 16384). No mid-decode R#14 reload.
; -----------------------------------------------------------------------------
; Calling convention: SDCC_STACKCALL (__sdcccall(0))
; void dzx0vram(unsigned char *source, uint32_t vram_offset);
;
; Stack layout on entry:
;   SP+0: return address
;   SP+2: source ptr (16-bit)
;   SP+4: vram_offset (32-bit, little-endian; only low 17 bits significant)
; -----------------------------------------------------------------------------

	.area _CODE

	.globl  _dzx0vram


_dzx0vram::
	; Save caller regs first, then extract args via HL+SP indexing.
	; After 4 pushes, args are at SP+10 (src), SP+12 (vram_offset).
	push bc
	push de
	push hl
	push iy
	ld   hl,#10
	add  hl,sp                 ; HL = &src
	ld   c,(hl)
	inc  hl
	ld   b,(hl)                ; BC = src
	inc  hl
	ld   e,(hl)
	inc  hl
	ld   d,(hl)                ; DE = vram_offset low 16 bits
	inc  hl
	ld   a,(hl)                ; A  = vram_offset bit 16 (page)
	ld   h,b
	ld   l,c                   ; HL = src

	ld   (#zx0_page),a

	; Configure VDP write mode at vram_offset (DE + page in A)
	scf                         ; carry=1 → write mode
	call zx0_setVDP_DE

	; Init offset to default 0xFFFF (= -1) in static memory.
	ld   bc,#0xffff
	ld   (#zx0_offset),bc
	inc  bc                     ; BC = 0
	ld   a,#0x80

zx0v_literals:
	call zx0v_elias             ; obtain length (BC)
	; --- inlined zx0v_ldir_ram_to_vram (single call site) ---
	; Copy BC literals from (HL) RAM to VRAM(DE). VDP already in write mode.
	push af                     ; preserve A+flags (Elias bit accumulator)
	ld   a,b
	or   c
	jr   z,zx0v_lrv_done
zx0v_lrv_loop:
	ld   a,(hl)
	out  (#0x98),a              ; VRAM write (auto-inc)
	inc  de                     ; track dst position (RAM-side counter)
	cpi                         ; HL++; BC--; flags: P/V=1 if BC≠0 (-1 byte vs inc/dec/or)
	jp   pe,zx0v_lrv_loop
zx0v_lrv_done:
	pop  af
	; --- end inlined ---
	add  a,a                    ; copy from last offset or new offset?
	jr   c,zx0v_new_offset
	call zx0v_elias             ; obtain length (BC)

zx0v_copy:
	; --- inlined zx0v_ldir_vram_to_vram (single call site) ---
	; Back-ref: copy BC bytes from VRAM(DE+offset_negative) to VRAM(DE).
	; Offset in (#zx0_offset). VDP-mode switched per byte (read src / write dst).
	push af                     ; preserve A+flags (Elias bit accumulator)
	push hl                     ; preserve src RAM ptr (HL destroyed inside loop)
	ld   a,b
	or   c
	jr   z,zx0v_lvv_done
zx0v_lvv_loop:
	; src VRAM = dst (DE,page) + offset (#zx0_offset). ZX0 stores offset as
	; NEGATIVE 16-bit (two's complement), so `add hl,de` (not sbc); page byte
	; propagates via `adc a,#0xff` (carry=1 → page; carry=0 → page-1).
	push de                     ; preserve dst low 16
	ld   hl,(#zx0_offset)
	add  hl,de
	ld   a,(#zx0_page)
	adc  a,#0xff
	ex   de,hl                  ; DE = src_low for setVDP_DE
	or   a                      ; carry=0 → read mode
	call zx0_setVDP_DE
	in   a,(#0x98)
	ex   af,af'                 ; preserve byte read
	pop  de                     ; restore DE = dst low
	ld   a,(#zx0_page)
	scf                         ; carry=1 → write mode
	call zx0_setVDP_DE
	ex   af,af'
	out  (#0x98),a
	inc  de                     ; advance dst tracker
	dec  bc
	ld   a,b
	or   c
	jr   nz,zx0v_lvv_loop
zx0v_lvv_done:
	pop  hl                     ; restore src RAM ptr
	pop  af
	; --- end inlined ---
	add  a,a                    ; copy from literals or new offset?
	jr   nc,zx0v_literals

zx0v_new_offset:
	ld   c,#0xfe                ; prepare negative offset
	call zx0v_elias_loop        ; obtain offset MSB
	inc  c
	jp   z,zx0v_done            ; check end marker
	ld   b,c
	ld   c,(hl)                 ; obtain offset LSB
	inc  hl
	rr   b                      ; last offset bit becomes first length bit
	rr   c
	ld   (#zx0_offset),bc       ; preserve new offset (negative 16-bit)
	ld   bc,#1                  ; obtain length
	call nc,zx0v_elias_backtrack
	inc  bc
	jr   zx0v_copy

zx0v_done:
	pop  iy
	pop  hl
	pop  de
	pop  bc
	ret


; -----------------------------------------------------------------------------
; Elias gamma helpers (identical to RAM standard decoder).
; A = bit accumulator; HL = source RAM ptr; BC = length (for elias) or
; offset working register (for elias_loop / elias_backtrack).
; -----------------------------------------------------------------------------
zx0v_elias:
	inc  c
zx0v_elias_loop:
	add  a,a
	jr   nz,zx0v_elias_skip
	ld   a,(hl)                 ; load another group of 8 bits
	inc  hl
	rla
zx0v_elias_skip:
	ret  c
zx0v_elias_backtrack:
	add  a,a
	rl   c
	rl   b
	jr   zx0v_elias_loop


; -----------------------------------------------------------------------------
; zx0_setVDP_DE — unified VDP VRAM access setup (write or read mode).
; Replaces what would otherwise be two near-identical helpers.
;   In:  A = page byte (bit 16 of VRAM addr; rest accumulated below)
;        DE = low 16 bits of VRAM addr
;        Carry: 1=WRITE mode, 0=READ mode
;   Preserved: HL, BC, DE.
;   Trashes:   A.
; -----------------------------------------------------------------------------
zx0_setVDP_DE:
	push bc                     ; preserve caller BC (we use B as OR mask)
	ld   b,#0                   ; READ mode: OR mask = 0
	jr   nc,zx0_setVDP_common
	ld   b,#0b01000000          ; WRITE mode: OR mask = bit 6
zx0_setVDP_common:
	push de
	rlc  d
	rla
	rlc  d
	rla
	srl  d
	srl  d
	di
	out  (#0x99),a              ; R#14 data byte
	ld   a,#128+14
	out  (#0x99),a              ; select R#14
	ld   a,e
	nop
	out  (#0x99),a              ; addr bits 0-7
	ld   a,d
	or   b                      ; OR with mode mask (0 or 0x40)
	ei
	out  (#0x99),a              ; addr bits 8-13 | mode bit
	pop  de
	pop  bc
	ret


	.area _DATA

zx0_page:
	.ds  1
zx0_offset:
	.ds  2
