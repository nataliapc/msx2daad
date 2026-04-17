/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include "daad.h"

/*
 * Function: getObjectWeight
 * --------------------------------
 * Return the weight of a object by ID. Also can return 
 * the total weight of location or carried/worn objects
 * if objno is NULLWORD.
 *  
 * @param objno			Object ID or NULLWORD.
 * @param isCarriedWorn	Check carried/worn objects if True.
 * @return				Return the weight of one or a sum of objects.
 */
// Sum weights of all objects whose location == loc, descending recursively
// into nested containers. Works uniformly for player-held aggregates
// (LOC_CARRIED / LOC_WORN) and for the contents of a specific container.
// Per the WEIGH spec, a container of zero weight transmits zero weight for
// both itself and its contents ("magic" bag), so its contents are not summed.
static uint8_t _sumLocation(uint8_t loc)
{
	uint16_t weight = 0;
	Object *obj = objects;
	for (uint8_t i=0; i<hdr->numObjDsc; i++) {
		if (obj->location == loc) {
			uint8_t w = obj->attribs.mask.weight;
			weight += w;
			if (w > 0 && obj->attribs.mask.isContainer)
				weight += _sumLocation(i);
		}
		obj++;
	}
	return weight>255 ? 255 : (uint8_t)weight;
}

uint8_t getObjectWeight(uint8_t objno, bool isCarriedWorn)
{
	isCarriedWorn;	// only meaningful with NULLWORD; both callsites pass true there
	if (objno == NULLWORD)
		return _sumLocation(LOC_CARRIED) + _sumLocation(LOC_WORN);

	Object *obj = &objects[objno];
	uint16_t weight = obj->attribs.mask.weight;
	if (weight > 0 && obj->attribs.mask.isContainer)
		weight += _sumLocation(objno);
	return weight>255 ? 255 : (uint8_t)weight;
}
