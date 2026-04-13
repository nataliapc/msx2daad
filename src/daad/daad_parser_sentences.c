/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include <string.h>
#include "daad.h"


// Internal variables
uint8_t lsBuffer0[TEXT_BUFFER_LEN/2+1];	// Logical sentence buffer [type+id] for PARSE 0
uint8_t lsBuffer1[TEXT_BUFFER_LEN/4+1];	// Logical sentence buffer [type+id] for PARSE 1

/*
 * Function: parser
 * --------------------------------
 * Parse the words in user entry text and compare
 * them with VOCabulary table.
 *
 * @return			none.
 */
void parser()
{
	uint8_t i;
	char *tmpVOC, *p, *p2;
	Vocabulary *voc;
	uint8_t *lsBuffer, *aux;
	lsBuffer = lsBuffer0;
	aux = lsBuffer1;
	p = tmpMsg;
	tmpVOC = safeMemoryAllocate();

	//Clear logical sentences buffer
	clearLogicalSentences();

	while (*p) {
		//Clear tmpVOC
		memset(tmpVOC, ' ', 5);

		if (*p=='"') {
			aux = lsBuffer;
			lsBuffer = lsBuffer1;
			p++;
#ifdef VERBOSE2
cprintf("DETECTED START of literal phrase!\n");
#endif
		}

		//Copy first 5 chars max of word
		p2 = p;
		while (p2-p<5 && *p2!=' ' && *p2!='"' && *p2!='\0') p2++;
		memcpy(tmpVOC, p, p2-p);
#ifdef VERBOSE2
cprintf("%u %c%c%c%c%c: ",p2-p, tmpVOC[0],tmpVOC[1],tmpVOC[2],tmpVOC[3],tmpVOC[4]);
#endif
		for (i=0; i<5; i++) tmpVOC[i] = 255 - tmpVOC[i];

		//Search it in VOCabulary table
		voc = (Vocabulary*)hdr->vocPos;
		while (voc->word[0]) {
			if (!memcmp(tmpVOC, voc->word, 5)) {
				*lsBuffer++ = voc->id;
				*lsBuffer++ = voc->type;
				*lsBuffer = 0;
#ifdef VERBOSE2
cprintf("Found! %u / %u [%c%c%c%c%c]\n",voc->id, voc->type, 255-voc->word[0], 255-voc->word[1], 255-voc->word[2], 255-voc->word[3], 255-voc->word[4]);
#endif
				break;
			}
			voc++;
		}
#ifdef VERBOSE2
if (!voc->word[0]) cprintf("NOT FOUND!\n");
#endif
		while (*p!=' ' && *p!='\0') {
			if (*p=='"') {
				lsBuffer = aux;
#ifdef VERBOSE2
cprintf("DETECTED END of literal phrase!\n");
#endif
			}
			p++;
		}
		while (*p!='\0' && *p==' ') p++;
	}
	safeMemoryDeallocate(tmpVOC);
#ifdef VERBOSE2
cprintf("%u %u %u %u %u %u %u %u \n",lsBuffer[0],lsBuffer[1],lsBuffer[2],lsBuffer[3],lsBuffer[4],lsBuffer[5],lsBuffer[6],lsBuffer[7]);
#endif
}

/*
 * Function: getLogicalSentence
 * --------------------------------
 * Get the first logical sentence from parsed user entry 
 * and fill noun, verbs, adjectives, etc.
 * If no sentences prompt to user.
 * 
 * @return		Boolean with True if any logical sentence found.
 */
bool getLogicalSentence()
{
	// If not logical sentences in buffer we ask the user again
	if (!*lsBuffer0) {
		prompt(true);
		parser();
	}
	return populateLogicalSentence();
}

/*
 * Function: useLiteralSentence
 * --------------------------------
 * Copy sentence between "..." if any typed.
 *
 * @return			Boolean with True if any logical sentence found.
 */
bool useLiteralSentence()
{
	memcpy(lsBuffer0, lsBuffer1, sizeof(lsBuffer1));
	return populateLogicalSentence();
}

/*
 * Function: clearLogicalSentences
 * --------------------------------
 * Clear pending logical sentences if any.
 * 
 * @return			none.
 */
void clearLogicalSentences()
{
#ifdef VERBOSE2
cputs("clearLogicalSentences()\n");
#endif
	memset(lsBuffer0, 0, sizeof(lsBuffer0));
	memset(lsBuffer1, 0, sizeof(lsBuffer1));
}

/*
 * Function: nextLogicalSentence
 * --------------------------------
 * Move next logical sentence to start of logical 
 * sentence buffer.
 * 
 * @return			none.
 */
void nextLogicalSentence()
{
#ifdef VERBOSE2
cputs("nextLogicalSentence()\n");
#endif
	char *p, *c;
	p = lsBuffer0;
	c = lsBuffer0;

	while (*p!=CONJUNCTION && *p!=0) p+=2;
	p+=2;
	for (;;) {
		*c++ = *p;
		*c++ = *(p+1);
		if (!*p) break;
		p+=2;
	}
	*c++ = 0;
	*c = 0;
}

