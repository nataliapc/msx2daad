/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include <string.h>
#include "daad.h"


// Internal variables
static const uint8_t nullObjFake[] = { 0, 0, 0, 0, 0, 0 };

/*
 * Function: initDAAD
 * --------------------------------
 * Initialize DDB and DAAD engine.
 * 
 * @return			none.
 */
bool initDAAD(int argc, char **argv)
{
	uint16_t *p;
	
	loadFilesBin(argc, argv);

	hdr = (DDB_Header*)ddb;
	p = (uint16_t *)&hdr->tokensPos;

	#ifdef DEBUG
		cprintf("Version.......... %u\n"
				"Machine.......... %u\n"
				"Language......... %u\n"
				"Magic............ 0x%x\n"
				"Num.Obj.......... %u\n"
				"Num.Locations.... %u\n"
				"Num.Usr.Msg...... %u\n"
				"Num.Sys.Msg...... %u\n"
				"Num.Proc......... %u\n"
				"Tokens pos....... 0x%x\n"
				"Proc list pos.... 0x%x\n"
				"Obj. list pos.... 0x%x\n"
				"Loc. list pos.... 0x%x\n"
				"Usr. msg. pos.... 0x%x\n"
				"Sys. msg. pos.... 0x%x\n"
				"Connections pos.. 0x%x\n"
				"Vocabulary pos... 0x%x\n"
				"Obj.Loc. list.... 0x%x\n"
				"Obj.Name pos..... 0x%x\n"
				"Obj.Attr pos..... 0x%x\n"
				"Obj.Extr pos..... 0x%x\n\n",
				hdr->version, hdr->target.value.machine, hdr->target.value.language, hdr->magic, 
				hdr->numObjDsc, hdr->numLocDsc, hdr->numUsrMsg, hdr->numSysMsg, hdr->numPrc, 
				hdr->tokensPos, hdr->prcLstPos, hdr->objLstPos, hdr->locLstPos, hdr->usrMsgPos, 
				hdr->sysMsgPos, hdr->conLstPos, hdr->vocPos, hdr->objLocLst, hdr->objNamePos, 
				hdr->objAttrPos, hdr->objExtrPos);
	#endif

#ifdef DAADV3
	isV3 = (hdr->version == 3);
	//If not a valid DDB version exits
	if (hdr->version != 2 && !isV3)
		return false;
#else
	if (hdr->version != 2)
		return false;
#endif

	//Update header positions addresses
	for (uint8_t i=0; i<12; i++) {
		*(p++) += (uint16_t)ddb;
	}

	//Skip first token
	while ((*(char*)(hdr->tokensPos++) & 0x80) == 0);

	//Get memory for RAMSAVE
	ramsave = (char*)malloc(512);	// 256 bytes for Flags + 256 bytes for Objects location
	memset(ramsave, 0, 512);
	//Get memory for objects
	objects = (Object*)malloc(sizeof(Object)*hdr->numObjDsc);
	nullObject = (Object*)nullObjFake;
	//Get memory for tmpTok & tmpMsg
	tmpTok = (char*)malloc(32);
	tmpMsg = (char*)malloc(TEXT_BUFFER_LEN);

	checkPrintedLines_inUse = false;

	#if defined(DEBUG) || defined(TEST)
		cprintf("File length...... %u bytes\n"
				"DDB max size..... %u bytes\n\n", hdr->fileLength, getFreeMemory());
	#endif

	#ifdef TRANSCRIPT
		trIdx = 0;
	#endif

	return true;
}

/*
 * Function: initFlags
 * --------------------------------
 * Initialize DAAD flags and some structs.
 * 
 * @return			none.
 */
void initFlags()
{
	uint8_t i;

	//Clear all flags
	memset(flags, 0, 256);

	//Initialize screen flags [fGFlags] & [fScMode]
	gfxSetScreenModeFlags();

	//Initialize DAAD windows
	windows = malloc(sizeof(Window)*WINDOWS_NUM);
	memset(windows, 0, sizeof(Window)*WINDOWS_NUM);
	flags[fCurWin] = 0;
	cw = windows;
	for (i=0; i<WINDOWS_NUM; i++, cw++) {
		cw->winX = cw->winY = 0;
		cw->winW = MAX_COLUMNS;
		cw->winH = MAX_LINES;
		cw->lastPicLocation = 256;
		gfxSetPaperCol(0);
		gfxSetInkCol(15);
	}
	cw = windows;
	#ifndef DISABLE_SAVEAT
		savedPosX = savedPosY = 0;
	#endif

	//Initialize last onscreen picture
	lastPicShow = false;

	//Clear logical sentences
	clearLogicalSentences();

	doingPrompt = false;
	printedLines = 0;
	checkPrintedLines_inUse = false;
}

/*
 * Function: initObjects
 * --------------------------------
 * Initialize Objects.
 * 
 * @return			none.
 */
void initObjects()
{
	uint8_t *objLoc = (uint8_t*)hdr->objLocLst,
			*attrLoc = (uint8_t*)hdr->objAttrPos,
			*extAttrLoc = (uint8_t*)hdr->objExtrPos,
			*nameObj = (uint8_t*)hdr->objNamePos;

	flags[fNOCarr] = 0;

	for (int i=0; i < hdr->numObjDsc; i++) {
		objects[i].location     = *(objLoc++);
		objects[i].attribs.byte = *(attrLoc++);
		objects[i].extAttr1     = *(extAttrLoc++);
		objects[i].extAttr2     = *(extAttrLoc++);
		objects[i].nounId       = *(nameObj++);
		objects[i].adjectiveId  = *(nameObj++);
		if (objects[i].location==LOC_CARRIED) flags[fNOCarr]++;
	}
}
