;===============================================================================
; Change to 50Hz or 60Hz using VDP(9) reg
; In:  -
; Out: -
; Chg: A HL DE
; http://map.grauw.nl/articles/vdp_commands_speed.php
;
.include "vdp.s"

_enable50Hz::
enable50Hz::
        ld      a,(RG8SAV+1)     ; VDPReg9
        or      #0b00000010
        jr      .enablehz_cont

_enable60Hz::
enable60Hz::
        ld      a,(RG8SAV+1)     ; VDPReg9
        and     #0b11111101

.enablehz_cont:
        ld      b,a
        ld      c,#0x09
        call    setRegVDP8
        ret
