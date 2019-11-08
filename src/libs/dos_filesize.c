#include "dos.h"
#include "utils.h"

#include <stdio.h>


#ifdef MSXDOS2

uint16_t filesize(char *fn)
{

	uint16_t fp = fopen(fn, O_RDONLY);
	uint32_t len;
	char *error;
	if (fp & 0xff00) {
		len = 0;
		if ((uint8_t)fp != 0xd7) {
			explain(error, (uint8_t)fp);
			die(error);
		}
	} else {
		len = fseek(fp, 0, SEEK_END);
		fclose(fp);
	}
	return (uint16_t)len;
}

#else //MSXDOS1 (FCB)

uint16_t filesize(char *fn) __naked
{
  fn;

  __asm
    call    dos_initializeFCB

    push    ix                    ; Put filename in DE
    ld      ix,#4
    add     ix,sp
    ld      e,0(ix)
    ld      d,1(ix)

    call    dos_copyFilenameToFCB

    ld      de,#SYSFCB
    ld      c,#FOPEN               ; Call FOPEN Bios function
    DOSCALL

    or a                          ; Return result
    jr z,fsize_noerror$
    ld hl, #0xffd7                ; Set return value ERR
    jp fsize_cont$
  fsize_noerror$:
    ld hl,(SYSFCB+16)             ; Set return value OK with filesize
    push hl
    call _fclose
    pop hl

  fsize_cont$:
    pop ix
    ret
  __endasm;
}

#endif
