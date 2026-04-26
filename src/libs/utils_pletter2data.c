/*
 * pletter2Data — Pletter5 decompression with HMMC streaming output.
 *
 * Decompresses Pletter5 input to a hidden VRAM scratch zone (page 1 hidden,
 * Y=468..511, 11264 bytes), then copies the decompressed data from VRAM to
 * port #9B for HMMC streaming.
 *
 * Caller MUST have dispatched an HMMC command via fastVCopy() previously.
 * Caller GUARANTEES uncompSize <= 11264 (cap enforced by imgwizard).
 *
 * The pending HMMC persists across pletter2vram() because pletter2vram only
 * accesses VDP registers R0/R1/R14 (CPU↔VRAM access) via port #99, and
 * never touches R32-R46 (command registers) or port #9B.
 */

#include <stdint.h>
#include "utils.h"
#include "vdp.h"

#define VRAM_PLETTER_SCRATCH  0x1D400UL    // 119808: page 1 hidden zone
                                            // (Y=468..511 in SC7/8/A/C, 11264 bytes)

void pletter2Data(char *src, uint16_t uncompSize)
{
	pletter2vram(src, VRAM_PLETTER_SCRATCH);
	copyVRam2CmdData(VRAM_PLETTER_SCRATCH, uncompSize);
}
