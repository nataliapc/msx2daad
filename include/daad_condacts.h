/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.
*/
#include "daad_defines.h"

#ifndef __DAAD_CONDACTS_H__
#define __DAAD_CONDACTS_H__


// Condact functions definitions
static void do_AT();
static void do_NOTAT();
static void do_ATGT();
static void do_ATLT();
static void do_PRESENT();
static void do_ABSENT();
static void do_WORN();
static void do_NOTWORN();
static void do_CARRIED();
static void do_NOTCARR();
static void do_CHANCE();
static void do_ZERO();
static void do_NOTZERO();
static void do_EQ();
static void do_GT();
static void do_LT();
static void do_ADJECT1();
static void do_ADVERB();
static void do_SFX();
static void do_DESC();
static void do_QUIT();
static void do_END();
static void do_DONE();
static void do_OK();
static void do_ANYKEY();
static void do_SAVE();
static void do_LOAD();
static void do_DPRINT();
static void do_DISPLAY();
void do_CLS();
static void do_DROPALL();
static void do_AUTOG();
static void do_AUTOD();
static void do_AUTOW();
static void do_AUTOR();
static void do_PAUSE();
static void do_SYNONYM();
static void do_GOTO();
static void do_MESSAGE();
static void do_REMOVE();
static void do_GET();
static void do_DROP();
static void do_WEAR();
static void do_DESTROY();
static void do_CREATE();
static void do_SWAP();
static void do_PLACE();
static void do_SET();
static void do_CLEAR();
static void do_PLUS();
static void do_MINUS();
static void do_LET();
void do_NEWLINE();
static void do_PRINT();
static void do_SYSMESS();
static void do_ISAT();
static void do_SETCO();
static void do_SPACE();
static void do_HASAT();
static void do_HASNAT();
static void do_LISTOBJ();
static void do_EXTERN();
static void do_RAMSAVE();
static void do_RAMLOAD();
static void do_BEEP();
static void do_PAPER();
static void do_INK();
static void do_BORDER();
static void do_PREP();
static void do_NOUN2();
static void do_ADJECT2();
static void do_ADD();
static void do_SUB();
static void do_PARSE();
static void do_LISTAT();
static void do_PROCESS();
static void do_SAME();
static void do_MES();
static void do_WINDOW();
static void do_NOTEQ();
static void do_NOTSAME();
static void do_MODE();
static void do_WINAT();
static void do_TIME();
static void do_PICTURE();
static void do_DOALL();
static void do_MOUSE();
static void do_GFX();
static void do_ISNOTAT();
static void do_WEIGH();
static void do_PUTIN();
static void do_TAKEOUT();
static void do_NEWTEXT();
static void do_ABILITY();
static void do_WEIGHT();
static void do_RANDOM();
static void do_INPUT();
static void do_SAVEAT();
static void do_BACKAT();
static void do_PRINTAT();
static void do_WHATO();
static void do_CALL();
static void do_PUTO();
static void do_NOTDONE();
static void do_AUTOP();
static void do_AUTOT();
static void do_MOVE();
static void do_WINSIZE();
static void do_REDO();
static void do_CENTRE();
static void do_EXIT();
static void do_INKEY();
static void do_BIGGER();
static void do_SMALLER();
static void do_ISDONE();
static void do_ISNDONE();
static void do_SKIP();
static void do_RESTART();
static void do_TAB();
static void do_COPYOF();
static void do_COPYOO();
static void do_COPYFO();
static void do_COPYFF();
static void do_COPYBF();
static void do_RESET();
static void do_NOT_USED();

#ifdef VERBOSE
typedef struct
{
	char name[12];
	unsigned char args;
} CondactArgs;
#endif	//VERBOSE

typedef struct {
	void (* const function)();
	uint8_t flag;					//0:not count for ISDONE | 1:count for ISDONE
} CONDACT_LIST;


#endif //__DAAD_CONDACTS_H__
