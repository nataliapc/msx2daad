/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include "daad.h"


/*
 * Function: transcript
 * --------------------------------
 * Store the game texts and sentences in a file named "TRANSCR.TXT"
 *  
 * @param c			Char to store.
 * @return			none.
 */
#ifdef TRANSCRIPT
static const uint8_t transcript_translate[] = {
	"ª¡¿«»áéíóúñÑçÇüÜ"
};

static const char transcript_filename[] = "TRANSCR.TXT";
static char transcript_buff[1024];

void transcript_flush()
{
	uint16_t fp;
	uint32_t size;

	size = filesize(transcript_filename);
	if (size>=0xff00)
		fp = fcreate(transcript_filename, 0, 0);
	else {
		fp = fopen(transcript_filename, O_WRONLY);
		fseek(fp, size, SEEK_SET);
	}
	if (fp<0xff00) {
		fputs(transcript_buff, fp);
		fclose(fp);
	}
	trIdx = 0;
	transcript_buff[0] = '\0';
}

void transcript_char(char c) __z88dk_fastcall
{
	char *utf;

	if (c == 127) {
		transcript_buff[trIdx++] = ' ';
	} else
	if (c < 16) {
		transcript_buff[trIdx++] = '\n';
	} else
	if (c < 32) {
		utf = transcript_translate+((c-16)<<1);
		transcript_buff[trIdx++] = *utf++;
		transcript_buff[trIdx++] = *utf;
	} else {
		transcript_buff[trIdx++] = c;
	}
	transcript_buff[trIdx] = 0;

	if (trIdx>1020) {
		transcript_flush();
	}
}

#endif
