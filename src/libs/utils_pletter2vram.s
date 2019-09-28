; pletter2vram.s
;-----------------------------------------------------------
; Pletter2 v0.5b VRAM Depacker v1.1 - 16Kb version
; by metalion@orange.fr
; HL = RAM/ROM source ; DE = VRAM destination
;-----------------------------------------------------------
; Daniel Bienvenu modifications
; + to optimize the ROM size, not the speed
; http://www.gamopat-forum.com/t75544-termine-pang-msx
; Size: 244 bytes
;-----------------------------------------------------------
; Adapter to MSX2 by Natalia Pujol (VRAM Page 0 only)

 ; global from this code

    .globl  _pletter2vram
    ; void pletter2vram(unsigned char* data, unsigned int vram_offset);
 

_pletter2vram::
    push ix
    ld ix,#4
    add ix,sp
    ld l, 0(ix)
    ld h, 1(ix)
    ld e, 2(ix)
    ld d, 3(ix)
    push af
    push bc
    push de
    push hl
    push iy

; VRAM address setup
	ld	a,d
	rlca
	rlca
	and	#0b00000011
    di
	out	(0x99),a	; VRAM address A14-A15
	ld	a,#128+14
	out	(0x99),a	; VRAM access base address register
    ld	a,e
	out	(0x99),a	; VRAM a0-7  
	ld	a,d
	and	#0b00111111
	or	#0b01000000	; Write
    ei
	out	(0x99),a	; VRAM a8-13

; Initialization
    ld a,(hl)
    inc hl
    exx
    ld de,#0
    push    de
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
    inc hl
    ld  d,(hl)
    push    de
    pop ix
    pop de
    ld e,#1
    exx
    ;ld iy,#loop

; Main depack loop
literal:
    ld c,#0x98
    outi
    inc de
loop:
    call    getbits
    jr nc,literal

; Compressed data
    exx
    ld h,d
    ld l,e
getlen:
    call getbitsexx
    jr nc,lenok
lus:
    call getbitsexx
    adc hl,hl
    ret c
    call getbitsexx
    jr nc,lenok
    call getbitsexx
    adc hl,hl
    jp c,Depack_out
    call getbitsexx
    jp c,lus
lenok:
    inc hl
    exx
    ld c,(hl)
    inc hl
    ld b,#0
    bit 7,c
    jp z,offsok
    jp (ix)

mode7:
    call    getbits
    rl b
mode6:
    call    getbits
    rl b
mode5:
    call    getbits
    rl b
mode4:
    call    getbits
    rl b
mode3:
    call    getbits
    rl b
mode2:
    call    getbits
    rl b
    call    getbits
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
    push af
$9:
	ld	a,h
	rlca
	rlca
	and	#0b11
    di
	out	(0x99),a	; VRAM address A14-A15
	ld	a,#128+14
	out	(0x99),a	; VRAM access base address register
    ld	a,l
	out	(0x99),a	; VRAM a0-7  
	ld	a,h
	and	#0b00111111	; Read
    ei
	out	(0x99),a	; VRAM a8-13
	nop
	nop
	in	a,(0x98)
    ex af,af'

	ld	a,d
	rlca
	rlca
	and	#0b11
    di
	out	(0x99),a	; VRAM address A14-A15
	ld	a,#128+14
	out	(0x99),a	; VRAM access base address register
    ld	a,e
	out	(0x99),a	; VRAM a0-7  
	ld	a,d
	and	#0b00111111
	or	#0b01000000	; Write
    ei
	out	(0x99),a	; VRAM a8-13
    ex af,af'
    out (0x98),a

    inc de
    cpi
    jp pe,$9
    ei
    pop af
    pop hl
    ;jp (iy)
    jp loop

getbitsexx:
    exx
    call    getbits
    exx
    ret

getbits:
    add a,a
    ret nz
    ld a,(hl)
    inc hl
    rla
    ret

; Depacker exit
Depack_out:
    pop iy
    pop hl
    pop de
    pop bc
    pop af
    pop ix
    ret

modes:
    .dw offsok
    .dw mode2
    .dw mode3
    .dw mode4
    .dw mode5
    .dw mode6
    .dw mode7
