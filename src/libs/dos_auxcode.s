;
; Code to be used by several library functions
;
SYSFCB = 0x5c


; Fills with 0x00 the FCB and with 0x20(' ') the filename
; REGs changed: A, BC, DE, HL
dos_initializeFCB::
    ld      hl,#SYSFCB            ; Initialize FCB with zeroes
    xor     a
    ld      (hl),a
    ld      de,#SYSFCB+1
    ld      bc,#36
    ldir

    ld      hl,#SYSFCB+1          ; Initialize FCB filename with ' '
    ld      a,#' '
    ld      (hl),a
    ld      de,#SYSFCB+2
    ld      bc,#10
    ldir

	ret


; Copy a filename (in DE) to FCB in 8+3 chars format
; REGs changed: A DE HL
dos_copyFilenameToFCB::
    ld      hl,#SYSFCB+1          ; Copy filename to FCB
.open_loop:
    ld      a,(de)
    cp      #'.'
    jr      z,.open_dot
    or      a
    jr      z,.open_cont
    ld      (hl),a
    inc     hl
    inc     de
    jr      .open_loop
.open_dot:
    inc     de
    ld      hl,#SYSFCB+9
    jr      .open_loop
.open_cont:
	ret
