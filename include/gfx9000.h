#pragma once
#ifndef __GFX9000_H__
#define __GFX9000_H__


// GFX9000 (V9990) Ports
#define G9k_PORT	0x60
#define G9k_P0		(G9k_PORT+0)	// VRAM DATA	(R/W)
#define G9k_P1		(G9k_PORT+1)	// PALETTE DATA	(R/W)
#define G9k_P2		(G9k_PORT+2)	// COMMAND DATA	(R/W)
#define G9k_P3		(G9k_PORT+3)	// REGISTER DATA	(R/W)
#define G9k_P4		(G9k_PORT+4)	// REGISTER SELECT	(W)
#define G9k_P5		(G9k_PORT+5)	// STATUS	(R)
#define G9k_P6		(G9k_PORT+6)	// INTERRUPT FLAG	(R/W)
#define G9k_P7		(G9k_PORT+7)	// SYSTEM CONTROL	(W)


// V9990 Screen Modes
#define G9K_B1_BD8				10100	// 256x212 256 colors (RGB332: 8 bpp)
#define G9K_B1_BD16				10116	// 256x212 32768 colors (RGB555: 16 bpp)
#define G9K_B2_BD8				10200	// 384x290 256 colors (RGB332: 8 bpp)
#define G9K_B2_BD16				10216	// 384x290 32768 colors (RGB555: 16 bpp)
#define G9K_B3_BD8				10300	// 512x212 256 colors (RGB332: 8 bpp)
#define G9K_B3_BD16				10316	// 512x212 32768 colors (RGB555: 16 bpp)
#define G9K_B3_BD8i				10301	// 512x424 256 colors (RGB332: 8 bpp)


// V9990 Commands variables & constants
	// Operation Commands
#define G9K_CMD_STOP    0b00000000		// Command being executed is stopped.
#define G9K_CMD_LMMC    0b00010000		// Data is transferred from CPU to VRAM rectangle area.
#define G9K_CMD_LMMV    0b00100000		// VRAM rectangle area is painted out.
#define G9K_CMD_LMCM    0b00110000		// VRAM rectangle area data is transferred to CPU.
#define G9K_CMD_LMMM    0b01000000		// Rectangle area data is transferred from VRAM to VRAM.
#define G9K_CMD_CMMC    0b01010000		// CPU character data is color-developed and transferred to VRAM rectangle area.
#define G9K_CMD_CMMM    0b01110000		// VRAM character data is color-developed and transferred to VRAM rectangle area.
#define G9K_CMD_BMXL    0b10000000		// Data on VRAM linear address is transferred to VRAM rectangle area.
#define G9K_CMD_BMLX    0b10010000		// VRAM rectangle area data is transferred onto VRAM linear address.
#define G9K_CMD_BMLL    0b10100000		// Data on VRAM linear address is transferred onto VRAM linear address.
#define G9K_CMD_LINE    0b10110000		// Straight line is drawn on X/Y-coordinates.
#define G9K_CMD_SRCH    0b11000000		// Border color coordinates on X/Y space are detected.
#define G9K_CMD_POINT   0b11010000		// Color code of specified point on X/Y-coordinates is read out.
#define G9K_CMD_PSET    0b11100000		// Drawing is executed at drawing point on X/Y-coordinates.
#define G9K_CMD_ADVCE   0b11110000		// Drawing point on X/Y-coordinates is shifted.
	// Logical operations: 0 0 0 TP L11 L10 L01 L00
#define G9K_LOG_IMP     0b00001100		// DC=SC
#define G9K_LOG_AND     0b00001000		// DC=SC & DC
#define G9K_LOG_OR      0b00001110		// DC=SC | DC
#define G9K_LOG_XOR     0b00000110		// DC=SC ^ DC
#define G9K_LOG_NOT     0b00000011		// DC=!SC
#define G9K_LOG_TIMP    0b00011100		// if SC=0 then DC=DC else DC=SC
#define G9K_LOG_TAND    0b00011000		// if SC=0 then DC=DC else DC=SC & DC
#define G9K_LOG_TOR     0b00011110		// if SC=0 then DC=DC else DC=SC | DC
#define G9K_LOG_TXOR    0b00010110		// if SC=0 then DC=DC else DC=SC ^ DC
#define G9K_LOG_TNOT    0b00010011		// if SC=0 then DC=DC else DC=!SC


// Window
typedef struct {
	uint16_t sx;
	uint16_t sy;
	uint16_t dx;
	uint16_t dy;
	uint16_t nx;
	uint16_t ny;
	uint8_t  arg;
	uint8_t  log;
	uint16_t mask;
	uint16_t foreColor;
	uint16_t backColor;
	uint8_t  cmd;
} G9K_CMD;


void g9k_enablePAL() __naked;
void g9k_enableNTSC() __naked;

void g9k_setPage(uint8_t page) __naked;

void g9k_setPalette_BD16(char *paletteAddress) __naked;
void g9k_setColorPal_BD16(uint8_t colIndex, uint16_t grb) __naked;

void g9k_setBorder(char border) __naked;

void g9k_bitBlt(
			uint16_t sx, uint16_t sy, 
			uint16_t dx, uint16_t dy, 
			uint16_t nx, uint16_t ny, 
			uint8_t arg, 
			uint8_t log, 
			uint16_t mask, 
			uint16_t foreColor, 
			uint16_t backColor, 
			uint8_t cmd
) __naked;

void g9k_fastCmd(void *bitbltData) __naked __z88dk_fastcall;
void g9k_waitCmdReady() __naked;
void g9k_sendData(uint8_t port, char *data, uint16_t size) __naked;
void g9k_sendDataRLE(char *data) __naked __z88dk_fastcall;
void g9k_sendDataPletter(char *data) __naked __z88dk_fastcall;

#endif//__GFX9000_H__

