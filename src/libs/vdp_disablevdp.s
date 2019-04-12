;===============================================================================
; Dis/Enable BL Blank screen using VDP(1) reg
; In:  -
; Out: -
; Chg: A HL DE
; http://map.grauw.nl/articles/vdp_commands_speed.php
;
.include "vdp.s"

_disableVDP::
        ld      a,(RG0SAV+1)     ; VDPReg1
        and     #0b10111111
        jr      .enablevdp_cont

_enableVDP::
        ld      a,(RG0SAV+1)     ; VDPReg1
        or      #0b01000000
.enablevdp_cont:
        ld      b,a
        ld      c,#0x01
        call    setRegVDP
        ret
