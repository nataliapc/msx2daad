;===============================================================================
; In:  -
; Out: -
; Chg: A BC HL DE
;     B: $1f:page0  $3f:page1  $5f:page2  $7f:page3
;
.include "vdp.s"

_setVPage0::
setVPage0::
        ld      bc,#0x1f02
        jr      .setvpage_call

_setVPage1::
setVPage1::
        ld      bc,#0x3f02
        jr      .setvpage_call

_setVPage2::
setVPage2::
        ld      bc,#0x5f02
        jr      .setvpage_call

_setVPage3::
setVPage3::
        ld      bc,#0x7f02
.setvpage_call:
        call    setRegVDP
        ret
