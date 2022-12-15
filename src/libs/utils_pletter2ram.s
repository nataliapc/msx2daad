; pletter v0.5c msx unpacker

; call unpack with hl pointing to some pletter5 data, and de pointing to the destination.
; changes all registers

; define lengthindata when the original size is written in the pletter data
; define LENGTHINDATA

;  module pletter

;  macro GETBIT
;  add a,a
;  call z,getbit
;  endmacro

;  macro GETBITEXX
;  add a,a
;  call z,getbitexx
;  endmacro

    .globl  _pletter2ram
    ; void pletter2ram(unsigned char* datain, unsigned int dataout);
 
_pletter2ram::

	push ix
	ld ix,#4
	add ix,sp
	push af
	push bc
	push de
	push hl
	push iy
	ld l, 0(ix)
	ld h, 1(ix)
	ld e, 2(ix)
	ld d, 3(ix)
	call unpack
	pop iy
	pop hl
	pop de
	pop bc
	pop af
	pop ix
	ret

unpack:
 ; 	 ifdef LENGTHINDATA
 ;   inc hl
 ;   inc hl
 ;   endif

	ld a,(hl)
	inc hl
	exx
	ld de,#0
	add a,a
	inc a
	rl e
	add a,a
	rl e
	add a,a
	rl e
	rl e
	ld hl,#modes
	add hl,de
	ld e,(hl)
	.db 0xdd, 0x6b ;ld ixl,e
	inc hl
	ld e,(hl)
	.db 0xdd, 0x63 ;ld ixh,e
	ld e,#1
	exx
	ld iy,#loop
literal:
  	ldi
loop:
	add a,a
	call z,getbit
	jr nc,literal
	exx
	ld h,d
	ld l,e
getlen:
	add a,a
	call z,getbitexx
	jr nc,.lenok
.lus:
	add a,a
	call z,getbitexx
	adc hl,hl
	ret c
	add a,a
	call z,getbitexx
	jr nc,.lenok
	add a,a
	call z,getbitexx
	adc hl,hl
	ret c
	add a,a
	call z,getbitexx
	jp c,.lus
.lenok:
	inc hl
	exx
	ld c,(hl)
	inc hl
	ld b,#0
	bit 7,c
	jp z,offsok
	jp (ix)

mode6:
	add a,a
	call z,getbit
	rl b
mode5:
	add a,a
	call z,getbit
	rl b
mode4:
	add a,a
	call z,getbit
	rl b
mode3:
	add a,a
	call z,getbit
	rl b
mode2:
	add a,a
	call z,getbit
	rl b
	add a,a
	call z,getbit
	jr nc,offsok
	or a
	inc b
	res 7,c
offsok:
	inc bc
	push hl
	exx
	push hl
	exx
	ld l,e
	ld h,d
	sbc hl,bc
	pop bc
	ldir
	pop hl
	jp (iy)

getbit:
	ld a,(hl)
	inc hl
	rla
	ret

getbitexx:
	exx
	ld a,(hl)
	inc hl
	exx
	rla
	ret

modes:
	.word offsok
	.word mode2
	.word mode3
	.word mode4
	.word mode5
	.word mode6

;  	 endmodule

;eof