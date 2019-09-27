#include "daad_defines.h"

#ifndef __DAAD_CONDACTS_H__
#define __DAAD_CONDACTS_H__


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

const CondactArgs const CONDACTS[256] = {
	{ "AT",        1 },
	{ "NOTAT",     1 },
	{ "ATGT",      1 },
	{ "ATLT",      1 },
	{ "PRESENT",   1 },
	{ "ABSENT",    1 },
	{ "WORN",      1 },
	{ "NOTWORN",   1 },
	{ "CARRIED",   1 },
	{ "NOTCARR",   1 },
	{ "CHANCE",    1 },
	{ "ZERO",      1 },
	{ "NOTZERO",   1 },
	{ "EQ",        2 },
	{ "GT",        2 },
	{ "LT",        2 },
	{ "ADJECT1",   1 },
	{ "ADVERB",    1 },
	{ "SFX",       2 },
	{ "DESC",      1 },
	{ "QUIT",      0 },
	{ "END",       0 },
	{ "DONE",      0 },
	{ "OK",        0 },
	{ "ANYKEY",    0 },
	{ "SAVE",      1 },
	{ "LOAD",      1 },
	{ "DPRINT",    1 },
	{ "DISPLAY",   1 },
	{ "CLS",       0 },
	{ "DROPALL",   0 },
	{ "AUTOG",     0 },
	{ "AUTOD",     0 },
	{ "AUTOW",     0 },
	{ "AUTOR",     0 },
	{ "PAUSE",     1 },
	{ "SYNONYM",   2 },
	{ "GOTO",      1 },
	{ "MESSAGE",   1 },
	{ "REMOVE",    1 },
	{ "GET",       1 },
	{ "DROP",      1 },
	{ "WEAR",      1 },
	{ "DESTROY",   1 },
	{ "CREATE",    1 },
	{ "SWAP",      2 },
	{ "PLACE",     2 },
	{ "SET",       1 },
	{ "CLEAR",     1 },
	{ "PLUS",      2 },
	{ "MINUS",     2 },
	{ "LET",       2 },
	{ "NEWLINE",   0 },
	{ "PRINT",     1 },
	{ "SYSMESS",   1 },
	{ "ISAT",      2 },
	{ "SETCO",     1 },
	{ "SPACE",     0 },
	{ "HASAT",     1 },
	{ "HASNAT",    1 },
	{ "LISTOBJ",   0 },
	{ "EXTERN",    2 },
	{ "RAMSAVE",   0 },
	{ "RAMLOAD",   1 },
	{ "BEEP",      2 },
	{ "PAPER",     1 },
	{ "INK",       1 },
	{ "BORDER",    1 },
	{ "PREP",      1 },
	{ "NOUN2",     1 },
	{ "ADJECT2",   1 },
	{ "ADD",       2 },
	{ "SUB",       2 },
	{ "PARSE",     1 },
	{ "LISTAT",    1 },
	{ "PROCESS",   1 },
	{ "SAME",      2 },
	{ "MES",       1 },
	{ "WINDOW",    1 },
	{ "NOTEQ",     2 },
	{ "NOTSAME",   2 },
	{ "MODE",      1 },
	{ "WINAT",     2 },
	{ "TIME",      2 },
	{ "PICTURE",   1 },
	{ "DOALL",     1 },
	{ "MOUSE",     1 },
	{ "GFX",       2 },
	{ "ISNOTAT",   2 },
	{ "WEIGH",     2 },
	{ "PUTIN",     2 },
	{ "TAKEOUT",   2 },
	{ "NEWTEXT",   0 },
	{ "ABILITY",   2 },
	{ "WEIGHT",    1 },
	{ "RANDOM",    1 },
	{ "INPUT",     2 },
	{ "SAVEAT",    0 },
	{ "BACKAT",    0 },
	{ "PRINTAT",   2 },
	{ "WHATO",     0 },
	{ "CALL",      1 },
	{ "PUTO",      1 },
	{ "NOTDONE",   0 },
	{ "AUTOP",     1 },
	{ "AUTOT",     1 },
	{ "MOVE",      1 },
	{ "WINSIZE",   2 },
	{ "REDO",      0 },
	{ "CENTRE",    0 },
	{ "EXIT",      1 },
	{ "INKEY",     0 },
	{ "BIGGER",    2 },
	{ "SMALLER",   2 },
	{ "ISDONE",    0 },
	{ "ISNDONE",   0 },
	{ "SKIP",      1 },
	{ "RESTART",   0 },
	{ "TAB",       1 },
	{ "COPYOF",    2 },
	{ "NOT_USED1", 0 },
	{ "COPYOO",    2 },
	{ "NOT_USED2", 0 },
	{ "COPYFO",    2 },
	{ "NOT_USED3", 0 },
	{ "COPYFF",    2 },
	{ "COPYBF",    2 },
	{ "RESET",     0 }
};
#endif	//VERBOSE

typedef struct {
	void (* const function)();
	uint8_t flag;					//0:not count for ISDONE | 1:count for ISDONE
} CONDACT_LIST;


#endif //__DAAD_CONDACTS_H__
