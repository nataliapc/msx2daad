;       Arkos Tracker 2 AKM (Minimalist) player (format V0).
;       By Targhan/Arkos.
;
;       Thanks to Hicks/Vanity for two small (but relevant!) optimizations.

;       This compiles with RASM. Check the compatibility page on the Arkos Tracker 2 website, it contains a source converter to any Z80 assembler!;

;       This is a Minimalist player. Only a subset of the generic player is used. Use this player for 4k demo or other productions
;       with a tight memory limitation. However, this remains a versatile and powerful player, so it may fit any production!
;
;       Though the player is optimized in speed, it is much slower than the generic one or the AKY player.
;       With effects used at the same time, it can reach 45 scanlines on a CPC, plus some few more if you are using sound effects.
;       So it's about as fast as the Soundtrakker 128 player, but smaller and more powerful (so what are you complaining about?).
;
;       The player uses the stack for optimizations. Make sure the interruptions are disabled before it is called.
;       The stack pointer is saved at the beginning and restored at the end.
;
;       Target hardware:
;       ---------------
;       This code can target Amstrad CPC, MSX, Spectrum and Pentagon. By default, it targets Amstrad CPC.
;       Simply use one of the follow line (BEFORE this player):
;       PLY_AKM_HARDWARE_CPC = 1
;		PLY_AKM_HARDWARE_MSX = 1
;       PLY_AKM_HARDWARE_SPECTRUM = 1
;       PLY_AKM_HARDWARE_PENTAGON = 1
;       Note that the PRESENCE of this variable is tested, NOT its value.

;       Some severe optimizations of CPU/memory can be performed:
;       ---------------------------------------------------------
;       - Use the Player Configuration of Arkos Tracker 2 to generate a configuration file to be included at the beginning of this player.
;         It will disable useless features according to your songs! Check the manual for more details, or more simply the testers.

;       Sound effects:
;       --------------
;       Sound effects are disabled by default. Declare PLY_AKM_MANAGE_SOUND_EFFECTS to enable it:
;       PLY_AKM_MANAGE_SOUND_EFFECTS = 1
;       Check the sound effect tester to see how it enables it.
;       Note that the PRESENCE of this variable is tested, NOT its value.
;
;       ROM
;       ----------------------
;       To use a ROM player (no automodification, use of a small buffer to put in RAM):
;       PLY_AKM_Rom = 1
;       PLY_AKM_ROM_Buffer = #4000 (or wherever).
;       This makes the player a bit slower and slightly bigger.
;       The buffer is PLY_AKM_ROM_BufferSize bytes long (199 bytes max).
;
;       -------------------------------------------------------

		.globl PLY_AKM_INIT
		.globl PLY_AKM_PLAY
		.globl PLY_AKM_STOP


PLY_AKM_START:
	jp PLY_AKM_INIT
    jp PLY_AKM_PLAY
    jp PLY_AKM_STOP

_PLY_AKM_INIT::
PLY_AKM_INIT: ld de,#PLY_AKM_READLINE+1
    ldi
    ldi
    ld de,#PLY_AKM_PTARPEGGIOS+1
    ldi
    ldi
    ld de,#PLY_AKM_PTPITCHES+1
    ldi
    ldi
    add a,a
    ld e,a
    ld d,#0
    add hl,de
    ld a,(hl)
    inc hl
    ld h,(hl)
    ld l,a
    ld ix,#PLY_AKM_INITVARS_START
    ld a,#13
PLY_AKM_INITVARS_LOOP: ld e,+0(ix)
    ld d,+1(ix)
    inc ix
    inc ix
    ldi
    dec a
    jr nz,PLY_AKM_INITVARS_LOOP
    ld (PLY_AKM_PATTERNREMAININGHEIGHT+1),a
    ex de,hl
    ld hl,#PLY_AKM_PTLINKER+1
    ld (hl),e
    inc hl
    ld (hl),d
    ld hl,#PLY_AKM_TRACK1_DATA
    ld de,#PLY_AKM_TRACK1_TRANSPOSITION
    ld bc,#37
    ld (hl),a
    ldir
    ld a,(PLY_AKM_SPEED+1)
    dec a
    ld (PLY_AKM_TICKCOUNTER+1),a
    ld hl,(PLY_AKM_READLINE+1)
    ld e,(hl)
    inc hl
    ld d,(hl)
    inc de
    ld (PLY_AKM_TRACK1_PTINSTRUMENT),de
    ld (PLY_AKM_TRACK2_PTINSTRUMENT),de
    ld (PLY_AKM_TRACK3_PTINSTRUMENT),de
    ret 
PLY_AKM_INITVARS_START: .dw PLY_AKM_NOTEINDEXTABLE+1
    .dw PLY_AKM_NOTEINDEXTABLE+2
    .dw PLY_AKM_LINKER+1
    .dw PLY_AKM_LINKER+2
    .dw PLY_AKM_SPEED+1
    .dw PLY_AKM_RT_PRIMARYINSTRUMENT+1
    .dw PLY_AKM_RT_SECONDARYINSTRUMENT+1
    .dw PLY_AKM_RT_PRIMARYWAIT+1
    .dw PLY_AKM_RT_SECONDARYWAIT+1
    .dw PLY_AKM_DEFAULTSTARTNOTEINTRACKS+1
    .dw PLY_AKM_DEFAULTSTARTINSTRUMENTINTRACKS+1
    .dw PLY_AKM_DEFAULTSTARTWAITINTRACKS+1
    .dw PLY_AKM_FLAGNOTEANDEFFECTINCELL+1
PLY_AKM_INITVARS_END:
_PLY_AKM_STOP::
PLY_AKM_STOP: ld (PLY_AKM_SENDPSGREGISTEREND+1),sp
    xor a
    ld (PLY_AKM_TRACK1_VOLUME),a
    ld (PLY_AKM_TRACK2_VOLUME),a
    ld (PLY_AKM_TRACK3_VOLUME),a
    ld a,#191
    ld (PLY_AKM_MIXERREGISTER),a
    jp PLY_AKM_SENDPSG
_PLY_AKM_PLAY::
PLY_AKM_PLAY: ld (PLY_AKM_SENDPSGREGISTEREND+1),sp
PLY_AKM_TICKCOUNTER: ld a,#0
    inc a
PLY_AKM_SPEED: cp #1
    jp nz,PLY_AKM_TICKCOUNTERMANAGED
PLY_AKM_PATTERNREMAININGHEIGHT: ld a,#0
    sub #1
    jr c,PLY_AKM_LINKER
    ld (PLY_AKM_PATTERNREMAININGHEIGHT+1),a
    jr PLY_AKM_READLINE
PLY_AKM_LINKER:
PLY_AKM_TRACKINDEX: ld de,#0
    exx
PLY_AKM_PTLINKER: ld hl,#0
PLY_AKM_LINKERPOSTPT: xor a
    ld (PLY_AKM_TRACK1_DATA),a
    ld (PLY_AKM_TRACK1_DATA_END),a
    ld (PLY_AKM_TRACK2_DATA_END),a
PLY_AKM_DEFAULTSTARTNOTEINTRACKS: ld a,#0
    ld (PLY_AKM_TRACK1_ESCAPENOTE),a
    ld (PLY_AKM_TRACK2_ESCAPENOTE),a
    ld (PLY_AKM_TRACK3_ESCAPENOTE),a
PLY_AKM_DEFAULTSTARTINSTRUMENTINTRACKS: ld a,#0
    ld (PLY_AKM_TRACK1_ESCAPEINSTRUMENT),a
    ld (PLY_AKM_TRACK2_ESCAPEINSTRUMENT),a
    ld (PLY_AKM_TRACK3_ESCAPEINSTRUMENT),a
PLY_AKM_DEFAULTSTARTWAITINTRACKS: ld a,#0
    ld (PLY_AKM_TRACK1_ESCAPEWAIT),a
    ld (PLY_AKM_TRACK2_ESCAPEWAIT),a
    ld (PLY_AKM_TRACK3_ESCAPEWAIT),a
    ld b,(hl)
    inc hl
    rr b
    jr nc,PLY_AKM_LINKERAFTERSPEEDCHANGE
    ld a,(hl)
    inc hl
    or a
    jr nz,PLY_AKM_LINKERSPEEDCHANGE
    ld a,(hl)
    inc hl
    ld h,(hl)
    ld l,a
    jr PLY_AKM_LINKERPOSTPT
PLY_AKM_LINKERSPEEDCHANGE: ld (PLY_AKM_SPEED+1),a
PLY_AKM_LINKERAFTERSPEEDCHANGE: rr b
    jr nc,PLY_AKM_LINKERUSEPREVIOUSHEIGHT
    ld a,(hl)
    inc hl
    ld (PLY_AKM_LINKERUSEPREVIOUSHEIGHT+1),a
    jr PLY_AKM_LINKERSETREMAININGHEIGHT
PLY_AKM_LINKERUSEPREVIOUSHEIGHT:
PLY_AKM_LINKERPREVIOUSREMAININGHEIGHT: ld a,#0
PLY_AKM_LINKERSETREMAININGHEIGHT: ld (PLY_AKM_PATTERNREMAININGHEIGHT+1),a
    ld ix,#PLY_AKM_TRACK1_DATA
    call PLY_AKM_CHECKTRANSPOSITIONANDTRACK
    ld ix,#PLY_AKM_TRACK1_DATA_END
    call PLY_AKM_CHECKTRANSPOSITIONANDTRACK
    ld ix,#PLY_AKM_TRACK2_DATA_END
    call PLY_AKM_CHECKTRANSPOSITIONANDTRACK
    ld (PLY_AKM_PTLINKER+1),hl
PLY_AKM_READLINE:
PLY_AKM_PTINSTRUMENTS: ld de,#0
PLY_AKM_NOTEINDEXTABLE: ld bc,#0
    exx
    ld ix,#PLY_AKM_TRACK1_DATA
    call PLY_AKM_READTRACK
    ld ix,#PLY_AKM_TRACK1_DATA_END
    call PLY_AKM_READTRACK
    ld ix,#PLY_AKM_TRACK2_DATA_END
    call PLY_AKM_READTRACK
    xor a
PLY_AKM_TICKCOUNTERMANAGED: ld (PLY_AKM_TICKCOUNTER+1),a
    ld de,#PLY_AKM_PERIODTABLE
    exx
    ld c,#224
    ld ix,#PLY_AKM_TRACK1_DATA
    call PLY_AKM_MANAGEEFFECTS
    ld iy,#PLY_AKM_TRACK3_DATA_END
    call PLY_AKM_PLAYSOUNDSTREAM
    srl c
    ld ix,#PLY_AKM_TRACK1_DATA_END
    call PLY_AKM_MANAGEEFFECTS
    ld iy,#PLY_AKM_TRACK2_REGISTERS
    call PLY_AKM_PLAYSOUNDSTREAM
    scf
    rr c
    ld ix,#PLY_AKM_TRACK2_DATA_END
    call PLY_AKM_MANAGEEFFECTS
    ld iy,#PLY_AKM_TRACK3_REGISTERS
    call PLY_AKM_PLAYSOUNDSTREAM
    ld a,c
    ld (PLY_AKM_MIXERREGISTER),a
PLY_AKM_SENDPSG: ld sp,#PLY_AKM_TRACK3_DATA_END
PLY_AKM_SENDPSGREGISTER: pop hl
PLY_AKM_SENDPSGREGISTERAFTERPOP: ld a,l
    out (160),a
    ld a,h
    out (161),a
    ret 
PLY_AKM_SENDPSGREGISTERR13:
PLY_AKM_SETREG13: ld a,#0
PLY_AKM_SETREG13OLD: cp #0
    jr z,PLY_AKM_SENDPSGREGISTEREND
    ld (PLY_AKM_SETREG13OLD+1),a
    ld h,a
    ld l,#13
    ret 
PLY_AKM_SENDPSGREGISTEREND:
PLY_AKM_SAVESP: ld sp,#0
    ret 
PLY_AKM_CHECKTRANSPOSITIONANDTRACK: rr b
    jr nc,PLY_AKM_CHECKTRANSPOSITIONANDTRACK_AFTERTRANSPOSITION
    ld a,(hl)
    ld +1(ix),a
    inc hl
PLY_AKM_CHECKTRANSPOSITIONANDTRACK_AFTERTRANSPOSITION: rr b
    jr nc,PLY_AKM_CHECKTRANSPOSITIONANDTRACK_NONEWTRACK
    ld a,(hl)
    inc hl
    sla a
    jr nc,PLY_AKM_CHECKTRANSPOSITIONANDTRACK_TRACKOFFSET
    exx
    ld l,a
    ld h,#0
    add hl,de
    ld a,(hl)
    ld +2(ix),a
    ld +4(ix),a
    inc hl
    ld a,(hl)
    ld +3(ix),a
    ld +5(ix),a
    exx
    ret 
PLY_AKM_CHECKTRANSPOSITIONANDTRACK_TRACKOFFSET: rra 
    ld d,a
    ld e,(hl)
    inc hl
    ld c,l
    ld a,h
    add hl,de
    .db 221
    .db 117
    .db +2
    .db 221
    .db 116
    .db +3
    .db 221
    .db 117
    .db +4
    .db 221
    .db 116
    .db +5
    ld l,c
    ld h,a
    ret 
PLY_AKM_CHECKTRANSPOSITIONANDTRACK_NONEWTRACK: ld a,+2(ix)
    ld +4(ix),a
    ld a,+3(ix)
    ld +5(ix),a
    ret 
PLY_AKM_READTRACK: ld a,+0(ix)
    sub #1
    jr c,PLY_AKM_RT_NOEMPTYCELL
    ld +0(ix),a
    ret 
PLY_AKM_RT_NOEMPTYCELL: ld l,+4(ix)
    ld h,+5(ix)
PLY_AKM_RT_GETDATABYTE: ld b,(hl)
    inc hl
    ld a,b
    and #15
PLY_AKM_FLAGNOTEANDEFFECTINCELL: cp #12
    jr c,PLY_AKM_RT_NOTEREFERENCE
    sub #12
    jr z,PLY_AKM_RT_NOTEANDEFFECTS
    dec a
    jr z,PLY_AKM_RT_NONOTEMAYBEEFFECTS
    dec a
    jr z,PLY_AKM_RT_NEWESCAPENOTE
    ld a,+7(ix)
    jr PLY_AKM_RT_AFTERNOTEREAD
PLY_AKM_RT_NEWESCAPENOTE: ld a,(hl)
    ld +7(ix),a
    inc hl
    jr PLY_AKM_RT_AFTERNOTEREAD
PLY_AKM_RT_NOTEANDEFFECTS: dec a
    ld (PLY_AKM_RT_READEFFECTSFLAG+1),a
    jr PLY_AKM_RT_GETDATABYTE
PLY_AKM_RT_NONOTEMAYBEEFFECTS: bit 4,b
    jr z,PLY_AKM_RT_READWAITFLAGS
    ld a,b
    ld (PLY_AKM_RT_READEFFECTSFLAG+1),a
    jr PLY_AKM_RT_READWAITFLAGS
PLY_AKM_RT_NOTEREFERENCE: exx
    ld l,a
    ld h,#0
    add hl,bc
    ld a,(hl)
    exx
PLY_AKM_RT_AFTERNOTEREAD: add a,+1(ix)
    ld +6(ix),a
    ld a,b
    and #48
    jr z,PLY_AKM_RT_SAMEESCAPEINSTRUMENT
    cp #16
    jr z,PLY_AKM_RT_PRIMARYINSTRUMENT
    cp #32
    jr z,PLY_AKM_RT_SECONDARYINSTRUMENT
    ld a,(hl)
    inc hl
    ld +8(ix),a
    jr PLY_AKM_RT_STORECURRENTINSTRUMENT
PLY_AKM_RT_SAMEESCAPEINSTRUMENT: ld a,+8(ix)
    jr PLY_AKM_RT_STORECURRENTINSTRUMENT
PLY_AKM_RT_SECONDARYINSTRUMENT:
PLY_AKM_SECONDARYINSTRUMENT: ld a,#0
    jr PLY_AKM_RT_STORECURRENTINSTRUMENT
PLY_AKM_RT_PRIMARYINSTRUMENT:
PLY_AKM_PRIMARYINSTRUMENT: ld a,#0
PLY_AKM_RT_STORECURRENTINSTRUMENT: exx
    add a,a
    ld l,a
    ld h,#0
    add hl,de
    ld a,(hl)
    inc hl
    ld h,(hl)
    ld l,a
    ld a,(hl)
    inc hl
    ld +13(ix),a
    .db 221
    .db 117
    .db +10
    .db 221
    .db 116
    .db +11
    exx
    xor a
    ld +12(ix),a
    ld +15(ix),a
    ld +16(ix),a
    ld +17(ix),a
    ld +24(ix),a
    ld +25(ix),a
    ld a,+27(ix)
    ld +26(ix),a
    ld +32(ix),a
    ld +33(ix),a
    ld a,+35(ix)
    ld +34(ix),a
PLY_AKM_RT_READWAITFLAGS: ld a,b
    and #192
    jr z,PLY_AKM_RT_SAMEESCAPEWAIT
    cp #64
    jr z,PLY_AKM_RT_PRIMARYWAIT
    cp #128
    jr z,PLY_AKM_RT_SECONDARYWAIT
    ld a,(hl)
    inc hl
    ld +9(ix),a
    jr PLY_AKM_RT_STORECURRENTWAIT
PLY_AKM_RT_SAMEESCAPEWAIT: ld a,+9(ix)
    jr PLY_AKM_RT_STORECURRENTWAIT
PLY_AKM_RT_PRIMARYWAIT:
PLY_AKM_PRIMARYWAIT: ld a,#0
    jr PLY_AKM_RT_STORECURRENTWAIT
PLY_AKM_RT_SECONDARYWAIT:
PLY_AKM_SECONDARYWAIT: ld a,#0
PLY_AKM_RT_STORECURRENTWAIT: ld +0(ix),a
PLY_AKM_RT_READEFFECTSFLAG: ld a,#0
    or a
    jr nz,PLY_AKM_RT_READEFFECTS
PLY_AKM_RT_AFTEREFFECTS: .db 221
    .db 117
    .db +4
    .db 221
    .db 116
    .db +5
    ret 
PLY_AKM_RT_READEFFECTS: xor a
    ld (PLY_AKM_RT_READEFFECTSFLAG+1),a
PLY_AKM_RT_READEFFECT: ld iy,#PLY_AKM_EFFECTTABLE
    ld b,(hl)
    ld a,b
    inc hl
    and #14
    ld e,a
    ld d,#0
    add iy,de
    ld a,b
    rra 
    rra 
    rra 
    rra 
    and #15
    jp (iy)
PLY_AKM_RT_READEFFECT_RETURN: bit 0,b
    jr nz,PLY_AKM_RT_READEFFECT
    jr PLY_AKM_RT_AFTEREFFECTS
PLY_AKM_RT_WAITLONG: ld a,(hl)
    inc hl
    ld +0(ix),a
    jr PLY_AKM_RT_CELLREAD
PLY_AKM_RT_WAITSHORT: ld a,b
    rlca 
    rlca 
    and #3
    ld +0(ix),a
PLY_AKM_RT_CELLREAD: .db 221
    .db 117
    .db +4
    .db 221
    .db 116
    .db +5
    ret 
PLY_AKM_MANAGEEFFECTS: ld a,+15(ix)
    or a
    jr z,PLY_AKM_ME_PITCHUPDOWNFINISHED
    ld l,+18(ix)
    ld h,+16(ix)
    ld e,+19(ix)
    ld d,+20(ix)
    ld a,+17(ix)
    bit 7,d
    jr nz,PLY_AKM_ME_PITCHUPDOWN_NEGATIVESPEED
PLY_AKM_ME_PITCHUPDOWN_POSITIVESPEED: add hl,de
    adc a,#0
    jr PLY_AKM_ME_PITCHUPDOWN_SAVE
PLY_AKM_ME_PITCHUPDOWN_NEGATIVESPEED: res 7,d
    or a
    sbc hl,de
    sbc a,#0
PLY_AKM_ME_PITCHUPDOWN_SAVE: ld +17(ix),a
    .db 221
    .db 117
    .db +18
    .db 221
    .db 116
    .db +16
PLY_AKM_ME_PITCHUPDOWNFINISHED: ld a,+21(ix)
    or a
    jr z,PLY_AKM_ME_ARPEGGIOTABLEFINISHED
    ld e,+22(ix)
    ld d,+23(ix)
    ld l,+24(ix)
    ld h,#0
    add hl,de
    ld a,(hl)
    sra a
    ld +28(ix),a
    ld a,+25(ix)
    cp +26(ix)
    jr c,PLY_AKM_ME_ARPEGGIOTABLE_SPEEDNOTREACHED
    ld +25(ix),#0
    inc +24(ix)
    inc hl
    ld a,(hl)
    rra 
    jr nc,PLY_AKM_ME_ARPEGGIOTABLEFINISHED
    ld l,a
    ld +24(ix),a
    jr PLY_AKM_ME_ARPEGGIOTABLEFINISHED
PLY_AKM_ME_ARPEGGIOTABLE_SPEEDNOTREACHED: inc a
    ld +25(ix),a
PLY_AKM_ME_ARPEGGIOTABLEFINISHED: ld a,+29(ix)
    or a
    ret z
    ld l,+30(ix)
    ld h,+31(ix)
    ld e,+32(ix)
    ld d,#0
    add hl,de
    ld a,(hl)
    sra a
    jp p,PLY_AKM_ME_PITCHTABLEENDNOTREACHED_POSITIVE
    dec d
PLY_AKM_ME_PITCHTABLEENDNOTREACHED_POSITIVE: ld +36(ix),a
    .db 221
    .db 114
    .db +37
    ld a,+33(ix)
    cp +34(ix)
    jr c,PLY_AKM_ME_PITCHTABLE_SPEEDNOTREACHED
    ld +33(ix),#0
    inc +32(ix)
    inc hl
    ld a,(hl)
    rra 
    ret nc
    ld l,a
    ld +32(ix),a
    ret 
PLY_AKM_ME_PITCHTABLE_SPEEDNOTREACHED: inc a
    ld +33(ix),a
    ret 
PLY_AKM_PLAYSOUNDSTREAM: ld l,+10(ix)
    ld h,+11(ix)
PLY_AKM_PSS_READFIRSTBYTE: ld a,(hl)
    ld b,a
    inc hl
    rra 
    jr c,PLY_AKM_PSS_SOFTORSOFTANDHARD
    rra 
    jr c,PLY_AKM_PSS_SOFTWARETOHARDWARE
    rra 
    jr nc,PLY_AKM_PSS_NSNH_NOTENDOFSOUND
    ld a,(hl)
    inc hl
    ld h,(hl)
    ld l,a
    .db 221
    .db 117
    .db +10
    .db 221
    .db 116
    .db +11
    jr PLY_AKM_PSS_READFIRSTBYTE
PLY_AKM_PSS_NSNH_NOTENDOFSOUND: set 2,c
    call PLY_AKM_PSS_SHARED_ADJUSTVOLUME
    ld +1(iy),a
    rl b
    call c,PLY_AKM_PSS_READNOISE
    jr PLY_AKM_PSS_SHARED_STOREINSTRUMENTPOINTER
PLY_AKM_PSS_SOFTORSOFTANDHARD: rra 
    jr c,PLY_AKM_PSS_SOFTANDHARD
    call PLY_AKM_PSS_SHARED_ADJUSTVOLUME
    ld +1(iy),a
    ld d,#0
    rl b
    jr nc,PLY_AKM_PSS_S_AFTERARPANDORNOISE
    ld a,(hl)
    inc hl
    sra a
    ld d,a
    call c,PLY_AKM_PSS_READNOISE
PLY_AKM_PSS_S_AFTERARPANDORNOISE: ld a,d
    call PLY_AKM_CALCULATEPERIODFORBASENOTE
    rl b
    call c,PLY_AKM_READPITCHANDADDTOPERIOD
    exx
    ld +5(iy),l
    ld +9(iy),h
    exx
PLY_AKM_PSS_SHARED_STOREINSTRUMENTPOINTER: ld a,+12(ix)
    cp +13(ix)
    jr nc,PLY_AKM_PSS_S_SPEEDREACHED
    inc +12(ix)
    ret 
PLY_AKM_PSS_S_SPEEDREACHED: .db 221
    .db 117
    .db +10
    .db 221
    .db 116
    .db +11
    ld +12(ix),#0
    ret 
PLY_AKM_PSS_SOFTANDHARD: call PLY_AKM_PSS_SHARED_READENVBITPITCHARP_SOFTPERIOD_HARDVOL_HARDENV
    ld a,(hl)
    ld (PLY_AKM_REG11),a
    inc hl
    ld a,(hl)
    ld (PLY_AKM_REG12),a
    inc hl
    jr PLY_AKM_PSS_SHARED_STOREINSTRUMENTPOINTER
PLY_AKM_PSS_SOFTWARETOHARDWARE: call PLY_AKM_PSS_SHARED_READENVBITPITCHARP_SOFTPERIOD_HARDVOL_HARDENV
    ld a,b
    rlca 
    rlca 
    rlca 
    rlca 
    and #7
    exx
    jr z,PLY_AKM_PSS_STH_RATIOEND
PLY_AKM_PSS_STH_RATIOLOOP: srl h
    rr l
    dec a
    jr nz,PLY_AKM_PSS_STH_RATIOLOOP
    jr nc,PLY_AKM_PSS_STH_RATIOEND
    inc hl
PLY_AKM_PSS_STH_RATIOEND: ld a,l
    ld (PLY_AKM_REG11),a
    ld a,h
    ld (PLY_AKM_REG12),a
    exx
    jr PLY_AKM_PSS_SHARED_STOREINSTRUMENTPOINTER
PLY_AKM_PSS_SHARED_READENVBITPITCHARP_SOFTPERIOD_HARDVOL_HARDENV: and #2
    add a,#8
    ld (PLY_AKM_SENDPSGREGISTERR13+1),a
    ld +1(iy),#16
    xor a
    bit 7,b
    jr z,PLY_AKM_PSS_SHARED_RENVBAP_AFTERARPEGGIO
    ld a,(hl)
    inc hl
PLY_AKM_PSS_SHARED_RENVBAP_AFTERARPEGGIO: call PLY_AKM_CALCULATEPERIODFORBASENOTE
    bit 2,b
    call nz,PLY_AKM_READPITCHANDADDTOPERIOD
    exx
    ld +5(iy),l
    ld +9(iy),h
    exx
    ret 
PLY_AKM_PSS_SHARED_ADJUSTVOLUME: and #15
    sub +14(ix)
    ret nc
    xor a
    ret 
PLY_AKM_PSS_READNOISE: ld a,(hl)
    inc hl
    ld (PLY_AKM_NOISEREGISTER),a
    res 5,c
    ret 
PLY_AKM_CALCULATEPERIODFORBASENOTE: exx
    ld h,#0
    add a,+6(ix)
    add a,+28(ix)
    ld bc,#65292
PLY_AKM_FINDOCTAVE_LOOP: inc b
    sub c
    jr nc,PLY_AKM_FINDOCTAVE_LOOP
    add a,c
    add a,a
    ld l,a
    ld h,#0
    add hl,de
    ld a,(hl)
    inc hl
    ld h,(hl)
    ld l,a
    ld a,b
    or a
    jr z,PLY_AKM_FINDOCTAVE_OCTAVESHIFTLOOP_FINISHED
PLY_AKM_FINDOCTAVE_OCTAVESHIFTLOOP: srl h
    rr l
    djnz PLY_AKM_FINDOCTAVE_OCTAVESHIFTLOOP
PLY_AKM_FINDOCTAVE_OCTAVESHIFTLOOP_FINISHED: jr nc,PLY_AKM_FINDOCTAVE_FINISHED
    inc hl
PLY_AKM_FINDOCTAVE_FINISHED: ld a,+29(ix)
    or a
    jr z,PLY_AKM_CALCULATEPERIODFORBASENOTE_NOPITCHTABLE
    ld c,+36(ix)
    ld b,+37(ix)
    add hl,bc
PLY_AKM_CALCULATEPERIODFORBASENOTE_NOPITCHTABLE: ld c,+16(ix)
    ld b,+17(ix)
    add hl,bc
    exx
    ret 
PLY_AKM_READPITCHANDADDTOPERIOD: ld a,(hl)
    inc hl
    exx
    ld c,a
    exx
    ld a,(hl)
    inc hl
    exx
    ld b,a
    add hl,bc
    exx
    ret 
PLY_AKM_EFFECTRESETWITHVOLUME: ld +14(ix),a
    xor a
    ld +15(ix),a
    ld +21(ix),a
    ld +28(ix),a
    ld +29(ix),a
    jp PLY_AKM_RT_READEFFECT_RETURN
PLY_AKM_EFFECTVOLUME: ld +14(ix),a
    jp PLY_AKM_RT_READEFFECT_RETURN
PLY_AKM_EFFECTFORCEINSTRUMENTSPEED: call PLY_AKM_EFFECTREADIFESCAPE
    ld +13(ix),a
    jp PLY_AKM_RT_READEFFECT_RETURN
PLY_AKM_EFFECTFORCEPITCHSPEED: call PLY_AKM_EFFECTREADIFESCAPE
    ld +34(ix),a
    jp PLY_AKM_RT_READEFFECT_RETURN
PLY_AKM_EFFECTFORCEARPEGGIOSPEED: call PLY_AKM_EFFECTREADIFESCAPE
    ld +26(ix),a
    jp PLY_AKM_RT_READEFFECT_RETURN
PLY_AKM_EFFECTTABLE: jr PLY_AKM_EFFECTRESETWITHVOLUME
    jr PLY_AKM_EFFECTVOLUME
    jr PLY_AKM_EFFECTPITCHUPDOWN
    jr PLY_AKM_EFFECTARPEGGIOTABLE
    jr PLY_AKM_EFFECTPITCHTABLE
    jr PLY_AKM_EFFECTFORCEINSTRUMENTSPEED
    jr PLY_AKM_EFFECTFORCEARPEGGIOSPEED
    jr PLY_AKM_EFFECTFORCEPITCHSPEED
PLY_AKM_EFFECTPITCHUPDOWN: rra 
    jr nc,PLY_AKM_EFFECTPITCHUPDOWN_DEACTIVATED
    ld +15(ix),#255
    ld a,(hl)
    inc hl
    ld +19(ix),a
    ld a,(hl)
    inc hl
    ld +20(ix),a
    jp PLY_AKM_RT_READEFFECT_RETURN
PLY_AKM_EFFECTPITCHUPDOWN_DEACTIVATED: ld +15(ix),#0
    jp PLY_AKM_RT_READEFFECT_RETURN
PLY_AKM_EFFECTARPEGGIOTABLE: call PLY_AKM_EFFECTREADIFESCAPE
    ld +21(ix),a
    jr z,PLY_AKM_EFFECTARPEGGIOTABLE_STOP
    add a,a
    exx
    ld l,a
    ld h,#0
PLY_AKM_PTARPEGGIOS: ld bc,#0
    add hl,bc
    ld a,(hl)
    inc hl
    ld h,(hl)
    ld l,a
    ld a,(hl)
    inc hl
    ld +27(ix),a
    ld +26(ix),a
    .db 221
    .db 117
    .db +22
    .db 221
    .db 116
    .db +23
    exx
    xor a
    ld +24(ix),a
    ld +25(ix),a
    jp PLY_AKM_RT_READEFFECT_RETURN
PLY_AKM_EFFECTARPEGGIOTABLE_STOP: ld +28(ix),a
    jp PLY_AKM_RT_READEFFECT_RETURN
PLY_AKM_EFFECTPITCHTABLE: call PLY_AKM_EFFECTREADIFESCAPE
    ld +29(ix),a
    jp z,PLY_AKM_RT_READEFFECT_RETURN
    add a,a
    exx
    ld l,a
    ld h,#0
PLY_AKM_PTPITCHES: ld bc,#0
    add hl,bc
    ld a,(hl)
    inc hl
    ld h,(hl)
    ld l,a
    ld a,(hl)
    inc hl
    ld +35(ix),a
    ld +34(ix),a
    .db 221
    .db 117
    .db +30
    .db 221
    .db 116
    .db +31
    exx
    xor a
    ld +32(ix),a
    ld +33(ix),a
    jp PLY_AKM_RT_READEFFECT_RETURN
PLY_AKM_EFFECTREADIFESCAPE: cp #15
    ret c
    ld a,(hl)
    inc hl
    ret 
PLY_AKM_TRACK1_DATA:
PLY_AKM_TRACK1_WAITEMPTYCELL: .db 0
PLY_AKM_TRACK1_TRANSPOSITION: .db 0
PLY_AKM_TRACK1_PTSTARTTRACK: .dw 0
PLY_AKM_TRACK1_PTTRACK: .dw 0
PLY_AKM_TRACK1_BASENOTE: .db 0
PLY_AKM_TRACK1_ESCAPENOTE: .db 0
PLY_AKM_TRACK1_ESCAPEINSTRUMENT: .db 0
PLY_AKM_TRACK1_ESCAPEWAIT: .db 0
PLY_AKM_TRACK1_PTINSTRUMENT: .dw 0
PLY_AKM_TRACK1_INSTRUMENTCURRENTSTEP: .db 0
PLY_AKM_TRACK1_INSTRUMENTSPEED: .db 0
PLY_AKM_TRACK1_TRACKINVERTEDVOLUME: .db 0
PLY_AKM_TRACK1_ISPITCHUPDOWNUSED: .db 0
PLY_AKM_TRACK1_TRACKPITCHINTEGER: .dw 0
PLY_AKM_TRACK1_TRACKPITCHDECIMAL: .db 0
PLY_AKM_TRACK1_TRACKPITCHSPEED: .dw 0
PLY_AKM_TRACK1_ISARPEGGIOTABLEUSED: .db 0
PLY_AKM_TRACK1_PTARPEGGIOTABLE: .dw 0
PLY_AKM_TRACK1_PTARPEGGIOOFFSET: .db 0
PLY_AKM_TRACK1_ARPEGGIOCURRENTSTEP: .db 0
PLY_AKM_TRACK1_ARPEGGIOCURRENTSPEED: .db 0
PLY_AKM_TRACK1_ARPEGGIOORIGINALSPEED: .db 0
PLY_AKM_TRACK1_CURRENTARPEGGIOVALUE: .db 0
PLY_AKM_TRACK1_ISPITCHTABLEUSED: .db 0
PLY_AKM_TRACK1_PTPITCHTABLE: .dw 0
PLY_AKM_TRACK1_PTPITCHOFFSET: .db 0
PLY_AKM_TRACK1_PITCHCURRENTSTEP: .db 0
PLY_AKM_TRACK1_PITCHCURRENTSPEED: .db 0
PLY_AKM_TRACK1_PITCHORIGINALSPEED: .db 0
PLY_AKM_TRACK1_CURRENTPITCHTABLEVALUE: .dw 0
PLY_AKM_TRACK1_DATA_END:
PLY_AKM_TRACK2_DATA:
PLY_AKM_TRACK2_WAITEMPTYCELL: .db 0
    .db 0
    .db 0
    .db 0
PLY_AKM_TRACK2_PTTRACK: .db 0
    .db 0
    .db 0
PLY_AKM_TRACK2_ESCAPENOTE: .db 0
PLY_AKM_TRACK2_ESCAPEINSTRUMENT: .db 0
PLY_AKM_TRACK2_ESCAPEWAIT: .db 0
PLY_AKM_TRACK2_PTINSTRUMENT: .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
PLY_AKM_TRACK2_DATA_END:
PLY_AKM_TRACK3_DATA:
PLY_AKM_TRACK3_WAITEMPTYCELL: .db 0
    .db 0
    .db 0
    .db 0
PLY_AKM_TRACK3_PTTRACK: .db 0
    .db 0
    .db 0
PLY_AKM_TRACK3_ESCAPENOTE: .db 0
PLY_AKM_TRACK3_ESCAPEINSTRUMENT: .db 0
PLY_AKM_TRACK3_ESCAPEWAIT: .db 0
PLY_AKM_TRACK3_PTINSTRUMENT: .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
    .db 0
PLY_AKM_TRACK3_DATA_END:
PLY_AKM_REGISTERS_RETTABLE:
PLY_AKM_TRACK1_REGISTERS: .db 8
PLY_AKM_TRACK1_VOLUME: .db 0
    .dw PLY_AKM_SENDPSGREGISTER
    .db 0
PLY_AKM_TRACK1_SOFTWAREPERIODLSB: .db 0
    .dw PLY_AKM_SENDPSGREGISTER
    .db 1
PLY_AKM_TRACK1_SOFTWAREPERIODMSB: .db 0
    .dw PLY_AKM_SENDPSGREGISTER
PLY_AKM_TRACK2_REGISTERS: .db 9
PLY_AKM_TRACK2_VOLUME: .db 0
    .dw PLY_AKM_SENDPSGREGISTER
    .db 2
PLY_AKM_TRACK2_SOFTWAREPERIODLSB: .db 0
    .dw PLY_AKM_SENDPSGREGISTER
    .db 3
PLY_AKM_TRACK2_SOFTWAREPERIODMSB: .db 0
    .dw PLY_AKM_SENDPSGREGISTER
PLY_AKM_TRACK3_REGISTERS: .db 10
PLY_AKM_TRACK3_VOLUME: .db 0
    .dw PLY_AKM_SENDPSGREGISTER
    .db 4
PLY_AKM_TRACK3_SOFTWAREPERIODLSB: .db 0
    .dw PLY_AKM_SENDPSGREGISTER
    .db 5
PLY_AKM_TRACK3_SOFTWAREPERIODMSB: .db 0
    .dw PLY_AKM_SENDPSGREGISTER
    .db 6
PLY_AKM_NOISEREGISTER: .db 0
    .dw PLY_AKM_SENDPSGREGISTER
    .db 7
PLY_AKM_MIXERREGISTER: .db 0
    .dw PLY_AKM_SENDPSGREGISTER
    .db 11
PLY_AKM_REG11: .db 0
    .dw PLY_AKM_SENDPSGREGISTER
    .db 12
PLY_AKM_REG12: .db 0
    .dw PLY_AKM_SENDPSGREGISTERR13
    .dw PLY_AKM_SENDPSGREGISTERAFTERPOP
    .dw PLY_AKM_SENDPSGREGISTEREND
PLY_AKM_PERIODTABLE: .dw 6778
    .dw 6398
    .dw 6039
    .dw 5700
    .dw 5380
    .dw 5078
    .dw 4793
    .dw 4524
    .dw 4270
    .dw 4030
    .dw 3804
    .dw 3591