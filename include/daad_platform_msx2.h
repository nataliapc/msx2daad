/*
==========================================================
	MSX2: Platform dependent API functions

		You must define the next constants:
			SCREEN
					5: 256x212 16col (GRB332)
					6: 512x212 4col  (GRB332)
					7: 512x212 16col (GRB332)
					8: 256x212 256col (fixed palette)
			FONTWIDTH
					6: font 6x8 pixels
					8: font 8x8 pixels
			FONTHEIGHT
			SCREEN_WIDTH
			SCREEN_HEIGHT
==========================================================
*/

#define SCREEN 				8
#define FONTWIDTH       	6
#define FONTHEIGHT       	8

#if SCREEN==6 || SCREEN==7
	#define SCREEN_WIDTH    512
#else
	#define SCREEN_WIDTH    256
#endif
#define SCREEN_HEIGHT		212

// Image format constants
#define IMG_MAGIC 			"IMG "
#define IMG_CHUNK_REDIRECT	0		// Redirect to another image
#define IMG_CHUNK_PALETTE	1		// Load palette
#define IMG_CHUNK_RAW		2		// Load RAW format data
#define IMG_CHUNK_RLE		3		// Load RLE format data
#define IMG_CHUNK_PLETTER	4		// Load Pletter5 format data
