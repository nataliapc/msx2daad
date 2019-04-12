;===============================================================================
; Use 192 or 212 lines using VDP(9) reg
; In:  -
; Out: -
; Chg: A HL DE
; http://map.grauw.nl/articles/vdp_commands_speed.php
;
.include "vdp.s"

_enable212lines::
enable212lines::
        ld      a,(RG8SAV+1)     ; VDPReg9
        or      #0b10000000
        jr      .enablelines_cont

_enable192lines::
enable192lines::
        ld      a,(RG8SAV+1)     ; VDPReg9
        and     #0b01111111
.enablelines_cont:
        ld      b,a
        ld      c,#0x09
        call    setRegVDP8
        ret

