/*
==========================================================
	Platform dependent API functions
==========================================================
*/
#ifndef  __DAAD_PLATFORM_API_H__
#define  __DAAD_PLATFORM_API_H__

#include <stdint.h>
#if defined(MSX2)
	#include "daad_platform_msx2.h"
#elif defined(PC_TXT)
	#include "daad_platform_pctxt.h"
#endif


#if !defined(FONTWIDTH)
	//#error "FONTWIDTH constant must be 6 or 8!"
#endif

#if !defined(SCREEN_WIDTH)
	//#error "SCREEN_WIDTH must be defined!"
#endif

#if !defined(SCREEN_WIDTH) || !defined(SCREEN_HEIGHT)
	//#error "SCREEN_WIDTH and SCREEN_HEIGHT must be defined!"
#endif

#define MAX_COLUMNS		((int)(SCREEN_WIDTH/FONTWIDTH))
#define MAX_LINES		(SCREEN_HEIGHT/FONTHEIGHT)

// Macro helpers
#define STRINGIFY2(x)	#x
#define STRINGIFY(x) STRINGIFY2(x)

// System functions
bool     checkPlatformSystem();
uint16_t getFreeMemory();
char*    getCharsTranslation();
void     setTime(uint16_t time);
uint16_t getTime();
void     waitForPrompt();

// Filesystem
void     loadFilesBin();
uint16_t loadFile(char *filename, uint8_t *destaddress, uint16_t size);
uint16_t fileSize(char *filename);


// GFX functions
void gfxSetScreen();
void gfxClearLines(uint16_t start, uint16_t lines);
void gfxClearScreen();
void gfxClearWindow();
void gfxSetPaperCol(uint8_t col);
void gfxSetInkCol(uint8_t col);
void gfxSetBorderCol(uint8_t col);
void gfxPutChWindow(uint8_t c);
void gfxPutChPixels(uint8_t c, uint16_t dx, uint16_t dy);
void gfxPutCh(char c);
void gfxPuts(char *str);
void gfxPutsln(char *str);
void gfxScrollUp();
bool gfxPicturePrepare(uint8_t location);
void gfxPictureShow();


// SFX functions
void sfxSound(uint8_t value1, uint8_t value2);


#endif  //__DAAD_PLATFORM_API_H__
