#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "daad.h"


// Constants
const char CHARS_MSX[]  = "\xA6\xAD\xA8\xAE\xAF\xA0\x82\xA1\xA2\xA3\xA4\xA5\x87\x80\x81\x9A";	//ª¡¿«»áéíóúñÑçÇüÜ
const char PIC_NAME[] = { '0','0','0','.','I','M','0'+SCREEN, '\0' };	// Picture filename
const char PLT_NAME[] = { '0','0','0','.','P','L','0'+SCREEN, '\0' };	// Palette filename

// External
extern void do_CLS();
extern void do_INKEY();

// Global variables
uint8_t *ddb;
DDB_Header *hdr;
Object *objects;

Z80_registers regs;


// Internal variables
uint8_t flags[256];
uint8_t lsBuffer[MAX_PROMPT_TEXT/2+1];	// Logical sentence buffer [type+id]
char    tmpTok[6];
char    tmpVoc[6];
char   *tmpMsg;							// MAX_TEXT_LEN
char    lastPrompt;
uint8_t offsetText;

Window  windows[8];						// 0-7 windows definitions
Window  *cw;							// Pointer to current active window
uint8_t savedPosX;
uint8_t savedPosY;



//=========================================================

bool initDDB()
{
	uint16_t *p;
	
	hdr = (DDB_Header*)ddb;
	p = (uint16_t *)&hdr->tokensPos;

#ifdef DEBUG
	printf("Version.......... %u\n", hdr->version);
	printf("Language......... %u\n", hdr->target.value.language);
	printf("Machine.......... %u\n", hdr->target.value.machine);
	printf("Magic............ 0x%02x\n", hdr->magic);
	printf("Num.Obj.......... %u\n", hdr->numObjDsc);
	printf("Num.Locations.... %u\n", hdr->numLocDsc);
	printf("Num.Usr.Msg...... %u\n", hdr->numUsrMsg);
	printf("Num.Sys.Msg...... %u\n", hdr->numSysMsg);
	printf("Num.Proc......... %u\n", hdr->numPrc);
	printf("Tokens pos....... 0x%04x\n", hdr->tokensPos);
	printf("Proc list pos.... 0x%04x\n", hdr->prcLstPos);
	printf("Obj. list pos.... 0x%04x\n", hdr->objLstPos);
	printf("Loc. list pos.... 0x%04x\n", hdr->locLstPos);
	printf("Usr. msg. pos.... 0x%04x\n", hdr->usrMsgPos);
	printf("Sys. msg. pos.... 0x%04x\n", hdr->sysMsgPos);
	printf("Connections pos.. 0x%04x\n", hdr->conLstPos);
	printf("Vocabulary pos... 0x%04x\n", hdr->vocPos);
	printf("Obj.Loc. list.... 0x%04x\n", hdr->objLocLst);
	printf("Obj.Name pos..... 0x%04x\n", hdr->objNamePos);
	printf("Obj.Attr pos..... 0x%04x\n", hdr->objAttrPos);
	printf("Obj.Extr pos..... 0x%04x\n", hdr->objExtrPos);
	printf("File length...... 0x%04x\n", hdr->fileLength);
#endif

	//If not a valid DDB version exits
	if (hdr->version != 2 || hdr->magic != 0x5f)
		return false;

	//Update header positions addresses
	for (int i=0; i<12; i++) {
		*(p++) += (uint16_t)ddb;
	}

	//Get memory for tmpMsg
	tmpMsg = (char*)malloc(MAX_TEXT_LEN);
	//Get memory for objects
	objects = (Object*)malloc(sizeof(Object)*hdr->numObjDsc);

	return true;
}

void initFlags()
{
	//Clear flags
	memset(flags, 0, 256);

	//Set screen flags
	#if SCREEN==5 || SCREEN==7
		flags[fScMode] = 13;	// EGA
	#endif
	#if SCREEN==6
		flags[fScMode] = 4;		// CGA
	#endif
	#if SCREEN==8
		flags[fScMode] = 141;	// VGA
	#endif

	//Initialize windows
	memset(windows, 0, sizeof(windows));
	flags[fCurWin] = 0;
	cw = &windows[0];
	cw->winX = 0;
	cw->winY = 0;
	cw->winW = MAX_COLUMNS;
	cw->winH = MAX_LINES;
	cw->cursorX = cw->cursorY = 0;
	savedPosX = savedPosY = 0;

	//Clear logical sentences
	clearLogicalSentences();

	offsetText = 0;
}

void initObjects()
{
	uint8_t  *objLoc = (uint8_t*)hdr->objLocLst;
	uint8_t  *attrLoc = (uint8_t*)hdr->objAttrPos;
	uint8_t  *extAttrLoc = (uint8_t*)hdr->objExtrPos;
	uint8_t  *nameObj = (uint8_t*)hdr->objNamePos;

	flags[fNOCarr] = 0;

	for (int i=0; i<hdr->numObjDsc; i++) {
		objects[i].location     = *(objLoc + i);
		objects[i].attribs.byte = *(attrLoc + i);
		objects[i].extAttr1     = *(extAttrLoc + i*2);
		objects[i].extAttr2     = *(extAttrLoc + i*2 + 1);
		objects[i].nounId       = *(nameObj + i*2);
		objects[i].adjectiveId  = *(nameObj + i*2 + 1);
		if (objects[i].location==LOC_CARRIED) flags[fNOCarr]++;
	}
}

void mainLoop()
{
	initFlags();
	initializePROC();

	pushPROC(0);
	processPROC();
}

void prompt()
{
	char c, *p = tmpMsg, *extChars;

	while (kbhit()) getchar();
	gfxPutCh('>');
	do {
		while (!kbhit());
		c = getchar();
		if (c=='\r' && p==tmpMsg) { c = 0; continue; }	// Avoid enter an empty text order
		if (c==0x08) {									// Back space (BS)
			if (p==tmpMsg) continue;
			p--;
			if (cw->cursorX>0) cw->cursorX--; else { cw->cursorX = cw->winW-1; cw->cursorY--; }
			gfxPutChPixels(' '-16);
		} else {
			if (p-tmpMsg > MAX_PROMPT_TEXT) continue;
			extChars = strchr(CHARS_MSX, c);
			if (extChars) c = (char)(extChars-CHARS_MSX+0x10);
			gfxPutCh(c);
			*p++ = toupper(c);
		}
	} while (c!='\r');
	*--p = '\0';
}

void parser()
{
	char *p = tmpMsg, *p2;
	uint8_t ils = 0;
	Vocabulary *voc;

	//Clear logical sentences buffer
	clearLogicalSentences();

	while (*p) {
		//Clear tmpTok
		memset(tmpTok, ' ', 5);

		//Copy first 5 chars max of word
		p2 = p;
		while (p2-p<5 && *p2!=' ' && *p2!='\0') p2++;
		memcpy(tmpTok, p, p2-p);
#ifdef VERBOSE2
printf("%u %c%c%c%c%c: ",p2-p, tmpTok[0],tmpTok[1],tmpTok[2],tmpTok[3],tmpTok[4]);
#endif
		for (int i=0; i<5; i++) tmpTok[i] = 255 - tmpTok[i];

		//Search it in VOCabulary table
		voc = (Vocabulary*)hdr->vocPos;
		while (voc->word[0]) {
			if (!memcmp(tmpTok, voc->word, 5)) {
				lsBuffer[ils++] = voc->id;
				lsBuffer[ils++] = voc->type;
#ifdef VERBOSE2
printf("Found! %u / %u\n",voc->id, voc->type);
#endif
				break;
			}
			voc++;
		}
#ifdef VERBOSE2
if (!voc->word[0]) printf("NOT FOUND!\n");
#endif
		p = strchr(p2, ' ');
		if (!p) break;
		p++;
	}
#ifdef VERBOSE2
printf("%02u %02u %02u %02u %02u %02u %02u %02u \n",lsBuffer[0],lsBuffer[1],lsBuffer[2],lsBuffer[3],lsBuffer[4],lsBuffer[5],lsBuffer[6],lsBuffer[7]);
#endif
}

bool getLogicalSentence()
{
	char *p = lsBuffer, type, id, adj = fAdject1;
	bool ret = false;

	// If not logical sentences in buffer we ask to user again
	if (!*p) {
		char newPrompt;

		newPrompt = flags[fPrompt];
		if (!newPrompt)
			while ((newPrompt=(rand()%4)+2)==lastPrompt);
		gfxPutsln(getSystemMsg(newPrompt));
		lastPrompt = newPrompt;

		prompt();
		parser();
	}

	// Clear parser flags
	flags[fVerb] = flags[fNoun1] = flags[fAdject1] = flags[fAdverb] = flags[fPrep] = flags[fNoun2] = flags[fAdject2] = 
		flags[fCPNoun] = flags[fCPAdject] = NULLWORD;
#ifdef VERBOSE2
printf("populateLogicalSentence()\n");
#endif
	while (*p && *(p+1)!=CONJUNCTION) {
		id = *p;
		type = *(p+1);
		if (type==VERB && flags[fVerb]==NULLWORD) {										// VERB
			flags[fVerb] = id;
			ret = true;
		} else if (type==NOUN && flags[fNoun1]==NULLWORD) {								// NOUN1
			flags[fNoun1] = id;
			if (id<20) flags[fVerb] = id;
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
#ifdef VERBOSE2
printf("VERB:%u NOUN1:%u ADJ1:%u, ADVERB:%u PREP: %u NOUN2:%u, ADJ2:%u %u\n",flags[fVerb],flags[fNoun1],flags[fAdject1],flags[fAdverb],flags[fPrep],flags[fNoun2],flags[fAdject2]);
#endif
	nextLogicalSentence();

	return ret;
}

void clearLogicalSentences()
{
#ifdef VERBOSE2
printf("clearLogicalSentence()\n");
#endif
	memset(lsBuffer, 0, sizeof(lsBuffer));
}

void nextLogicalSentence()
{
#ifdef VERBOSE2
printf("nextLogicalSentence()\n");
#endif
	char *p = lsBuffer, *c = lsBuffer;
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

void printBase10(uint16_t value)
{
	if (value<10) {
		if (value) gfxPutCh('0'+(uint8_t)value);
		return;
	}
	printBase10(value/10);
	gfxPutCh('0'+(uint8_t)(value%10));
}

//=========================================================

char* getToken(uint8_t num)
{
	char *p = (char*)hdr->tokensPos + 1;
	char i=0;

	while (num) {
		if (*p > 127) num--;
		p++;
		if (!num) break;
	}
	do {
		tmpTok[i++] = *p & 0x7f;
	} while (*p++ < 127);
	tmpTok[i]='\0';

	return tmpTok;
}

/*
char* getVocabulary(uint8_t num)
{
	char *p = (char*)hdr->vocPos + (num*7);
	char i=0;

	while (i<5) {
		if (*p==0xdf) break;	// (255-32)=space char
		tmpVoc[i++] = 255 - *p;
		p++;
	}
	tmpVoc[i] = '\0';

	return tmpVoc;
}

char getVocabularyID(uint8_t num)
{
	return ((Vocabulary*)hdr->vocPos + num)->id;
}

char getVocabularyType(uint8_t num)
{
	return ((Vocabulary*)hdr->vocPos + num)->type;
}
*/

char* _getMsg(uint16_t *lst, uint8_t num)
{
	char *p = &ddb[*(lst + num)];
	uint16_t i = 0;
	char c;

	tmpMsg[0]='\0';
	do {
		c = 255 - *p++;
		if (c >= 128) {
			tmpMsg[i] = '\0';
			strcat(&tmpMsg[i], getToken(c - 128));
			while (tmpMsg[i]) i++;
		} else {
			tmpMsg[i++] = c;
		}
		#ifdef DEBUG
		if (i>MAX_TEXT_LEN) die("Message exceeds limits of 'tmpMsg' variable!");
		#endif
	} while (c != 0x0a);		// = 255 - 0xf5
	tmpMsg[--i] = '\0';

	return tmpMsg;
}

char* getSystemMsg(uint8_t num)
{
	return _getMsg((uint16_t*)hdr->sysMsgPos, num);
}

char* getUserMsg(uint8_t num)
{
	return _getMsg((uint16_t*)hdr->usrMsgPos, num);
}

char* getObjectMsg(uint8_t num)
{
	return _getMsg((uint16_t*)hdr->objLstPos, num);
}

char* getLocationMsg(uint8_t num)
{
	return _getMsg((uint16_t*)hdr->locLstPos, num);
}

uint8_t getObjectById(uint8_t noun, uint8_t adjc)
{
	for (int i=0; i<hdr->numObjDsc; i++) {
		if (objects[i].nounId==noun && objects[i].adjectiveId==adjc)
			return i;
	}
	return NULLWORD;
}

uint8_t getObjectWeight(uint8_t objno, bool isCarriedWorn)
{
	uint16_t weight = 0;
	Object *obj = objects;
	for (int i=0; i<hdr->numObjDsc; i++) {
		if ((objno==NULLWORD || objno==i) && (!isCarriedWorn || obj->location==LOC_CARRIED || obj->location==LOC_WORN)) {
			if (obj->attribs.mask.isContainer && obj->attribs.mask.weight!=0) {
				weight += getObjectWeight(i, false);
			}
			weight += obj->attribs.mask.weight;
		}
		obj++;
	}
	return weight>255 ? 255 : (uint8_t)weight;
}

void referencedObject(uint8_t objno)
{
	flags[fCONum] = objno;
	flags[fCOLoc] = objects[objno].location;
	flags[fCOWei] = objects[objno].attribs.mask.weight;
	flags[fCOCon] = flags[fCOCon] & 0b01111111 | objects[objno].attribs.mask.isContainer << 7;
	flags[fCOWR]  = flags[fCOWR] & 0b01111111 | objects[objno].attribs.mask.isWareable << 7;
	flags[fCOAtt] = objects[objno].extAttr1;
	flags[fCOAtt+1] = objects[objno].extAttr2;
}

//=========================================================

#if SCREEN!=6
	#define COLOR_INK		15
#else
	#define COLOR_INK		3
#endif
#define COLOR_PAPER		0


uint16_t colorTranslation[] = {	// EGA rgb -> MSX grb
	0x000, //000 black
	0x006, //006 blue
	0x600, //060 green
	0x606, //066 dark cyan
	0x060, //600 red
	0x066, //606 dark purple
	0x260, //620 orange
	0x666, //666 light gray
	0x222, //222 dark gray
	0x337, //337 light blue
	0x722, //272 light green
	0x727, //277 light cyan
	0x272, //722 light red
	0x277, //727 light purple
	0x772, //772 yellow
	0x777  //777 white
};


void gfxSetScreen()
{
	//Set Color 15,0,0
	setColor(15, 0, 0);
	//Set SCREEN mode
	regs.Bytes.A = SCREEN;
	BiosCall(0x5f, &regs, REGS_ALL);
	enable212lines();
	enable50Hz();
	disableSPR();
}

void gfxClearLines(uint16_t start, uint16_t lines)
{
	bitBlt(0, 0, 0, start, SCREEN_WIDTH, lines, 0x00, 0, CMD_HMMV);
}

void gfxClearScreen()
{
	gfxClearLines(0, 212);
}

void gfxClearWindow()
{
	bitBlt(0, 0, cw->winX*FONTWIDTH, cw->winY*8, (cw->winW+1)*FONTWIDTH, cw->winH*8, 0x00, 0, CMD_HMMV);
}

void gfxSetPaperCol(uint8_t col)
{
	setColorPal(COLOR_PAPER, colorTranslation[col]);
}

void gfxSetInkCol(uint8_t col)
{
	setColorPal(COLOR_INK, colorTranslation[col]);
}

void gfxSetBorderCol(uint8_t col)
{
	gfxSetPaperCol(col);
}

void gfxPutChPixels(char c)		// c = ASCII-0x10
{
	bitBlt((c*8)%SCREEN_WIDTH, 256+(c>>(SCREEN_WIDTH/256+4)<<3), (cw->cursorX+cw->winX)*FONTWIDTH, (cw->cursorY+cw->winY)*8, FONTWIDTH, 8, 0x00, 0, CMD_LMMM);
}

void gfxPutCh(char c)
{
	switch (c) {
		case 11:		// \b
			do_CLS(); return;
		case 12:		// \k
			do_INKEY(); return;
		case 14:		// \g
			offsetText = 128; return;
		case 15:		// \t
			offsetText = 0; return;
	}
	if (c=='\r' || c=='\n') {
		cw->cursorX = 0;
		cw->cursorY++;
	} else 
	if (!(c==' ' && cw->cursorX==0)) {
		c -= 16;
		gfxPutChPixels(offsetText + c);
		cw->cursorX++;
		if (cw->cursorX >= cw->winW) {
			cw->cursorX = 0;
			cw->cursorY++;
		}
	}
	if (cw->cursorY >= cw->winH) {
		gfxScrollUp();
		cw->cursorX = 0;
		cw->cursorY--;
	}
}

void gfxPuts(char *str)
{
#ifdef VERBOSE
printf("================================================\n%s\n================================================\n", str);
#endif
	char *aux, c;
	while ((c = *str)) {
		if (c==' ') {
			aux = str+1;
			while (*aux && *aux!=0x20 && *aux!='\r' && *aux!='\n') {
				aux++;
			}
			if (cw->cursorX+(aux-(str+1)) >= cw->winW) {
				c = '\n';
			}
		}
		gfxPutCh(c);
		str++;
	}
}

void gfxPutsln(char *str)
{
	gfxPuts(str);
	gfxPutCh('\r');
}

void gfxScrollUp()
{
	bitBlt(0, cw->winY*8+8, cw->winX*6, cw->winY*8, cw->winW*6, cw->winH*8-8, 0x00, 0, CMD_YMMM);
	bitBlt(0, 0, cw->winX*6, (cw->winY+cw->winH-1)*8, cw->winW*6, 8, 0x00, 0, CMD_HMMV);
}

bool gfxPicturePrepare(uint8_t location)
{
	char *pic = PIC_NAME, *plt = PLT_NAME;

	pic[2] = plt[2] = location%10 + '0';
	location /= 10;
	pic[1] = plt[1] = location%10 + '0';
	location /= 10;
	pic[0] = plt[0] = location%10 + '0';

	return fileexists(PIC_NAME);
}

void gfxPictureShow()
{
	uint16_t fp;
	uint8_t i = cw->winH;
	char *buffer = heap_top;

	do_CLS();	//TODO clear only lines will be read from file

	#if SCREEN!=8
		fp = fopen(PLT_NAME, O_RDONLY);
		if (!(fp&0xff00)) {
			fread(buffer, 32, fp);
			setPalette(buffer);
			fclose(fp);
		}
	#endif

	fp = fopen(PIC_NAME, O_RDONLY);
	if (fp<0xff00) {
//		fseek(fp, 7+76*256, SEEK_CUR);
/*
		uint16_t chunk;
		uint16_t size;
		setVDP_Write(0x00000);
		do {
			size = fread(&chunk, 2, fp);
printf("size:%u chunk:%u\n",size,chunk);
			if (!(size & 0xff00)) {
				fread(buffer, chunk, fp);
printf("# size:%u chunk:%u\n",size,chunk);
				dzx7vram(buffer, 0);
			}
		} while (!(size & 0xff00));
*/
		setVDP_Write(0x00000);
		while (fread(buffer, MAX_PICFILE_READ, fp)==MAX_PICFILE_READ) {
			__asm
				push hl
				push bc
				ld hl,(#_heap_top)
				ld bc,#0x0098
				otir
				otir
				otir
				otir
				otir
				otir
				otir
				otir
				pop bc
				pop hl
			__endasm;
		}

		fclose(fp);
	}
}
