/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include "daad.h"


/*
 * Function: getObjectId
 * --------------------------------
 * Return de object ID by Noun+Adjc ID.
 *  
 * @param noun		Noun ID.
 * @param adjc		Adjective ID, or NULLWORD to disable adjective filter.
 * @location		Location where the object must be, LOC_HERE to disable location filter, or LOC_CONTAINER if location is a container.
 * @return			Object ID if found or NULLWORD.
 */
uint8_t getObjectId(uint8_t noun, uint8_t adjc, uint16_t location)
{
	for (uint16_t i=0; i<hdr->numObjDsc; i++) {
		if (noun!=NULLWORD && objects[i].nounId==noun && 
		   (adjc==NULLWORD || objects[i].adjectiveId==adjc) && 							// If 'adjc' not needed or 'adjc' matchs
		   ((location==LOC_HERE || objects[i].location==location) ||					// It's in anywhere or placed in 'location'...
		    (location==LOC_CONTAINER && location<hdr->numObjDsc && 
										objects[location].attribs.mask.isContainer)))	// ...or if it's in a container
		{
			return i;
		}
	}
	return NULLWORD;
}

/*
 * Function: referencedObject
 * --------------------------------
 * Modify DAAD flags to reference the last object used
 * in a logical sentence.
 *  
 * @param objno		Object ID.
 * @return			none.
 */
void referencedObject(uint8_t objno) __z88dk_fastcall
{
	Object *objRef = objno==NULLWORD ? nullObject : &objects[objno];

	flags[fCONum] = objno;							// Flag 51
	flags[fCOLoc] = objRef->location;				// Flag 54
	flags[fCOWei] = objRef->attribs.mask.weight;	// Flag 55
	flags[fCOCon] = flags[fCOCon] & 0b01111111 | (objRef->attribs.mask.isContainer << 7);	// Flag 56
	flags[fCOWR]  = flags[fCOWR] & 0b01111111 | (objRef->attribs.mask.isWareable << 7);		// Flag 57
	flags[fCOAtt] = objRef->extAttr2;				// Flag 58
	flags[fCOAtt+1] = objRef->extAttr1;				// Flag 59
}
