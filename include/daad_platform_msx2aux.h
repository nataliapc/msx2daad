/*
	Copyright (c) 2022 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#ifndef  __PLATFORM_MSX2AUX_H__
#define  __PLATFORM_MSX2AUX_H__


static void gfxAux_setScreen() __naked;
inline static void gfxAux_initScreen();
inline static void gfxAux_clearScreenBlock(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
inline static void gfxAux_copyScrollUp();
inline static void gfxAux_putChPixels(uint8_t c, uint16_t dx, uint16_t dy);
inline static void gfxAux_setPalette(uint8_t index, uint8_t red, uint8_t green, uint8_t blue);
inline static void gfxAux_paintDataChunk(IMG_CHUNK *chunk);
inline static uint16_t gfxAux_chunkSetPalette(FILEH fp, uint16_t size, IMG_CHUNK *chunk);

inline static void gfxAux_copyBackToPhys();
inline static void gfxAux_copyPhysToBack();
inline static void gfxAux_clearPhys(uint16_t page_offset);
inline static void gfxAux_copyCurrentWindowTo(uint8_t routine);


#endif //__PLATFORM_MSX2AUX_H__