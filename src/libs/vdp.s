;
;
;

.globl setRegVDP
.globl setRegVDP8
.globl fastVCopy
.globl waitVDPready


; System variables
	RG0SAV  = 0xf3df   ; Mirror of VDP Register 0 (R#0)
	RG8SAV  = 0xffe7   ; Mirror Of VDP Register 8 (R#8)
	CLIKSW  = 0xf3DB   ; Keyboard click sound
	FORCLR  = 0xf3e9   ; Foreground colour
	DPPAGE  = 0xfaf5   ; Display Page Number
	ACPAGE  = 0xfaf6   ; Active Page Number
	FORCLR	= 0xf3e9   ; Foreground colour
	BAKCLR	= 0xf3ea   ; Background colour
	BDRCLR	= 0xf3eb   ; Border colour

; V9938 Commands system variables
	BLT_SX  = 0xf562   ; SX: Source X (word)
	BLT_SY  = 0xf564   ; SY: Source Y (word)
	BLT_DX  = 0xf566   ; DX: Destination X (word)
	BLT_DY  = 0xf568   ; DY: Destination Y (word)
	BLT_NX  = 0xf56a   ; NX: Number of dots X (word)
	BLT_NY  = 0xf56c   ; NY: Number of dots Y (word)
	BLT_COL = 0xf56e   ; COL: $00 for normal use
	BLT_ARG = 0xf56f   ; ARG: 0 - MXD MSX DIY DIX - - = $00 for normal use
	BLT_CMD = 0xf570   ; CMD: Command + Logical operation

; V9938 Commands variables & constants
	; Commands
	CMD_HMMC  =   0b11110000       ; 
	CMD_YMMM  =   0b11100000       ; 
	CMD_HMMM  =   0b11010000       ; 
	CMD_HMMV  =   0b11000000       ; 
	CMD_LMMC  =   0b10110000       ; 
	CMD_LMCM  =   0b10100000       ; 
	CMD_LMMM  =   0b10010000       ; 
	CMD_LMMV  =   0b10000000       ; 
	CMD_LINE  =   0b01110000       ; 
	CMD_SRCH  =   0b01100000       ; 
	CMD_PSET  =   0b01010000       ; 
	CMD_POINT =   0b01000000       ; 
	CMD_STOP  =   0b00000000       ; 
	; Logical operations
	LOG_IMP   =   0b00000000       ; DC=SC
	LOG_AND   =   0b00000001       ; DC=SC & DC
	LOG_OR    =   0b00000010       ; DC=SC | DC
	LOG_XOR   =   0b00000011       ; DC=SC ^ DC
	LOG_NOT   =   0b00000100       ; DC=!SC
	LOG_TIMP  =   0b00001000       ; if SC=0 then DC=DC else DC=SC
	LOG_TAND  =   0b00001001       ; if SC=0 then DC=DC else DC=SC & DC
	LOG_TOR   =   0b00001010       ; if SC=0 then DC=DC else DC=SC | DC
	LOG_TXOR  =   0b00001011       ; if SC=0 then DC=DC else DC=SC ^ DC
	LOG_TNOT  =   0b00001100       ; if SC=0 then DC=DC else DC=!SC
	; More used
	LMMM_TIMP =   CMD_LMMM|LOG_TIMP
