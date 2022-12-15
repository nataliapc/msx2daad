

static void gfxAux_setScreen() __naked
{
	__asm
	#if SCREEN <= 8

		ld   a,#SCREEN
		ld   iy,(#EXPTBL)
		ld   ix,#0x5f
		call CALSLT

	#elif SCREEN == 10 || SCREEN == 12

		ld   a,#8
		ld   iy,(#EXPTBL)
		ld   ix,#0x5f
		call CALSLT
		#if SCREEN==10
			ld   bc,#0x1899		; enable YJK colors + Palette mixed mode
		#elif SCREEN==12
			ld   bc,#0x0899		; enable YJK colors
		#endif
		out  (c),b
		out  (c),c
		ret
	__endasm;

	#elif defined(V9990)
		ld hl,#.registers$
		ld c, #G9k_P7
		outi
		ld   a, #6
		out  (G9k_P4), a	// Select R#6
		ld c, #G9k_P3
		outi				// R#6 ScreenSize + ColorMode
		outi				// R#7 PAL/NTSC + Interlaced
		outi				// R#8 DisplayOn + CursorOff + DigitizeOff + YsOff
		ld   a, #13
		out  (G9k_P4), a	// Select R#13
		ld   a, #0b01000000
		out  (G9k_P3), a	// R#13 - Palette control: 256 colors mode
		xor  a
		out  (G9k_P3), a	// R#14 - Select Palette no. 0
		out  (G9k_P1), a
		out  (G9k_P1), a
		out  (G9k_P1), a	// Set Color no.0 to Black
		ld   a, #15
		out  (G9k_P4), a	// Select R#15
		xor  a
		out  (G9k_P3), a	// Border colour to Black
		ret
	.registers$:
		#if SCREEN==G9K_B1_BD8
			.db 0x00, 0b10000010, 0b00001000, 0b11000010
		#elif SCREEN==G9K_B1_BD16
			.db 0x00, 0b10000011, 0b00001000, 0b11000010
		#elif SCREEN==G9K_B2_BD8
			.db 0x01, 0b10010110, 0b00001000, 0b11000010
		#elif SCREEN==G9K_B2_BD16
			.db 0x01, 0b10010111, 0b00001000, 0b11000010
		#elif SCREEN==G9K_B3_BD8
			.db 0x00, 0b10010110, 0b00001000, 0b11000010
		#elif SCREEN==G9K_B3_BD8i
			.db 0x00, 0b10010110, 0b00001110, 0b11000010
		#elif SCREEN==G9K_B3_BD16
			.db 0x00, 0b10010111, 0b00001000, 0b11000010
		#else
			#error "Undefined Screen mode"
		#endif
	#endif
	__endasm;
}


inline static void gfxAux_initScreen()
{
	#ifdef V9958
		//Change screen settings
		disableInterlacedLines();
		#if SCREEN_HEIGHT==192
			enable192lines();
		#elif SCREEN_HEIGHT==212
			enable212lines();
		#endif

		//Clear VRAM page 2
		gfxRoutines(GFX_CLEAR_BACK, 0);	// Clear Back screen

		//Disable hardware sprites
		disableSPR();

		//Set screen adjust
		setRegVDP8(18, ADDR_POINTER_BYTE(0xFFF1));

	#endif
	#ifdef V9990
		//Clear VRAM page 1
		gfxRoutines(GFX_CLEAR_PHYS, 0);	// Clear Phys screen
		//Clear VRAM page 2
		gfxRoutines(GFX_CLEAR_BACK, 0);	// Clear Back screen
	#endif

	#if SCREEN < 100 && SCREEN != 8 && SCREEN != 12
		//Set Palette
		setPalette(colorTranslation);
	#endif
}


inline static void gfxAux_clearScreenBlock(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	static uint16_t w2;
	w2 = w<MAX_COLUMNS ? w*FONTWIDTH : SCREEN_WIDTH;
	#ifdef V9958
		bitBlt(0, 0, x*FONTWIDTH, y*FONTHEIGHT, w2, h*FONTHEIGHT, COLOR_PAPER, 0, CMD_HMMV);
	#elif defined(V9990)
		g9k_bitBlt(0, 0, 			// Sx Sy
				   x*FONTWIDTH, y*FONTHEIGHT, // Dx Dy
				   w2, h*FONTHEIGHT,// Nx Ny
				   0,				// arg
				   G9K_LOG_IMP,		// log
				   0xffff,			// mask
				   COLOR_PAPER,		// fc
				   0,				// bg
				   G9K_CMD_LMMV);	// VRAM rectangle area is painted out.
	#else
		#error "No BitBlt"
	#endif
}


inline static void gfxAux_copyScrollUp()
{
	#ifdef V9958
		bitBlt(cw->winX*FONTWIDTH, (cw->winY+1)*FONTHEIGHT, 
		       cw->winX*FONTWIDTH, cw->winY*FONTHEIGHT, 
			   cw->winW*FONTWIDTH, (cw->winH-1)*FONTHEIGHT, 
			   0, 
			   0, 
			   CMD_HMMM);
	#elif defined(V9990)
		g9k_bitBlt(cw->winX*FONTWIDTH, (cw->winY+1)*FONTHEIGHT,	// Sx Sy
				cw->winX*FONTWIDTH, cw->winY*FONTHEIGHT,		// Dx Dy
				cw->winW*FONTWIDTH, (cw->winH-1)*FONTHEIGHT,	// Nx Ny
				0,				// arg
				G9K_LOG_IMP,		// log
				0xffff,			// mask
				0,				// fc
				0,				// bg
				G9K_CMD_LMMM);	// Rectangle area data is transferred from VRAM to VRAM.
	#else
		#error "No BitBlt"
	#endif
}


inline static void gfxAux_putChPixels(uint8_t c, uint16_t dx, uint16_t dy)
{
	c -= 16;
	uint16_t sx = (c*8)%SCREEN_WIDTH,
	         sy = (c/(SCREEN_WIDTH/8)*FONTHEIGHT) + FONTINITY;

	if ((cw->mode & MODE_FORCEGCHAR) || offsetText) sy += (256+8);

	#if SCREEN <= 10
		#ifdef DISABLE_GFXCHAR_COLOR
			if (c>=128-16) {
				bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, AUX_HMMM);
			} else
		#endif
		if (COLOR_PAPER==COLOR_BLACK) {
			bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, AUX_HMMM);							// Paint char in white
			if (COLOR_INK!=COLOR_WHITE) {
				bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, COLOR_INK, 0, CMD_LMMV|LOG_TAND);			// Paint char INK foreground
			}
		} else {
			//Use VRAM like TEMP working space to avoid glitches
			if (COLOR_INK==COLOR_BLACK) {
				bitBlt( 0,  0, dx, FONTTEMPY, FONTWIDTH, FONTHEIGHT, 255, 0, AUX_HMMV);					// Paint white background destination
				bitBlt(sx, sy, dx, FONTTEMPY, FONTWIDTH, FONTHEIGHT, 0, 0, CMD_LMMM|LOG_XOR);			// Paint char in black
				bitBlt( 0,  0, dx, FONTTEMPY, FONTWIDTH, FONTHEIGHT, COLOR_PAPER, 0, CMD_LMMV|LOG_AND);	// Paint PAPER background destination
				bitBlt(dx, FONTTEMPY, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, AUX_HMMM);				// Copy TEMP char to destination
			} else {
				bitBlt( 0,  0, dx, dy, FONTWIDTH, FONTHEIGHT, COLOR_PAPER, 0, AUX_HMMV);				// Paint PAPER background destination
				bitBlt(sx, sy, dx, FONTTEMPY, FONTWIDTH, FONTHEIGHT, 0x00, 0, AUX_HMMM);				// Paint TEMP char in white
				bitBlt(sx, sy, dx, FONTTEMPY, FONTWIDTH, FONTHEIGHT, COLOR_INK, 0, CMD_LMMV|LOG_TAND);	// Paint TEMP INK color foreground
				bitBlt(dx, FONTTEMPY, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, CMD_LMMM|LOG_TIMP);		// Copy TEMP char to destination
			}
		}
	#elif SCREEN == 12
		bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, AUX_HMMM);								// Copy char in white
	#elif defined(V9990)
		#if defined(G9K_BD16)
		g9k_bitBlt(0, 0, dx, dy, FONTWIDTH, FONTHEIGHT, 0, G9K_LOG_IMP, 0xffff, COLOR_PAPER, 0, G9K_CMD_LMMV);
		g9k_bitBlt(sx, sy,	// Sx Sy
			dx, dy,			// Dx Dy
			FONTWIDTH, FONTHEIGHT,	// Nx Ny
			0,				// arg
			G9K_LOG_TIMP,	// log
			COLOR_INK,		// mask
			0,				// fc
			0,				// bg
			G9K_CMD_LMMM);	// Rectangle area data is transferred from VRAM to VRAM.
		#elif defined(G9K_BD8)
			g9k_bitBlt(0, 0, dx, dy, FONTWIDTH, FONTHEIGHT, 0, G9K_LOG_IMP, 0xffff, COLOR_PAPER, 0, G9K_CMD_LMMV);
			g9k_bitBlt(sx, sy,	// Sx Sy
				dx, dy,			// Dx Dy
				FONTWIDTH, FONTHEIGHT,	// Nx Ny
				0,				// arg
				G9K_LOG_TIMP,	// log
				0xffff,			// mask
				COLOR_INK,		// fc
				0,				// bg
				G9K_CMD_LMMM);	// Rectangle area data is transferred from VRAM to VRAM.
		#else
			#error "Missing drawChar method"
		#endif
	#else
		#error "Missing drawChar method"
	#endif
}


inline static void gfxAux_setPalette(uint8_t index, uint8_t red, uint8_t green, uint8_t blue)
{
	#if defined(V9958) || defined(G9K_BD8)
		setColorPal(index%16, (((uint16_t)red & 0b11100000)<<3) | ((green & 0b11100000)>>1) | ((blue & 0b11100000)>>5));
	#elif defined(G9K_BD16)
		uint16_t *pointer = &colorTranslation[index%8];
		*pointer =  ((((uint16_t)red)   & 0b11111000)<<2) | 
					((((uint16_t)green) & 0b11111000)<<7) | 
					((((uint16_t)blue)  & 0b11111000)>>3);
	#else
		#error "No Palette method"
	#endif
}


inline static uint16_t gfxAux_chunkSetPalette(FILEH fp, uint16_t size, IMG_CHUNK *chunk)
{
	#if SCREEN!=8 && SCREEN!=12
		size = fread(chunk->data, 32, fp);
		if (!(size & 0xff00))
			setPalette(chunk->data);
	#else
		fseek(fp, 32, SEEK_CUR);
	#endif

	return size;
}


inline static void gfxAux_paintDataChunk(IMG_CHUNK *chunk)
{
	#ifdef V9990
		if (chunk->type==IMG_CHUNK_G9K_CMD) {
			G9K_CMD *cmd = (G9K_CMD*)chunk->data;
			cmd->dx += cw->winX * FONTWIDTH;
			cmd->dy += cw->winY * FONTHEIGHT;
			g9k_fastCmd(chunk->data);
		} else
		if (chunk->type==IMG_CHUNK_G9K_RAW) {
			g9k_sendData(G9k_P2, chunk->data, chunk->chunkSize);
		} else
		if (chunk->type==IMG_CHUNK_G9K_RLE) {
			g9k_sendDataRLE(chunk->data);
		} else
		if (chunk->type==IMG_CHUNK_G9K_PLETTER) {
			g9k_sendDataPletter(chunk->data);
		}
	#endif
	#ifdef V9958
		if (chunk->type==IMG_CHUNK_RAW) {		// Show RAW data
			copyToVRAM((uint16_t)chunk->data, posVRAM, chunk->chunkSize);
		} else
		if (chunk->type==IMG_CHUNK_RLE) {		// Show RLE data
			unRLE_vram(chunk->data, posVRAM);
		} else
		if (chunk->type==IMG_CHUNK_PLETTER) {	// Show Pletter5 data
			pletter2vram(chunk->data, posVRAM);
		}
	#endif
}


inline static void gfxAux_copyBackToPhys()
{
	#ifdef V9958
		bitBlt(0, BACK_Y, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00, 0, CMD_HMMM); // Copy screen
	#elif defined(V9990)
		g9k_bitBlt(
			0, BACK_Y,		// Sx Sy
			0, 0,			// Dx Dy
			SCREEN_WIDTH, SCREEN_HEIGHT,// Nx Ny
			0,				// arg
			G9K_LOG_IMP,	// log
			0xffff,			// mask
			0,				// fc
			0,				// bg
			G9K_CMD_LMMM);	// Rectangle area data is transferred from VRAM to VRAM.
	#else
		#error "No BitBlt"
	#endif
}


inline static void gfxAux_copyPhysToBack()
{
	#ifdef V9958
		bitBlt(0, 0, 0, BACK_Y, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00, 0, CMD_HMMM); // Copy screen
	#elif defined(V9990)
		g9k_bitBlt(
			0, 0,			// Sx Sy
			0, BACK_Y,		// Dx Dy
			SCREEN_WIDTH, SCREEN_HEIGHT,// Nx Ny
			0,				// arg
			G9K_LOG_IMP,	// log
			0xffff,			// mask
			0,				// fc
			0,				// bg
			G9K_CMD_LMMM);	// Rectangle area data is transferred from VRAM to VRAM.
	#else
		#error "No BitBlt"
	#endif
}


inline static void gfxAux_clearPhys(uint16_t page_offset)
{
	#ifdef V9958
		bitBlt(0, 0, 0, page_offset, SCREEN_WIDTH, SCREEN_HEIGHT, getColor(0), 0, CMD_HMMV);
	#elif defined(V9990)
		g9k_bitBlt(
			0, 0,			// Sx Sy
			0, page_offset,	// Dx Dy
			SCREEN_WIDTH, SCREEN_HEIGHT,// Nx Ny
			0,				// arg
			G9K_LOG_IMP,	// log
			0xffff,			// mask
			getColor(0),	// fc
			0,				// bg
			G9K_CMD_LMMV);	// VRAM rectangle area is painted out.
	#else
		#error "No BitBlt"
	#endif
}


inline static void gfxAux_copyCurrentWindowTo(uint8_t routine)
{
	uint16_t cwX = cw->winX*FONTWIDTH, 
			 cwY = cw->winY*FONTHEIGHT, 
			 cwY2 = cwY;

	if (routine==GFX_CURRENT_WIN_TO_PHYS)
		cwY += BACK_Y;
	else
		cwY2 += BACK_Y;

	#ifdef V9958
		bitBlt(cwX, cwY, cwX, cwY2, cw->winW*FONTWIDTH, cw->winH*FONTHEIGHT, 0, 0, CMD_HMMM);	// Copy current Window
	#elif defined(V9990)
		g9k_bitBlt(
			cwX, cwY,		// Sx Sy
			cwX, cwY2,		// Dx Dy
			cw->winW*FONTWIDTH, cw->winH*FONTHEIGHT,// Nx Ny
			0,				// arg
			G9K_LOG_IMP,	// log
			0xffff,			// mask
			0,				// fc
			0,				// bg
			G9K_CMD_LMMM);	// Rectangle area data is transferred from VRAM to VRAM.
	#else
		#error "No BitBlt"
	#endif
}
