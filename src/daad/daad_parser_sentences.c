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
#ifdef DAADV3
	bool verbSeen = false;
#endif

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
#ifdef DAADV3
				if (ISV3) {
					if (voc->type == VERB)        verbSeen = true;
					if (voc->type == CONJUNCTION) verbSeen = false;
				}
#endif
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
		if (!voc->word[0]) {
#ifdef DAADV3
			if (ISV3 && verbSeen) {
				*lsBuffer++ = 0;
				*lsBuffer++ = UNKNOWN_WORD;
				*lsBuffer = 0;
			}
#endif
#ifdef VERBOSE2
cprintf("NOT FOUND!\n");
#endif
		}
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
 * Function: populateLogicalSentence
 * --------------------------------
 * Set the flags with the current logical sentence.
 *
 * @return		Boolean with True if any logical sentence found.
 */
bool populateLogicalSentence()
{
	char *p, type, id, adj;
	bool ret;
	p = lsBuffer0;
	adj = fAdject1;
	ret = false;

	// Clear parser flags (fCPNoun/fCPAdject persist across sentences — spec DAAD 1991)
	flags[fVerb] = flags[fNoun1] = flags[fAdject1] = flags[fAdverb] = flags[fPrep] =
		flags[fNoun2] = flags[fAdject2] = NULLWORD;
#ifdef DAADV3
	if (ISV3) flags[fOFlags] &= ~(F53_UNRECWRD | F53_PREPFIRST);
#endif
#ifdef VERBOSE2
cputs("populateLogicalSentence()\n");
#endif
	while (*p && *(p+1)!=CONJUNCTION) {
		id = *p;
		type = *(p+1);
		if (type==VERB && flags[fVerb]==NULLWORD) {										// VERB
			flags[fVerb] = id;
			ret = true;
		} else if (type==NOUN && flags[fNoun1]==NULLWORD) {								// NOUN1
			// word that works like noun and verb
			if (id<20 && flags[fVerb]==NULLWORD) {
				flags[fVerb] = id;
			} else {
				// workd only like noun
				flags[fNoun1] = id;
			}
			ret = true;
		} else if (type==NOUN && flags[fNoun2]==NULLWORD) {								// NOUN2
			flags[fNoun2] = id;
			adj = fAdject2;
			ret = true;
		} else if (type==ADVERB && flags[fAdverb]==NULLWORD) {							// ADVERB
			flags[fAdverb] = id;
			ret = true;
		} else if (type==PREPOSITION && flags[fPrep]==NULLWORD) {						// PREP
			flags[fPrep] = id;
#ifdef DAADV3
			if (ISV3 && flags[fNoun1] == NULLWORD) flags[fOFlags] |= F53_PREPFIRST;
#endif
			ret = true;
		} else if (type==ADJECTIVE && adj==fAdject1 && flags[fAdject1]==NULLWORD) {		// ADJ1
			flags[fAdject1] = id;
			ret = true;
		} else if (type==ADJECTIVE && adj==fAdject2 && flags[fAdject2]==NULLWORD) {		// ADJ2
			flags[fAdject2] = id;
			ret = true;
		} else if (type==PRONOUN) {															// PRONOUN
			// Replace Noun1/Adject1 with stored pronoun if no noun was given yet
			if (flags[fNoun1]==NULLWORD && flags[fCPNoun]!=NULLWORD) {
				flags[fNoun1]   = flags[fCPNoun];
				flags[fAdject1] = flags[fCPAdject];
			}
			ret = true;
		}
#ifdef DAADV3
		  else if (type==UNKNOWN_WORD) {
			flags[fOFlags] |= F53_UNRECWRD;
		}
#endif
		p+=2;
	}

	if (flags[fNoun2]!=NULLWORD) {
		uint8_t obj = getObjectId(flags[fNoun2], flags[fAdject2], LOC_HERE);
		if (obj!=NULLWORD) {
			flags[fO2Num] = obj;
			flags[fO2Loc] = objects[obj].location;
			flags[fO2Con] = objects[obj].attribs.mask.isContainer << 7;
			flags[fO2Att]   = objects[obj].extAttr2;	// Consistent with fCOAtt (flag 58)
			flags[fO2Att+1] = objects[obj].extAttr1;	// Consistent with fCOAtt+1 (flag 59)
		} else {
			flags[fO2Num] = LOC_NOTCREATED;							// TODO: check default values when Object2 is undefined
			flags[fO2Loc] = LOC_NOTCREATED;
			flags[fO2Con] = flags[fO2Att] = flags[fO2Att+1] = 0;
		}
	}

	// Save non-proper noun as pronoun reference (spec DAAD 1991: id >= 50)
	if (flags[fNoun1]!=NULLWORD && flags[fNoun1]>=50) {
		flags[fCPNoun]   = flags[fNoun1];
		flags[fCPAdject] = flags[fAdject1];
	}
#ifdef VERBOSE2
cprintf("VERB:%u NOUN1:%u ADJ1:%u, ADVERB:%u PREP: %u NOUN2:%u, ADJ2:%u\n",
		flags[fVerb],flags[fNoun1],flags[fAdject1],flags[fAdverb],flags[fPrep],flags[fNoun2],flags[fAdject2]);
#endif
	nextLogicalSentence();

	return ret;
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

	while (*p!=0 && *(p+1)!=CONJUNCTION) p+=2;
	if (!*p) { *c++ = 0; *c = 0; return; }
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

