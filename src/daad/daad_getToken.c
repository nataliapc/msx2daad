/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include "daad.h"


/*
 * Function: getToken
 * --------------------------------
 * Return the requested token.
 * 
 * @param num   	To get the token number 'num' in the token list.
 * @return			Return a pointer to the requested token.
 */
char* getToken(uint8_t num) __z88dk_fastcall
{
	char *p;
	uint8_t i;
	p = (char*)hdr->tokensPos;
	i=0;

	// Skip previous tokens
	while (num) {
		if (*p > 127) num--;
		p++;
		if (!num) break;
	}
	// Copy selected token
	do {
		tmpTok[i++] = *p & 0x7f;
	} while (*p++ < 127);
	tmpTok[i]='\0';

	return tmpTok;
}
