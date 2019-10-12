/*=========================================================

	MSX2: Platform dependent API functions

		You can change the next constants:
			SCREEN
					5: 256x212 16col (GRB332)
					6: 512x212 4col  (GRB332)
					7: 512x212 16col (GRB332)
					8: 256x212 256col (fixed palette)
			FONTWIDTH
					6: font 6x8 pixels
					8: font 8x8 pixels
			FONTHEIGHT
			
=========================================================*/
#ifndef  __PLATFORM_MSX2_H__
#define  __PLATFORM_MSX2_H__


// ========================================================
// Values to customize the MSX2 program

#define SCREEN 				8	// Screen 5/6/7/8
#define FONTWIDTH       	6	// 6/8px
#define FONTHEIGHT       	8	// Always 8px unless you know what you are doing :)

#define __MAPPER_CACHE			// Disabled right now


// ========================================================
// MSX2 Platform API constants

#if SCREEN==6 || SCREEN==7
	#define SCREEN_WIDTH    512
#else
	#define SCREEN_WIDTH    256
#endif
#define SCREEN_HEIGHT		212


// ========================================================
// MSX/MSX2 system constants

// System ports
#define SET_RAMSEG_P0	0xFC	// Default to segment 3
#define SET_RAMSEG_P1	0xFD	// Default to segment 2
#define SET_RAMSEG_P2	0xFE	// Default to segment 1
#define SET_RAMSEG_P3	0xFF	// Default to segment 0
// System calls
#define CALSLT	0x1C		// Interslot call
// System variables
#define MODE	0xfafc		/*Flag for screen mode. (1B/R)
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
#define JIFFY   0xfc9e		// (WORD) Contains value of the software clock, each interrupt of the VDP 
							//        it is increased by 1 (50/60Hz)
#define CLIKSW	0xf3db		// (BYTE) SCREEN ,,n will write to this address (0:disables_keys_click 1:enables_keys_click)
#define FNKSTR	0xf87f		// (10*16 bytes) Value of the function keys
#define NEWKEY  0xfbe5		// (11 bytes) with key matrix

// Tools
#define ADDR_POINTER_BYTE(X)	(*((uint8_t*)X))
#define ADDR_POINTER_WORD(X)	(*((uint16_t*)X))


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

typedef struct {
	char     magic[3];				// Magic text: "IMG".
	uint8_t  screenMode;			// Screen mode for this image: '5', '6', '7' or '8'.
} IMG_HEADER;

typedef struct {
	uint8_t  type;					// Chunk type
	uint16_t chunkSize;				// data[] size compressed   | also can be the target location for type Redirection
	uint16_t outSize;				// data[] size uncompressed | contain 0xffff if unused
	char     data[2043];			// Compressed data (uncompressed if type=RAW)
} IMG_CHUNK;


#endif //__PLATFORM_MSX2_H__
