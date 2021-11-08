/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio

	=========================================================

		MSX2: System dependent API functions

			You can change the next constants:

			SCREEN
					5: 256x212 16col (Paletted GRB332)
							Color 0: Always PAPER (default: black)
							Color 1-14: For bitmap images
							Color 15: Always INK (default: white)
							INK/PAPER changes, will change the 0/15 color palette for all the text/background.
							
					6: 512x212 4col  (Paletted GRB332)
							Color 0: Always PAPER (default: black)
							Color 1-2: For bitmap images
							Color 3: Always INK (default: white)
							INK/PAPER changes, will change the 0/3 color palette for all the text/background.

					7: 512x212 16col (Paletted GRB332)
							Color 0: Always PAPER (default: black)
							Color 1-14: For bitmap images
							Color 15: Always INK (default: white)
							INK/PAPER changes, will change the 0/15 color palette for all the text/background.

					8: 256x212 256col (fixed palette GRB332) [**DEFAULT MODE**]
							Bitmap mode with fixed palette (0-255)
							PAPER/INK color changes will change color to write new text or clear screen. Old text remains unchanged.

					12: 256x212 19268col (fixed palette YJK. Y vary each pixel, J & K remains each 4 pixels ~RGB555)
							Bitmap mode with fixed palette (0-19268)
							PAPER/INK changes don't have effect. INK is always white and PAPER is always black.

			SCREEN_HEIGHT
					212: Default (26 rows)
					192: You can change the screen height to 192px (24 rows)
			
			FONTWIDTH
					6: font 6x8 pixels
					8: font 8x8 pixels

			FONTHEIGHT
					8: Always 8px unless you know what you are doing :)

	=========================================================
*/
#ifndef  __PLATFORM_MSX2_H__
#define  __PLATFORM_MSX2_H__

// ========================================================
// Values to customize the MSX2 program

#ifndef VERSION
	#define VERSION				1.4
#endif
#ifndef SCREEN
	#define SCREEN 				8	// Screen 5/6/7/8/10/12
#endif
#ifndef SCREEN_HEIGHT
	#define SCREEN_HEIGHT		212	// Screen height in pixels: 192 or 212. MSX2 VDP can toggle it.
#endif
#ifndef FONTWIDTH
	#define FONTWIDTH       	6	// 6/8px
#endif
#ifndef FONTHEIGHT
	#define FONTHEIGHT       	8	// Always 8px unless you know what you are doing :)
#endif

#ifndef RAM_MAPPER
	//#define RAM_MAPPER			// [Experimental] Use of RAM Mapper if exists
#endif
#ifndef DISABLE_GFXCHAR_COLOR
	//#define DISABLE_GFXCHAR_COLOR	// Disable ink/paper with upper gfx chars (>=128)
									// This is used if you create multicolor object icons in the upper charset
#endif


// ========================================================
// MSX2 Platform API automatic constants

#ifdef LANG_EN
	#define LANG  	EN
#endif
#ifdef LANG_ES
	#define LANG  	ES
#endif
#define VERSION_STR			"MSX2DAAD v"STRINGIFY(VERSION)"#"STRINGIFY(LANG)"#"STRINGIFY(SCREEN)"_"STRINGIFY(FONTWIDTH)

// SCREEN_WIDTH
#if SCREEN==6 || SCREEN==7
	#define SCREEN_WIDTH    512
#else
	#define SCREEN_WIDTH    256
#endif

// BYTESxLINE
#if SCREEN >= 7
	#define BYTESxLINE		256		// Screen 7, 8, 10, 12
#else
	#define BYTESxLINE		128		// Screen 5, 6
#endif

// PIXELSxBYTE
#if SCREEN >= 8
	#define PIXELSxBYTE 	1		// Screen 8, 10, 12
#elif SCREEN ==6
	#define PIXELSxBYTE 	4		// Screen 6
#else
	#define PIXELSxBYTE		2		// Screen 5, 7
#endif


// FONT VRAM Location
#define FONTINITY			212
#define FONTTEMPY			(FONTINITY + 32)

// SCREEN_CHAR
#if SCREEN==10
	#define SCREEN_CHAR     STRINGIFY(A)
#elif SCREEN==12
	#define SCREEN_CHAR     STRINGIFY(C)
#else
	#define SCREEN_CHAR     STRINGIFY(SCREEN)
#endif

#ifndef FONTWIDTH
	#error "FONTWIDTH constant must be 4, 6 or 8!"
#endif

#ifndef SCREEN_WIDTH
	#error "SCREEN_WIDTH must be defined!"
#endif

#ifndef LANG
	#error "LANG_EN or LANG_ES must be defined!"
#endif

#if !defined(SCREEN_WIDTH) || !defined(SCREEN_HEIGHT)
	#error "SCREEN must be defined!"
#endif

#if SCREEN_HEIGHT!=192 && SCREEN_HEIGHT!=212
	#error "SCREEN_HEIGHT must be 192 or 212!"
#endif

#define MAX_COLUMNS		((int)(SCREEN_WIDTH/FONTWIDTH))
#define MAX_LINES		((int)(SCREEN_HEIGHT/FONTHEIGHT))


// ========================================================
// MSX/MSX2 system constants

#define FILEH			uint16_t

// RAM Addresses
#define PAGE0 			((void*)0x0000)
#define PAGE1 			((void*)0x4000)
#define PAGE2 			((void*)0x8000)
#define PAGE3 			((void*)0xC000)

// System ports
#define SET_RAMSEG_P0	0xFC	// Default to segment 3
#define SET_RAMSEG_P1	0xFD	// Default to segment 2
#define SET_RAMSEG_P2	0xFE	// Default to segment 1
#define SET_RAMSEG_P3	0xFF	// Default to segment 0
// System calls
#define CALSLT	0x1C		// Interslot call

// System variables
#define TPALIM		0x0006	// (WORD) TPA Limit, to calculate the max size of a program
#define CLIKSW		0xf3db	// (BYTE) SCREEN ,,n will write to this address (0:disables_keys_click 1:enables_keys_click)
#define PUTPNT		0xf3f8	// (WORD) Address in the keyboard buffer where a character will be written
#define GETPNT		0xf3fa	// (WORD) Address in the keyboard buffer where the next character is read
#define FNKSTR		0xf87f	// (10*16 bytes) Value of the function keys
#define MODE		0xfafc	/*Flag for screen mode. (1B/R)
								bit 7: 1 = conversion to Katakana; 0 = conversion to Hiragana. (MSX2+~)
								bit 6: 1 if Kanji ROM level 2. (MSX2+~)
								bit 5: 0/1 to draw in RGB / YJK mode SCREEN 10 or 11. (MSX2+~)
								bit 4: 0/1 to limit the Y coordinate to 211/255. (MSX2+~)
								bit 3: 1 to apply the mask in SCREEN 0~3.
								bits 1-2: VRAM size
											00 for 16kB
											01 for 64kB
											10 for 128kB
											11 for 192kB
								bit 0: 1 if the conversion of Romaji to Kana is possible. (MSX2~)*/
#define NEWKEY  	0xfbe5	// (11 bytes) with key matrix
#define KEYBUF		0xfbf0	// (40 bytes) Key code buffer [0xfbf0...0xfc17]
#define JIFFY   	0xfc9e	// (WORD) Contains value of the software clock, each interrupt of the VDP it is increased by 1 (50/60Hz)

volatile __at (TPALIM) uint16_t varTPALIMIT;
volatile __at (CLIKSW) uint8_t  varCLIKSW;
volatile __at (PUTPNT) uint16_t varPUTPNT;
volatile __at (GETPNT) uint16_t varGETPNT;
volatile __at (MODE)   uint8_t  varMODE;
volatile __at (JIFFY)  uint16_t varJIFFY;


#define ASM_HALT	__asm halt __endasm;

// ========================================================
// Image format IMx structs & constants

#define IMG_CHUNK_SIZE		2043
#define IMG_MAXREAD			IMG_CHUNK_SIZE + 1 + 2 + 2 	// 1:type + 2:sizeIn + 2:sizeOut + 2043:max_chunk_size

#define IMG_MAGIC 			"IMG"
#define IMG_CHUNK_REDIRECT	0		// Redirect to another image
#define IMG_CHUNK_PALETTE	1		// Load palette
#define IMG_CHUNK_RAW		2		// Load RAW format data
#define IMG_CHUNK_RLE		3		// Load RLE format data
#define IMG_CHUNK_PLETTER	4		// Load Pletter5 format data
#define IMG_CHUNK_RESET		16		// Reset VRAM write pointer to current Window
#define IMG_CHUNK_CLS		17		// Clear Window (CLS)
#define IMG_CHUNK_SKIP		18		// Skip VRAM bytes (SKIP)
#define IMG_CHUNK_PAUSE		19		// Pause in 1/50 sec units (PAUSE)

typedef struct {
	char     magic[3];				// Magic text: "IMG".
	uint8_t  screenMode;			// Screen mode for this image: '5', '6', '7' or '8'.
} IMG_HEADER;

typedef struct {
	uint8_t  type;					// Chunk type
	uint16_t chunkSize;				// data[] size compressed   | also can be the target location for type Redirection
	uint16_t auxData;				// data[] size uncompressed | contains 0x0000 if unused
	char     data[IMG_CHUNK_SIZE];	// Compressed data (uncompressed if type=RAW)
} IMG_CHUNK;


#endif //__PLATFORM_MSX2_H__
