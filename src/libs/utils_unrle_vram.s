; RLE decompression to VRAM (MSX2)
; NataliaPC
;
; 0x00  2bytes  Data length
; 0x01  1byte   Mark byte for repeat and eof
; 0x02  ...     Compressed data stream
;       ...
; n-2   2bytes  Mark+$00 (EOF mark)
;
.area _CODE

.globl setVDP_Write


_unRLE_vram::	; Entry point from C code: void unRLE_vram(char *srcmem, uint32_t vram);
		pop af
		pop hl
		pop de
		pop bc
		push bc
		push de
		push hl
		push af
		ld  a,c

unRLE_vram::	; Entry point from ASM code: A+DE=VRAM_target HL=RAM_source
		call    setVDP_Write

		ld      c, #0x98
;		inc     hl              ; saltamos tamaño de los datos descomprimidos
;		inc     hl
		ld      d,(hl)          ; coge mark
		inc     hl
.dec_loop:
		ld      a,(hl)          ; Leemos 1 byte
		cp      d
		jr      z,.rle_uncomp   ; si byte = mark => está comprimido o eof
		outi                    ; Copiamos 1 byte en crudo
		jr      .dec_loop       ; Repetimos el bucle
.rle_uncomp:                    ; bucle para descompresión RLE
		inc     hl              ; Nos colocamos en el valor
		xor     a               ; Si mark+$00, es eof y terminamos
		or      (hl)
		ret     z
		ld      b,(hl)          ; B= numero de repeticiones
		inc     hl
		ld      a,(hl)          ; A= valor a repetir
		inc     hl              ; pasamos al siguiente byte
.rle_loop:
		out     (0x98),a        ; Bucle de escritura B veces
		djnz    .rle_loop
		jr      .dec_loop
