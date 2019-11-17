; RLE decompression to VRAM (for MSX2)
; NataliaPC
;
; 0x00  2bytes  Data length (not all versions)
; 0x01  1byte   Mark byte for repeat and eof
; 0x02  ...     Compressed data stream
;       ...
; n-2   2bytes  Mark+$00 (EOF mark)
;
;	Tokens:
;		Mark+nn (nn==0) EOF mark (2 bytes)
;		        (nn==1) Skip output bytes: Mark+$01+bytesToSkip (3 bytes)
;		        (nn>3)  Compressed values: Mark+size+repeatedValue (3 bytes)
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
		ld      (#.vram_page),a
		push    de
		call    setVDP_Write
		pop     de

		ld      c, #0x98
;		inc     hl              ; saltamos tamaño de los datos descomprimidos
;		inc     hl
		ld      a,(hl)          ; coge mark
		ld      (#.mark),a
		inc     hl

.dec_loop:
		ld      a,(#.mark)      ; Leemos 1 byte
.dec_loop2:
		cp      (hl)
		jr      z,.rle_uncomp   ; si byte = mark => está comprimido o skip o eof
		inc     de
		outi                    ; Copiamos 1 byte en crudo
		jr      .dec_loop2      ; Repetimos el bucle

.rle_uncomp:                    ; bucle para descompresión RLE
		inc     hl              ; Nos colocamos en el valor
		xor     a               ; Si mark+$00, es eof y terminamos
		cp      (hl)
		ret     z
		inc     a				; Si mark+$01, skip output bytes
		cp      (hl)
		jr      z,.skip_output
		ld      b,(hl)          ; B= numero de repeticiones
		inc     hl
		ld      a,(hl)          ; A= valor a repetir
		inc     hl              ; pasamos al siguiente byte
.rle_loop:
		out     (0x98),a        ; Bucle de escritura B veces
		inc     de
		djnz    .rle_loop
		jr      .dec_loop

.skip_output:
		inc     hl
		ld      a,(hl)			; bytes to skip
		inc     hl
		add     a,e				; add A to DE
		ld      e,a
		adc     a,d
		sub     e
		ld      d,a
		ld      a,(#.vram_page)	; set write VRAM pointer to A+DE address
		push    de
		push    bc
		call    setVDP_Write
		pop     bc
		pop     de
		jr		.dec_loop

.vram_page:
		.ds     1
.mark:
		.ds     1
