/*
 * dzx0Data — ZX0 decompression with HMMC/LMMC streaming output.
 *
 * Decompresses ZX0 input to a hidden VRAM scratch zone (page 1 hidden,
 * Y=468..511, 11264 bytes), then copies the decompressed data from VRAM to
 * port #9B for HMMC/LMMC streaming.
 *
 * Caller MUST have dispatched an HMMC/LMMC command via fastVCopy() previously.
 * Caller GUARANTEES uncompSize <= 11264 (cap enforced by imgwizard).
 *
 * The pending HMMC/LMMC persists across dzx0vram() because dzx0vram only
 * accesses VDP registers R0/R1/R14 (CPU↔VRAM access) via port #99, and
 * never touches R32-R46 (command registers) or port #9B.
 */

#include <stdint.h>
#include "utils.h"
#include "vdp.h"


void dzx0Data(char *src, uint16_t uncompSize)
{
	dzx0vram(src, VRAM_DECOMP_SCRATCH);
	copyVRam2CmdData(VRAM_DECOMP_SCRATCH, uncompSize);
}
