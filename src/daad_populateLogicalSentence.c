/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio

	NOTE: populateLogicalSentence lives here (top-level, direct .rel) instead of
	inside daad.lib. When placed as a member of daad.lib the game hangs on the
	first ANYKEY prompt ("Pulsa una tecla para continuar"), even though the
	compiled ASM is byte-identical. None of the SDCC 4.1.0 optimization flags
	(--nolospre, --nogcse, plain -O0) change the symptom. Pending deeper
	investigation of the sdar/sdld library handling in this SDCC version.
*/
#include <string.h>
#include "daad.h"


// Internal variables
extern uint8_t lsBuffer0[TEXT_BUFFER_LEN/2+1];	// Logical sentence buffer [type+id] for PARSE 0
extern uint8_t lsBuffer1[TEXT_BUFFER_LEN/4+1];	// Logical sentence buffer [type+id] for PARSE 1

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

	// Clear parser flags
	flags[fVerb] = flags[fNoun1] = flags[fAdject1] = flags[fAdverb] = flags[fPrep] = flags[fNoun2] = flags[fAdject2] =
		flags[fCPNoun] = flags[fCPAdject] = NULLWORD;
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
			ret = true;
		} else if (type==ADJECTIVE && adj==fAdject1 && flags[fAdject1]==NULLWORD) {		// ADJ1
			flags[fAdject1] = id;
			ret = true;
		} else if (type==ADJECTIVE && adj==fAdject2 && flags[fAdject2]==NULLWORD) {		// ADJ2
			flags[fAdject2] = id;
			ret = true;
		}
		p+=2;
	}

	if (flags[fNoun2]!=NULLWORD) {
		uint8_t obj = getObjectId(flags[fNoun2], flags[fAdject2], LOC_HERE);
		if (obj!=NULLWORD) {
			flags[fO2Num] = obj;
			flags[fO2Loc] = objects[obj].location;
			flags[fO2Con] = objects[obj].attribs.mask.isContainer << 7;
			flags[fO2Att] = objects[obj].extAttr1;
			flags[fO2Att+1] = objects[obj].extAttr2;
		} else {
			flags[fO2Num] = LOC_NOTCREATED;							// TODO: check default values when Object2 is undefined
			flags[fO2Loc] = LOC_NOTCREATED;
			flags[fO2Con] = flags[fO2Att] = flags[fO2Att+1] = 0;
		}
	}
#ifdef VERBOSE2
cprintf("VERB:%u NOUN1:%u ADJ1:%u, ADVERB:%u PREP: %u NOUN2:%u, ADJ2:%u\n",
		flags[fVerb],flags[fNoun1],flags[fAdject1],flags[fAdverb],flags[fPrep],flags[fNoun2],flags[fAdject2]);
#endif
	nextLogicalSentence();

	return ret;
}
