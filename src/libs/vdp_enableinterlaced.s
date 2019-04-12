;===============================================================================
; Use normal or interlaced mode using VDP(9) reg
; In:  -
; Out: -
; Chg: A HL DE
; http://map.grauw.nl/articles/vdp_commands_speed.php
;
.include "vdp.s"

_enableInterlacedLines::
enableInterlacedLines::
        ld      a,(RG8SAV+1)     ; VDPReg9
        or      #0b00001000
        jr      .enableInterlacedLines_cont

_disableInterlacedLines::
disableInterlacedLines::
        ld      a,(RG8SAV+1)     ; VDPReg9
        and     #0b11110111
.enableInterlacedLines_cont:
        ld      b,a
        ld      c,#0x09
        call    setRegVDP8
        ret

