;
;
;

.globl setRegVDP
.globl setRegVDP8
.globl fastVCopy
.globl waitVDPready
.globl setVDP_Read
.globl setVDP_Write


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
	CMD_HMMC  =   0b11110000       ; High speed move CPU to VRAM: transfer data from the CPU to VRAM
	CMD_YMMM  =   0b11100000       ; High speed move VRAM to VRAM, Y coordinate only: transfer VRAM to VRAM in Y coordinate
	CMD_HMMM  =   0b11010000       ; High speed move VRAM to VRAM: transfer data from VRAM to VRAM
	CMD_HMMV  =   0b11000000       ; High speed move VDP to VRAM: paint in a specific rectangular area in the VRAM
	CMD_LMMC  =   0b10110000       ; Logical move CPU to VRAM: transfer data from the CPU to VRAM
	CMD_LMCM  =   0b10100000       ; Logical move VRAM to CPU: transfer data from VRAM to the CPU through VDP
	CMD_LMMM  =   0b10010000       ; Logical move VRAM to VRAM: transfer data from VRAM to VRAM
	CMD_LMMV  =   0b10000000       ; Logical move VDP to VRAM: paint in a specific rectangular area in the VRAM
	CMD_LINE  =   0b01110000       ; LINE command: used to draw straight line in VRAM
	CMD_SRCH  =   0b01100000       ; SRCH command: used to search for the specific color in VRAM
	CMD_PSET  =   0b01010000       ; PSET command: used to draw a dot in VRAM
	CMD_POINT =   0b01000000       ; POINT command: used to read the color of the specified dot located in VRAM
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
