; RLE decompression for Z80
; NataliaPC
;
; 0x00  2bytes  Data length
; 0x01  1byte   Mark byte for repeat and eof
; 0x02  ...     Compressed data stream
;       ...
; n-2   2bytes  Mark+$00 (EOF mark)
;
; unRLE length:  27 bytes
; _unRLE length: 33 bytes
;

.area _CODE

	
_unRLE::	; Entry point from C: void unRLE(char *source, char *target);
		pop af
		pop hl
		pop de
		push de
		push hl
		push af

unRLE::		; Entry point from ASM
;		inc     hl				; skip output size
;		inc     hl
		ld      c,(hl)          ; coge mark
		inc     hl
.dec_loop:
		ld      a,(hl)          ; Leemos 1 byte
		cp      c
		jr      z,.rle_dec      ; si byte = mark => está comprimido o eof
		ldi
		inc     bc              ; Copiamos 1 byte en crudo
		jr      .dec_loop       ; Repetimos el bucle
.rle_dec:                       ; bucle para descompresión RLE
		inc     hl              ; Nos colocamos en el valor
		ld      a,(hl)          ; Si mark+$00, es eof
		or      a
		ret     z
		ld      b,a             ; B= numero de repeticiones
		inc     hl
		ld      a,(hl)          ; A= valor a repetir
.bucle:
		ld      (de),a          ; \
		inc     de              ;  Bucle de escritura B veces
		djnz    .bucle          ; /
		inc     hl              ; pasamos al siguiente byte
		jr      .dec_loop       ; y lo procesamos
