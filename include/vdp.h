
#ifndef __VDP_H__
#define __VDP_H__



#if SCREEN==8 || SCREEN==7
	#define VRAM_PAGE1   0x10000
#else
	#define VRAM_PAGE1   0x08000
#endif

// BitBlt commands
#define CMD_HMMC	0b11110000       // High speed move CPU to VRAM: transfer data from the CPU to VRAM
#define CMD_YMMM	0b11100000       // High speed move VRAM to VRAM, Y coordinate only: transfer VRAM to VRAM in Y coordinate
#define CMD_HMMM	0b11010000       // High speed move VRAM to VRAM: transfer data from VRAM to VRAM
#define CMD_HMMV	0b11000000       // High speed move VDP to VRAM: paint in a specific rectangular area in the VRAM
#define CMD_LMMC	0b10110000       // Logical move CPU to VRAM: transfer data from the CPU to VRAM
#define CMD_LMCM	0b10100000       // Logical move VRAM to CPU: transfer data from VRAM to the CPU through VDP
#define CMD_LMMM	0b10010000       // Logical move VRAM to VRAM: transfer data from VRAM to VRAM
#define CMD_LMMV	0b10000000       // Logical move VDP to VRAM: paint in a specific rectangular area in the VRAM
#define CMD_LINE	0b01110000       // LINE command: used to draw straight line in VRAM
#define CMD_SRCH	0b01100000       // SRCH command: used to search for the specific color in VRAM
#define CMD_PSET	0b01010000       // PSET command: used to draw a dot in VRAM
#define CMD_POINT	0b01000000       // POINT command: used to read the color of the specified dot located in VRAM
#define CMD_STOP	0b00000000       // 
// Logical operations
#define LOG_IMP		0b00000000       // DC=SC
#define LOG_AND		0b00000001       // DC=SC & DC
#define LOG_OR		0b00000010       // DC=SC | DC
#define LOG_XOR		0b00000011       // DC=SC ^ DC
#define LOG_NOT		0b00000100       // DC=!SC
#define LOG_TIMP	0b00001000       // if SC=0 then DC=DC else DC=SC
#define LOG_TAND	0b00001001       // if SC=0 then DC=DC else DC=SC & DC
#define LOG_TOR	 	0b00001010       // if SC=0 then DC=DC else DC=SC | DC
#define LOG_TXOR	0b00001011       // if SC=0 then DC=DC else DC=SC ^ DC
#define LOG_TNOT	0b00001100       // if SC=0 then DC=DC else DC=!SC
// More used
#define LMMM_TIMP	CMD_LMMM | LOG_TIMP



void setRegVDP(char reg, char value);
void setRegVDP8(char reg, char value);

void setVDP_Read(uint32_t vram);
void setVDP_Write(uint32_t vram);

void enableVDP();
void disableVDP();

void enableSPR();
void disableSPR();

void enable50Hz();
void enable60Hz();

void enable192lines();
void enable212lines();

void enableInterlacedLines();
void disableInterlacedLines();

void setVPage0();
void setVPage1();
void setVPage2();
void setVPage3();

void setPalette(char *paletteAddress);
void setColorPal(uint8_t colIndex, uint16_t grb);	// grb bitfield: 00000GGG.0RRR0BBB

void setColor(char forecolor, char background, char border);

void clearSC5();
void clearSC5lines(uint16_t startline, uint16_t numLines);
void clearSC7();
void clearSC7lines(uint16_t startline, uint16_t numLines);

void bitBlt(uint16_t sx, uint16_t sy, uint16_t dx, uint16_t dy, uint16_t nx, uint16_t ny, uint8_t col, uint8_t arg, uint8_t cmd);
void fastVCopy(char *bitbltData);
void waitVDPready();



#endif //__VDP_H__
