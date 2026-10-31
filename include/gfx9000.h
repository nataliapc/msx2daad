/*
	GFX9000 (Yamaha V9990) support for MSX2DAAD.

	Only what the interpreter needs: bitmap mode B1 (256x212) with BD16
	colour (GRB555, 2 bytes per pixel, little endian, bit 15 = YS), the
	command engine (LMMV, LMMM, LMMC), CPU access to VRAM and the palette
	used by the backdrop colour.

	Port and register numbers checked against openMSX (src/video/v9990)
	and the Sunrise GFX9000 I/O base (0x60).
*/
#ifndef __GFX9000_H__
#define __GFX9000_H__

#include <stdint.h>
#include <stdbool.h>
#include "sdcc_compat.h"


// ========================================================
// Ports (GFX9000 at 0x60)

#define G9K_P_VRAM			0x60	// P#0 VRAM data (R/W)
#define G9K_P_PALETTE		0x61	// P#1 Palette data (R/W)
#define G9K_P_CMDDATA		0x62	// P#2 Command data (R/W)
#define G9K_P_REGDATA		0x63	// P#3 Register data (R/W)
#define G9K_P_REGSEL		0x64	// P#4 Register select (W): b7 WII, b6 RII
#define G9K_P_STATUS		0x65	// P#5 Status (R): b7 TR, b0 CE
#define G9K_P_INTFLAG		0x66	// P#6 Interrupt flag (R/W)
#define G9K_P_SYSCTRL		0x67	// P#7 System control (W): b1 SRS

// ========================================================
// Registers

#define G9K_R_WRITE_ADDR	0		// R#0-R#2 VRAM write address
#define G9K_R_READ_ADDR		3		// R#3-R#5 VRAM read address
#define G9K_R_SCREEN_MODE0	6		// R#6 DSPM, DCKM, XIMM, CLRM
#define G9K_R_SCREEN_MODE1	7		// R#7 C25M, SM1, SM, PAL, EO, IL, HSCN
#define G9K_R_CONTROL		8		// R#8 b7 DISP, b6 DIS (sprites off)
#define G9K_R_INTERRUPT0	9		// R#9 interrupt enables
#define G9K_R_PALETTE_CTRL	13		// R#13
#define G9K_R_PALETTE_PTR	14		// R#14
#define G9K_R_BACKDROP		15		// R#15 backdrop colour (palette index)
#define G9K_R_SCROLL_AY0	17		// R#17-R#18 layer A scroll Y
#define G9K_R_CMD			32		// R#32-R#52 command parameters

// ========================================================
// Command engine (R#52 OP)

#define G9K_OP_STOP			0x00
#define G9K_OP_LMMC			0x10	// CPU -> VRAM rectangle
#define G9K_OP_LMMV			0x20	// Fill VRAM rectangle with FC
#define G9K_OP_LMCM			0x30	// VRAM rectangle -> CPU
#define G9K_OP_LMMM			0x40	// VRAM rectangle -> VRAM rectangle
#define G9K_OP_BMXL			0x80	// VRAM linear -> VRAM rectangle

// Logical operations (R#45 LOG): 0 0 0 TP L11 L10 L01 L00
#define G9K_LOG_IMP			0x0C	// DC = SC
#define G9K_LOG_AND			0x08	// DC = SC & DC
#define G9K_LOG_OR			0x0E	// DC = SC | DC
#define G9K_LOG_XOR			0x06	// DC = SC ^ DC
#define G9K_LOG_NOT			0x03	// DC = !SC
#define G9K_LOG_TP			0x10	// Transparent: SC == 0 leaves DC unchanged

// Command packet, written to R#32..R#52 in this order (21 bytes)
typedef struct {
	uint16_t sx;			// R#32-R#33
	uint16_t sy;			// R#34-R#35
	uint16_t dx;			// R#36-R#37
	uint16_t dy;			// R#38-R#39
	uint16_t nx;			// R#40-R#41
	uint16_t ny;			// R#42-R#43
	uint8_t  arg;			// R#44
	uint8_t  log;			// R#45
	uint16_t mask;			// R#46-R#47 write mask
	uint16_t fc;			// R#48-R#49 foreground colour
	uint16_t bc;			// R#50-R#51 background colour
	uint8_t  op;			// R#52 (writing it starts the command)
} G9K_CMD;

// ========================================================
// VRAM layout used by MSX2DAAD in B1 BD16 (image space 256x1024)
//   Lines   0-211  visible screen         Lines 256-467  back screen
//   Lines 212-243  font                   Lines 476-507  graphic charset
//   Lines 244-251  TEMP char zone         Lines 768-980  picture buffer (linear, BMXL source)

#define G9K_LINE_BYTES		512				// 256 pixels * 2 bytes
#define G9K_VRAM_SCRATCH	0x60000UL		// Line 768: linear picture buffer (up to 256x212x2 bytes)

// GRB555 helper from 5-bit components
#define G9K_GRB555(r, g, b)	((uint16_t)(((uint16_t)(g) << 10) | ((uint16_t)(r) << 5) | (uint16_t)(b)))

// ========================================================
// Functions (src/libs/g9k_*)

bool g9kDetect();
void g9kInitB1BD16();
void g9kSetReg(uint8_t reg, uint8_t value);
void g9kDisplayOn();
void g9kDisplayOff();
void g9kSetScrollY(uint16_t y);
void g9kSetPaletteRGB(uint8_t index, uint8_t r5, uint8_t g5, uint8_t b5) SDCC_STACKCALL;

void g9kWaitCmd();
void g9kSendCmd(const G9K_CMD *cmd);

// Picture buffer: the pixel bytes of an LMMC rectangle are decoded to
// G9K_VRAM_SCRATCH through P#0 and the rectangle is drawn with one BMXL.
// g9kScratchBegin() sets the VRAM write pointer; after every chunk the caller
// adds its uncompressed size with g9kScratchAdvance().
extern uint8_t g9kScrPos[3];		// VRAM address of the next buffer byte (bits 18-0)
void g9kScratchBegin();
void g9kScratchAdvance(uint16_t length);
void g9kScratchRaw(const uint8_t *src, uint16_t length);
void g9kScratchRLE(const uint8_t *src);
void g9kScratchZX0(const uint8_t *src);

#endif //__GFX9000_H__
