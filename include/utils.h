
#ifndef __UTILS_H__
#define __UTILS_H__

#include "sdcc_compat.h"


#define VRAM_DECOMP_SCRATCH   0x1D400UL    // 119808: page 1 hidden zone
                                           // (Y=468..511 in SC7/8/A/C, 11264 bytes)
                                           // Shared scratch for Pletter5/ZX0 decoders.
#define VRAM_PLETTER_SCRATCH  VRAM_DECOMP_SCRATCH    // Legacy alias


void die(const char *s, ...);

void mouseInit();

void enableR800CPU();
void disableR800CPU();

void enableTurboCPU();
void disableTurboCPU();

void unRLE(char *source, char *target) SDCC_STACKCALL;
void unRLE_vram(char *srcmem, uint32_t vram) SDCC_STACKCALL;
void unRLE_Data(char *source) SDCC_STACKCALL;

void pletter2vram(char* data, uint32_t vram) SDCC_STACKCALL;
void pletter2ram(char* datain, char* dataout);
void pletter2Data(char* src, uint16_t uncompSize);

void dzx0(unsigned char* source, unsigned char* target);
void dzx0vram(unsigned char* source, uint32_t vram_offset) SDCC_STACKCALL;
void dzx0Data(char* src, uint16_t uncompSize);

void dzx7vram (char *data, uint32_t vram_offset) SDCC_STACKCALL;

void dzx7b(char *input, char *output);
void dzx7bvram(char *input, char *output, uint32_t vram);

int strnicmp (const char *s1, const char *s2, int maxlen);

#endif // __UTILS_H__
