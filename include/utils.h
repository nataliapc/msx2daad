
#ifndef __UTILS_H__
#define __UTILS_H__


void die(const char *s, ...);

void mouseInit();

void enableR800CPU();

void enableTurboCPU();
void disableTurboCPU();

void unRLE(char *source, char *target);
void unRLE_vram(char *srcmem, uint32_t vram);

void pletter2vram(char* data, uint32_t vram);
void pletter2ram(char* datain, char* dataout);

void dzx7vram (char *data, uint32_t vram_offset);

void dzx7b(char *input, char *output);
void dzx7bvram(char *input, char *output, uint32_t vram);

#endif // __UTILS_H__
