; zx7.s
; Decompression in VRAM version by Daniel Bienvenu
; based in DZX7 "Turbo"
;
; Adapted to MSX by Natalia Pujol
; Size: 131 bytes

	; global from this code
	;================
	; void dzx7vram (void *data, unsigned vram_offset);
	;.globl dzx7vram ;  HL = ADDR. TO COMPRESSED DATA , DE = DESTINATION IN VRAM
	
	
_dzx7vram::
	pop af
	pop de
	pop hl
	push hl
	push de
	push af

dzx7vram::
	; Set Write in VRAM at DE
	ld	a,e
	out	(0x99),a
	ld	a,d
	or	#0x40
	out	(0x99),a

	ld	a,#0x80

; copy literal byte
zx7_copy_byte_loop:
	ld	c,#0x98
	outi
	inc	de
zx7_main_loop:
	call	zx7_getbit 				; check next bit
	jr	nc,zx7_copy_byte_loop

; determine number of bits used for length (Elias gamma coding)
	push    de
	ld      bc, #1
	ld      d, b
zx7_len_size_loop:
	inc     d
	call	zx7_getbit 				; check next bit
	jr      nc, zx7_len_size_loop
	jp      zx7_len_value_start

zx7_len_value_loop:
	call	zx7_getbit 				; check next bit
	rl      c
	rl      b
	jr      c, zx7_exit 			; check end marker
zx7_len_value_start:
	dec     d
	jr      nz, zx7_len_value_loop
	inc     bc 						; adjust length

; determine offset
	ld      e, (hl) 				; load offset flag (1 bit) + offset value (7 bits)
	inc     hl
	.db	#0xcb, #0x33 				; opcode for undocumented instruction "SLL E" aka "SLS E"
	jr      nc, zx7_offset_end 		; if offset flag is set, load 4 extra bits
	call	zx7_getbit 				; check next bit
	rl      d 						; insert first bit into D
	call	zx7_getbit 				; check next bit
	rl      d 						; insert second bit into D
	call	zx7_getbit 				; check next bit
	rl      d 						; insert third bit into D
	call	zx7_getbit 				; check next bit
	ccf
	jr      c, zx7_offset_end
	inc     d 						; equivalent to adding 128??? to DE   ??? NO!
zx7_offset_end:
	rr      e 						; insert inverted fourth bit into E

; copy previous sequence
	ex      (sp), hl 				; store source, restore destination
	push    hl 						; store destination
	sbc     hl, de 					; HL = source = destination - offset - 1
	pop     de 						; DE = destination
	; BC = count
	; COPY BYTES
	ex	af,af'
	set	6,d
	di 								; probando a desactivar interrupciones para evitar fallos
zx7_copybytes_loop:
	push	bc
	ld	c,#0x99
	out	(c),l
; nop                                 ;;!!!!!!!!!!!!!!NOP añadido por aviso del OpenMSX (toggle_vdp_access_test)
; nop
; nop
	out	(c),h
	inc	hl
; nop                                 ;;!!!!!!!!!!!!!!NOP añadido por aviso del OpenMSX (toggle_vdp_access_test)
; nop
	in	a,(0x98)
	out	(c),e
; nop                                 ;;!!!!!!!!!!!!!!NOP añadido por aviso del OpenMSX (toggle_vdp_access_test)
; nop
; nop
	out	(c),d
; nop                                 ;;!!!!!!!!!!!!!!NOP añadido por aviso del OpenMSX (toggle_vdp_access_test)
; nop
	inc	de
; nop                                 ;;!!!!!!!!!!!!!!NOP añadido por aviso del OpenMSX (toggle_vdp_access_test)
; nop
	out	(0x98),a
	pop	bc
	dec	bc
	ld	a,b
	or	c
	jr	nz,zx7_copybytes_loop
	ei 								; volviendo a activar las interrupciones
	res	6,d
	ex	af,af'
	
zx7_exit:
	pop     hl 						; restore source address (compressed data)
	jp      nc, zx7_main_loop
	ret

zx7_getbit:
	add	a,a
  	ret	nz
	ld	a,(hl)
	inc	hl
	rla
	ret
