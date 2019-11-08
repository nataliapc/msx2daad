; utils_rammapper.s
;-----------------------------------------------------------
; Info about: https://www.msx.org/wiki/Memory_Mapper
; Developed by NataliaPC

FIRST_RAMSEG =	4
SET_RAMSEG_P0 =	0xFC	; Default to segment 3
SET_RAMSEG_P1 =	0xFD	; Default to segment 2
SET_RAMSEG_P2 =	0xFE	; Default to segment 1
SET_RAMSEG_P3 =	0xFF	; Default to segment 0

    .globl  _initializeMapper	; void initializeMapper();
	.globl  _setMapperPage2		; void setMapperPage2(uint8_t page);
	.globl  _restoreMapperPage2	; void restoreMapperPage2();

	.globl  _dosver

	.globl  _DOS2MAPPER
	.globl  _MAX_MAPPER_PAGES
	.globl  _PAGE2_RAMSEG
	.globl  _FIRST_RAMSEG


; void initializeMapper();
_initializeMapper::			
initializeMapper::

		call _dosver
		ld   a,l
		dec  a
		ld   (#_DOS2MAPPER),a		; Store the DOS version -1 (0:MSXDOS1 non-0:MSXDOS2 or better)
		jr   z,.im_msxdos1			; if MSX-DOS 1.x if detected we can use pagination without care
		
	.im_msxdos2:					; for MSX-DOS2 we disable RAM pagination
		xor  a						; TODO: implement MSX-DOS2 mapper functions
		ld   (#_FIRST_RAMSEG),a
		ret

	.im_msxdos1:
		ld   hl,#0x8000
		xor  a						; Set first byte of each page with their number
	.im_loop0:
		out  (SET_RAMSEG_P2),a
		ld   (hl),a
		inc  a
		jr   nz,.im_loop0

		ld   a,#255					; Check if the page exists reading their number
		ld   ix,#_MAX_MAPPER_PAGES
		ld   (ix),#0
	.im_loop1:
		out  (SET_RAMSEG_P2),a
		cp   (hl)
		jr   nz,.im_cont0
		inc  (ix)
		dec  a
		jr   nz,.im_loop1
	.im_cont0:
		add  a,#2					;  Skip First failed mapper segment & Page3 default mapper segment
		ld   (#_PAGE2_RAMSEG),a		; STORE Page2 default mapper segment
		add  a,#3					;  Skip Page2, Page1, Page0 default mapper segment2 & MSXDOS2 page
		ld   (#_FIRST_RAMSEG+0),a	; STORE First free mapper segment
		inc  a
		ld   (#_FIRST_RAMSEG+1),a
		inc  a
		ld   (#_FIRST_RAMSEG+2),a
		inc  a
		ld   (#_FIRST_RAMSEG+3),a
		dec  (ix)					;  Decrement available segments in 4 (64kb main RAM)
		dec  (ix)
		dec  (ix)
		dec  (ix)
		; Restore page 2 to default segment & Return


; void restoreMapperPage2();
_restoreMapperPage2::
restoreMapperPage2::
		ld   a,(#_PAGE2_RAMSEG)
		out  (SET_RAMSEG_P2),a
		ret


; void setMapperPage2(uint8_t page);
_setMapperPage2::
		pop  af
		pop  bc
		push bc
		push af
		ld   a,c
setMapperPage::
		out  (SET_RAMSEG_P2),a
		ret

MAPPER_JMP_TABLE:
		.dw	0

