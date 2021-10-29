/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio

	==========================================================
		System dependent API functions
	==========================================================
*/
#ifndef  __DAAD_PLATFORM_API_H__
#define  __DAAD_PLATFORM_API_H__

#include <stdint.h>
#include <stdbool.h>
#if defined(MSX2)
	#include "daad_platform_msx2.h"
#elif defined(CPM)
	#include "daad_platform_cpm.h"
#elif defined(PC_TXT)
	#include "daad_platform_pctxt.h"
#endif

#ifndef VERSION
	#error "VERSION define must be declared in the daad_platform_XXX.c file."
#endif

// Macro helpers
#define STRINGIFY2(x)	#x
#define STRINGIFY(x) STRINGIFY2(x)

// Tools
#define ADDR_POINTER_BYTE(X)	(*((uint8_t*)X))
#define ADDR_POINTER_WORD(X)	(*((uint16_t*)X))


// System functions
bool     checkPlatformSystem();
uint16_t getFreeMemory();
char*    getCharsTranslation();
void     setTime(uint16_t time) __z88dk_fastcall;
uint16_t getTime();
uint16_t checkKeyboardBuffer();
void     clearKeyboardBuffer();
uint8_t  getKeyInBuffer();
void     waitingForInput();
inline void *safeMemoryAllocate();
inline void safeMemoryDeallocate(void*);


// Filesystem
void     loadFilesBin(int argc, char **argv);
uint16_t loadFile(char *filename, uint8_t *destaddress, uint16_t size);


// External texts
void printXMES(uint16_t address) __z88dk_fastcall;


// GFX functions
void gfxSetScreen();
void gfxSetScreenModeFlags();
void gfxClearScreenBlock(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
inline void gfxClearWindow();
inline void gfxClearCurrentLine();
inline void gfxScrollUp();
inline void gfxSetPaperCol(uint8_t col);
inline void gfxSetInkCol(uint8_t col);
inline void gfxSetBorderCol(uint8_t col);
inline void gfxSetGraphCharset(bool value);
inline void gfxPutChWindow(uint8_t c);
static void gfxPutChPixels(uint8_t c, uint16_t dx, uint16_t dy);
inline void gfxPutInputEcho(char c, bool keepPos);
inline void gfxSetPalette(uint8_t index, uint8_t red, uint8_t green, uint8_t blue);
bool gfxPicturePrepare(uint8_t location);
inline bool gfxPictureShow();
inline void gfxRoutines(uint8_t routine, uint8_t value);

// GFX constants
#define GFX_FULL_COPY_TO_PHYS		0
#define GFX_FULL_COPY_TO_BACK		1
#define GFX_SWAP_PHYS_BACK			2
#define GFX_GRAPHICS_IN_PHYS		3
#define GFX_GRAPHICS_IN_BACK		4
#define GFX_CLEAR_PHYS				5
#define GFX_CLEAR_BACK				6
#define GFX_TEXTS_IN_PHYS			7	//**UNUSED YET**
#define GFX_TEXTS_IN_BACK			8	//**UNUSED YET**
#define GFX_SET_PALETTE				9
#define GFX_GET_PALETTE				10
#define GFX_CURRENT_WIN_TO_PHYS		128
#define GFX_CURRENT_WIN_TO_BACK		129


// SFX functions
void sfxInit();
void sfxWriteRegister(uint8_t reg, uint8_t value);
void sfxTone(uint8_t value1, uint8_t value2);


#endif  //__DAAD_PLATFORM_API_H__
