; RLE decompression to VRAM (MSX2)
; NataliaPC
;
; 0x00  2bytes  Data length
; 0x01  1byte   Mark byte for repeat and eof
; 0x02  ...     Compressed data stream
;       ...
; n-2   2bytes  Mark+$00 (EOF mark)
;
; unRLE_vram length:  31 bytes
; _unRLE_vram length: 40 bytes
;

.area _CODE

.globl setVDP_Write


_unRLE_vram::	; Entry point from C: void unRLE_vram(char *srcmem, uint32_t vram);
		pop af
		pop hl
		pop de
		pop bc
		push bc
		push de
		push hl
		push af
		ld  a,c

unRLE_vram::	; Entry point from ASM: A+DE=VRAM_targer HL=RAM_source
		call    setVDP_Write

		ld      c, #0x98
		inc     hl              ; saltamos tamaño de los datos descomprimidos
		inc     hl
		ld      d,(hl)          ; coge mark
		inc     hl
.dec_loop:
		ld      a,(hl)          ; Leemos 1 byte
		cp      d
		jr      z,.rle_dec      ; si byte = mark => está comprimido o eof
		outi                    ; Copiamos 1 byte en crudo
		jr      .dec_loop       ; Repetimos el bucle
.rle_dec:                       ; bucle para descompresión RLE
		inc     hl              ; Nos colocamos en el valor
		ld      a,(hl)          ; Si mark+$00, es eof
		or      a
		ret     z
		ld      b,a             ; B= numero de repeticiones
		inc     hl
		ld      a,(hl)          ; A= valor a repetir
.rle_loop:
		out     (c),a           ; Bucle de escritura B veces
		djnz    .rle_loop
		inc     hl              ; pasamos al siguiente byte
		jr      .dec_loop       ; y lo procesamos
