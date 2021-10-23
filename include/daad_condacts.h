/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.
*/
#include "daad_defines.h"

#ifndef __DAAD_CONDACTS_H__
#define __DAAD_CONDACTS_H__

#define NUM_PROCS	10

// Condact functions definitions
void do_AT();
void do_NOTAT();
void do_ATGT();
void do_ATLT();
void do_PRESENT();
void do_ABSENT();
void do_WORN();
void do_NOTWORN();
void do_CARRIED();
void do_NOTCARR();
void do_CHANCE();
void do_ZERO();
void do_NOTZERO();
void do_EQ();
void do_GT();
void do_LT();
void do_ADJECT1();
void do_ADVERB();
void do_SFX();
void do_DESC();
void do_QUIT();
void do_END();
void do_DONE();
void do_OK();
void do_ANYKEY();
void do_SAVE();
void do_LOAD();
void do_DPRINT();
void do_DISPLAY();
void do_CLS();
void do_DROPALL();
void do_AUTOG();
void do_AUTOD();
void do_AUTOW();
void do_AUTOR();
void do_PAUSE();
void do_SYNONYM();
void do_GOTO();
void do_MESSAGE();
void do_REMOVE();
void do_GET();
void do_DROP();
void do_WEAR();
void do_DESTROY();
void do_CREATE();
void do_SWAP();
void do_PLACE();
void do_SET();
void do_CLEAR();
void do_PLUS();
void do_MINUS();
void do_LET();
void do_NEWLINE();
void do_PRINT();
void do_SYSMESS();
void do_ISAT();
void do_SETCO();
void do_SPACE();
void do_HASAT();
void do_HASNAT();
void do_LISTOBJ();
void do_EXTERN();
void do_RAMSAVE();
void do_RAMLOAD();
void do_BEEP();
void do_PAPER();
void do_INK();
void do_BORDER();
void do_PREP();
void do_NOUN2();
void do_ADJECT2();
void do_ADD();
void do_SUB();
void do_PARSE();
void do_LISTAT();
void do_PROCESS();
void do_SAME();
void do_MES();
void do_WINDOW();
void do_NOTEQ();
void do_NOTSAME();
void do_MODE();
void do_WINAT();
void do_TIME();
void do_PICTURE();
void do_DOALL();
void do_MOUSE();
void do_GFX();
void do_ISNOTAT();
void do_WEIGH();
void do_PUTIN();
void do_TAKEOUT();
void do_NEWTEXT();
void do_ABILITY();
void do_WEIGHT();
void do_RANDOM();
void do_INPUT();
void do_SAVEAT();
void do_BACKAT();
void do_PRINTAT();
void do_WHATO();
void do_CALL();
void do_PUTO();
void do_NOTDONE();
void do_AUTOP();
void do_AUTOT();
void do_MOVE();
void do_WINSIZE();
void do_REDO();
void do_CENTRE();
void do_EXIT();
void do_INKEY();
void do_BIGGER();
void do_SMALLER();
void do_ISDONE();
void do_ISNDONE();
void do_SKIP();
void do_RESTART();
void do_TAB();
void do_COPYOF();
void do_COPYOO();
void do_COPYFO();
void do_COPYFF();
void do_COPYBF();
void do_RESET();
void do_NOT_USED();

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


#ifdef UNIT_TESTS
	bool *pIndirection();
	bool *pCheckEntry();
	bool *pIsDone();
	bool *pLastIsDone();
	bool *pLastPicShow();
#endif


#endif //__DAAD_CONDACTS_H__
