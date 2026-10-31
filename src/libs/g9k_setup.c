/*
	GFX9000 (V9990) set-up for MSX2DAAD: detection, bitmap mode B1 with BD16
	colour, display, scroll and palette.
*/
#include <stdint.h>
#include <stdbool.h>
#include "gfx9000.h"

__sfr __at (G9K_P_VRAM)     g9kPortVram;
__sfr __at (G9K_P_PALETTE)  g9kPortPalette;
__sfr __at (G9K_P_REGDATA)  g9kPortRegData;
__sfr __at (G9K_P_REGSEL)   g9kPortRegSel;
__sfr __at (G9K_P_SYSCTRL)  g9kPortSysCtrl;

static void setAddress(uint8_t reg, uint8_t page)
{
	g9kPortRegSel = reg;		// write increment on
	g9kPortRegData = 0;
	g9kPortRegData = 0;
	g9kPortRegData = page;		// R#2 / R#5 makes the pointer valid
}

/*
 * Function: g9kDetect
 * --------------------------------
 * Look for a V9990 at 0x60: write two patterns to VRAM and read them back.
 * An empty I/O range reads 0xFF.
 *
 * @return			true if the V9990 answers.
 */
bool g9kDetect()
{
	setAddress(G9K_R_WRITE_ADDR, 0x07);		// 0x70000: free VRAM
	g9kPortVram = 0x5A;
	g9kPortVram = 0xA5;
	setAddress(G9K_R_READ_ADDR, 0x07);
	if (g9kPortVram != 0x5A) return false;
	return g9kPortVram == 0xA5;
}

/*
 * Function: g9kInitB1BD16
 * --------------------------------
 * Reset the V9990 and set bitmap mode B1 (256x212, XTAL/4), image space 256
 * pixels wide, BD16 colour. The display stays off and VRAM is not cleared.
 *
 * @return			none.
 */
void g9kInitB1BD16()
{
	g9kPortSysCtrl = 0x02;		// SRS: every register to 0
	g9kPortSysCtrl = 0x00;
	g9kPortRegSel = G9K_R_SCREEN_MODE0;
	g9kPortRegData = 0x83;		// R#6: bitmap, B1, 256 pixels per line, BD16
	g9kPortRegData = 0x00;		// R#7: NTSC, non interlaced
	g9kPortRegData = 0x42;		// R#8: display off, sprites off, 512 KB
	g9kPortRegData = 0x00;		// R#9: no interrupts
}

void g9kDisplayOn()
{
	g9kSetReg(G9K_R_CONTROL, 0xC2);		// R#8: display on, sprites off, 512 KB
}

/*
 * Function: g9kDisplayOff
 * --------------------------------
 * Blank the GFX9000 display (as after g9kInitB1BD16), before going back to
 * MSX-DOS. It does not wait for the command engine: die() also calls it when
 * there is no GFX9000, and P#5 then reads 0xFF (busy for ever).
 *
 * @return			none.
 */
void g9kDisplayOff()
{
	g9kSetReg(G9K_R_CONTROL, 0x42);		// R#8: display off, sprites off, 512 KB
}

/*
 * Function: g9kSetScrollY
 * --------------------------------
 * Show the image space from line `y` (layer A scroll, R#17-R#18). The new
 * value is taken by the V9990 at the start of the next frame.
 *
 * @param y			First line to show.
 * @return			none.
 */
void g9kSetScrollY(uint16_t y)
{
	g9kPortRegSel = G9K_R_SCROLL_AY0;
	g9kPortRegData = (uint8_t)y;
	g9kPortRegData = (uint8_t)(y >> 8) & 0x1F;
}

/*
 * Function: g9kSetPaletteRGB
 * --------------------------------
 * Set a palette entry (used by the backdrop colour, R#15).
 *
 * @param index		Palette entry (0-63).
 * @param r5		Red (0-31).
 * @param g5		Green (0-31).
 * @param b5		Blue (0-31).
 * @return			none.
 */
void g9kSetPaletteRGB(uint8_t index, uint8_t r5, uint8_t g5, uint8_t b5) SDCC_STACKCALL
{
	g9kSetReg(G9K_R_PALETTE_PTR, index << 2);
	g9kPortPalette = r5;
	g9kPortPalette = g5;
	g9kPortPalette = b5;
}
