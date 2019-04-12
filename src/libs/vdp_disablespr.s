;===============================================================================
; Dis/Enable sprites using VDP(8) reg
; In:  -
; Out: -
; Chg: A HL DE
; http://map.grauw.nl/articles/vdp_commands_speed.php
;
.include "vdp.s"

_disableSPR::
disableSPR::
        ld      a,(RG8SAV+0)     ; VDPReg8
        or      #0b00000010
        jr      .enablespr_cont

_enableSPR::
enableSPR::
        ld      a,(RG8SAV+0)     ; VDPReg8
        and     #0b11111101
.enablespr_cont:
        ld      b,a
        ld      c,#0x08
        call    setRegVDP8
        ret
