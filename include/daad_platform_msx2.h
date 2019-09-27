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

#define SCREEN 				7
#define FONTWIDTH       	6
#define FONTHEIGHT       	8

#if SCREEN==6 || SCREEN==7
	#define SCREEN_WIDTH    512
#else
	#define SCREEN_WIDTH    256
#endif
#define SCREEN_HEIGHT		212
