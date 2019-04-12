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
void do_NOT_USED();
void do_COPYOO();
void do_COPYFO();
void do_COPYFF();
void do_COPYBF();
void do_RESET();

#ifdef VERBOSE
const char const CONDACTS[256][12] = {
	"AT",         //0   //   1
	"NOTAT",      //1   //   1
	"ATGT",       //2   //   1
	"ATLT",       //3   //   1
	"PRESENT",    //4   //   1
	"ABSENT",     //5   //   1
	"WORN",       //6   //   1
	"NOTWORN",    //7   //   1
	"CARRIED",    //8   //   1
	"NOTCARR",    //9   //   1
	"CHANCE",     //10  //   1
	"ZERO",       //11  //   1
	"NOTZERO",    //12  //   1
	"EQ",         //13  //   2
	"GT",         //14  //   2
	"LT",         //15  //   2
	"ADJECT1",    //16  //   1
	"ADVERB",     //17  //   1
	"SFX",        //18  //   2
	"DESC",       //19  //   1
	"QUIT",       //20  //   0
	"END",        //21  //   0
	"DONE",       //22  //   0
	"OK",         //23  //   0
	"ANYKEY",     //24  //   0
	"SAVE",       //25  //   1
	"LOAD",       //26  //   1
	"DPRINT",     //27  //   1 *
	"DISPLAY",    //28  //   1 *
	"CLS",        //29  //   0
	"DROPALL",    //30  //   0
	"AUTOG",      //31  //   0
	"AUTOD",      //32  //   0
	"AUTOW",      //33  //   0
	"AUTOR",      //34  //   0
	"PAUSE",      //35  //   1
	"SYNONYM",    //36  //   2 *
	"GOTO",       //37  //   1
	"MESSAGE",    //38  //   1
	"REMOVE",     //39  //   1
	"GET",        //40  //   1
	"DROP",       //41  //   1
	"WEAR",       //42  //   1
	"DESTROY",    //43  //   1
	"CREATE",     //44  //   1
	"SWAP",       //45  //   2
	"PLACE",      //46  //   2
	"SET",        //47  //   1
	"CLEAR",      //48  //   1
	"PLUS",       //49  //   2
	"MINUS",      //50  //   2
	"LET",        //51  //   2
	"NEWLINE",    //52  //   0
	"PRINT",      //53  //   1
	"SYSMESS",    //54  //   1
	"ISAT",       //55  //   2
	"SETCO",      //56  //   1
	"SPACE",      //57  //   0
	"HASAT",      //58  //   1
	"HASNAT",     //59  //   1
	"LISTOBJ",    //60  //   0
	"EXTERN",     //61  //   2
	"RAMSAVE",    //62  //   0
	"RAMLOAD",    //63  //   1
	"BEEP",       //64  //   2
	"PAPER",      //65  //   1
	"INK",        //66  //   1
	"BORDER",     //67  //   1
	"PREP",       //68  //   1
	"NOUN2",      //69  //   1
	"ADJECT2",    //70  //   1
	"ADD",        //71  //   2
	"SUB",        //72  //   2
	"PARSE",      //73  //   1
	"LISTAT",     //74  //   1
	"PROCESS",    //75  //   1
	"SAME",       //76  //   2
	"MES",        //77  //   1
	"WINDOW",     //78  //   1
	"NOTEQ",      //79  //   2
	"NOTSAME",    //80  //   2
	"MODE",       //81  //   1
	"WINAT",      //82  //   2
	"TIME",       //83  //   2
	"PICTURE",    //84  //   1
	"DOALL",      //85  //   1
	"MOUSE",      //86  //   1
	"GFX",        //87  //   2
	"ISNOTAT",    //88  //   2
	"WEIGH",      //89  //   2
	"PUTIN",      //90  //   2
	"TAKEOUT",    //91  //   2
	"NEWTEXT",    //92  //   0
	"ABILITY",    //93  //   2
	"WEIGHT",     //94  //   1
	"RANDOM",     //95  //   1
	"INPUT",      //96  //   2
	"SAVEAT",     //97  //   0
	"BACKAT",     //98  //   0
	"PRINTAT",    //99  //   2
	"WHATO",      //100 //   0
	"CALL",       //101 //   1
	"PUTO",       //102 //   1
	"NOTDONE",    //103 //   0
	"AUTOP",      //104 //   1
	"AUTOT",      //105 //   1
	"MOVE",       //106 //   1
	"WINSIZE",    //107 //   2
	"REDO",       //108 //   0
	"CENTRE",     //109 //   0
	"EXIT",       //110 //   1
	"INKEY",      //111 //   0
	"BIGGER",     //112 //   2
	"SMALLER",    //113 //   2
	"ISDONE",     //114 //   0
	"ISNDONE",    //115 //   0
	"SKIP",       //116 //   1
	"RESTART",    //117 //   0
	"TAB",        //118 //   1
	"COPYOF",     //119 //   2
	"NOT_USED1",  //120 //   0 (according DAAD manual", internal)
	"COPYOO",     //121 //   2
	"NOT_USED2",  //122 //   0 (according DAAD manual", internal)
	"COPYFO",     //123 //   2
	"NOT_USED3",  //124 //   0 (according DAAD manual", internal)
	"COPYFF",     //125 //   2
	"COPYBF",     //126 //   2
	"RESET"       //127 //   0
};
#endif

typedef struct {
	void (* const condact)();
	uint8_t flag;
} CONDACTS_FUNC;

const CONDACTS_FUNC condactList[] = {
	{ do_AT, 		0 },
	{ do_NOTAT,		0 },
	{ do_ATGT,		0 },
	{ do_ATLT,		0 },
	{ do_PRESENT,	0 },
	{ do_ABSENT,	0 },
	{ do_WORN,		0 },
	{ do_NOTWORN,	0 },
	{ do_CARRIED,	0 },
	{ do_NOTCARR,	0 },
	{ do_CHANCE,	0 },
	{ do_ZERO,		0 },
	{ do_NOTZERO,	0 },
	{ do_EQ,		0 },
	{ do_GT,		0 },
	{ do_LT,		0 },
	{ do_ADJECT1,	0 },
	{ do_ADVERB,	0 },
	{ do_SFX,		1 },
	{ do_DESC,		1 },
	{ do_QUIT,		0 },
	{ do_END,		1 },
	{ do_DONE,		1 },
	{ do_OK,		1 },
	{ do_ANYKEY,	1 },
	{ do_SAVE,		1 },
	{ do_LOAD,		1 },
	{ do_DPRINT,	1 },
	{ do_DISPLAY,	1 },
	{ do_CLS,		1 },
	{ do_DROPALL,	1 },
	{ do_AUTOG,		1 },
	{ do_AUTOD,		1 },
	{ do_AUTOW,		1 },
	{ do_AUTOR,		1 },
	{ do_PAUSE,		1 },
	{ do_SYNONYM,	1 },
	{ do_GOTO,		1 },
	{ do_MESSAGE,	1 },
	{ do_REMOVE,	1 },
	{ do_GET,		1 },
	{ do_DROP,		1 },
	{ do_WEAR,		1 },
	{ do_DESTROY,	1 },
	{ do_CREATE,	1 },
	{ do_SWAP,		1 },
	{ do_PLACE,		1 },
	{ do_SET,		1 },
	{ do_CLEAR,		1 },
	{ do_PLUS,		1 },
	{ do_MINUS,		1 },
	{ do_LET,		1 },
	{ do_NEWLINE,	1 },
	{ do_PRINT,		1 },
	{ do_SYSMESS,	1 },
	{ do_ISAT,		0 },
	{ do_SETCO,		1 },
	{ do_SPACE,		1 },
	{ do_HASAT,		0 },
	{ do_HASNAT,	0 },
	{ do_LISTOBJ,	1 },
	{ do_EXTERN,	1 },
	{ do_RAMSAVE,	1 },
	{ do_RAMLOAD,	1 },
	{ do_BEEP,		1 },
	{ do_PAPER,		1 },
	{ do_INK,		1 },
	{ do_BORDER,	1 },
	{ do_PREP,		0 },
	{ do_NOUN2,		0 },
	{ do_ADJECT2,	0 },
	{ do_ADD,		1 },
	{ do_SUB,		1 },
	{ do_PARSE,		1 },
	{ do_LISTAT,	1 },
	{ do_PROCESS,	1 },
	{ do_SAME,		0 },
	{ do_MES,		1 },
	{ do_WINDOW,	1 },
	{ do_NOTEQ,		0 },
	{ do_NOTSAME,	0 },
	{ do_MODE,		1 },
	{ do_WINAT,		1 },
	{ do_TIME,		1 },
	{ do_PICTURE,	1 },
	{ do_DOALL,		1 },
	{ do_MOUSE,		1 },
	{ do_GFX,		1 },
	{ do_ISNOTAT,	0 },
	{ do_WEIGH,		1 },
	{ do_PUTIN,		1 },
	{ do_TAKEOUT,	1 },
	{ do_NEWTEXT,	1 },
	{ do_ABILITY,	1 },
	{ do_WEIGHT,	1 },
	{ do_RANDOM,	1 },
	{ do_INPUT,		1 },
	{ do_SAVEAT,	1 },
	{ do_BACKAT,	1 },
	{ do_PRINTAT,	1 },
	{ do_WHATO,		1 },
	{ do_CALL,		1 },
	{ do_PUTO,		1 },
	{ do_NOTDONE,	0 },
	{ do_AUTOP,		1 },
	{ do_AUTOT,		1 },
	{ do_MOVE,		1 },
	{ do_WINSIZE,	1 },
	{ do_REDO,		1 },
	{ do_CENTRE,	1 },
	{ do_EXIT,		1 },
	{ do_INKEY,		0 },
	{ do_BIGGER,	0 },
	{ do_SMALLER,	0 },
	{ do_ISDONE,	0 },
	{ do_ISNDONE,	0 },
	{ do_SKIP,		1 },
	{ do_RESTART,	1 },
	{ do_TAB,		1 },
	{ do_COPYOF,	1 },
	{ do_NOT_USED,	1 },
	{ do_COPYOO,	1 },
	{ do_NOT_USED,	1 },
	{ do_COPYFO,	1 },
	{ do_NOT_USED,	1 },
	{ do_COPYFF,	1 },
	{ do_COPYBF,	1 },
	{ do_RESET,		1 },
};

/*
void (* const condactList[])() = {
	do_AT,         //0   //   1
	do_NOTAT,      //1   //   1
	do_ATGT,       //2   //   1
	do_ATLT,       //3   //   1
	do_PRESENT,    //4   //   1
	do_ABSENT,     //5   //   1
	do_WORN,       //6   //   1
	do_NOTWORN,    //7   //   1
	do_CARRIED,    //8   //   1
	do_NOTCARR,    //9   //   1
	do_CHANCE,     //10  //   1
	do_ZERO,       //11  //   1
	do_NOTZERO,    //12  //   1
	do_EQ,         //13  //   2
	do_GT,         //14  //   2
	do_LT,         //15  //   2
	do_ADJECT1,    //16  //   1
	do_ADVERB,     //17  //   1
	do_SFX,        //18  //   2
	do_DESC,       //19  //   1
	do_QUIT,       //20  //   0
	do_END,        //21  //   0
	do_DONE,       //22  //   0
	do_OK,         //23  //   0
	do_ANYKEY,     //24  //   0
	do_SAVE,       //25  //   1
	do_LOAD,       //26  //   1
	do_DPRINT,     //27  //   1 *
	do_DISPLAY,    //28  //   1 *
	do_CLS,        //29  //   0
	do_DROPALL,    //30  //   0
	do_AUTOG,      //31  //   0
	do_AUTOD,      //32  //   0
	do_AUTOW,      //33  //   0
	do_AUTOR,      //34  //   0
	do_PAUSE,      //35  //   1
	do_SYNONYM,    //36  //   2 *
	do_GOTO,       //37  //   1
	do_MESSAGE,    //38  //   1
	do_REMOVE,     //39  //   1
	do_GET,        //40  //   1
	do_DROP,       //41  //   1
	do_WEAR,       //42  //   1
	do_DESTROY,    //43  //   1
	do_CREATE,     //44  //   1
	do_SWAP,       //45  //   2
	do_PLACE,      //46  //   2
	do_SET,        //47  //   1
	do_CLEAR,      //48  //   1
	do_PLUS,       //49  //   2
	do_MINUS,      //50  //   2
	do_LET,        //51  //   2
	do_NEWLINE,    //52  //   0
	do_PRINT,      //53  //   1
	do_SYSMESS,    //54  //   1
	do_ISAT,       //55  //   2
	do_SETCO,      //56  //   1
	do_SPACE,      //57  //   0
	do_HASAT,      //58  //   1
	do_HASNAT,     //59  //   1
	do_LISTOBJ,    //60  //   0
	do_EXTERN,     //61  //   2
	do_RAMSAVE,    //62  //   0
	do_RAMLOAD,    //63  //   1
	do_BEEP,       //64  //   2
	do_PAPER,      //65  //   1
	do_INK,        //66  //   1
	do_BORDER,     //67  //   1
	do_PREP,       //68  //   1
	do_NOUN2,      //69  //   1
	do_ADJECT2,    //70  //   1
	do_ADD,        //71  //   2
	do_SUB,        //72  //   2
	do_PARSE,      //73  //   1
	do_LISTAT,     //74  //   1
	do_PROCESS,    //75  //   1
	do_SAME,       //76  //   2
	do_MES,        //77  //   1
	do_WINDOW,     //78  //   1
	do_NOTEQ,      //79  //   2
	do_NOTSAME,    //80  //   2
	do_MODE,       //81  //   1
	do_WINAT,      //82  //   2
	do_TIME,       //83  //   2
	do_PICTURE,    //84  //   1
	do_DOALL,      //85  //   1
	do_MOUSE,      //86  //   1
	do_GFX,        //87  //   2
	do_ISNOTAT,    //88  //   2
	do_WEIGH,      //89  //   2
	do_PUTIN,      //90  //   2
	do_TAKEOUT,    //91  //   2
	do_NEWTEXT,    //92  //   0
	do_ABILITY,    //93  //   2
	do_WEIGHT,     //94  //   1
	do_RANDOM,     //95  //   1
	do_INPUT,      //96  //   2
	do_SAVEAT,     //97  //   0
	do_BACKAT,     //98  //   0
	do_PRINTAT,    //99  //   2
	do_WHATO,      //100 //   0
	do_CALL,       //101 //   1
	do_PUTO,       //102 //   1
	do_NOTDONE,    //103 //   0
	do_AUTOP,      //104 //   1
	do_AUTOT,      //105 //   1
	do_MOVE,       //106 //   1
	do_WINSIZE,    //107 //   2
	do_REDO,       //108 //   0
	do_CENTRE,     //109 //   0
	do_EXIT,       //110 //   1
	do_INKEY,      //111 //   0
	do_BIGGER,     //112 //   2
	do_SMALLER,    //113 //   2
	do_ISDONE,     //114 //   0
	do_ISNDONE,    //115 //   0
	do_SKIP,       //116 //   1
	do_RESTART,    //117 //   0
	do_TAB,        //118 //   1
	do_COPYOF,     //119 //   2
	do_NOT_USED,   //120 //   0 (according DAAD manual, internal)
	do_COPYOO,     //121 //   2
	do_NOT_USED,   //122 //   0 (according DAAD manual, internal)
	do_COPYFO,     //123 //   2
	do_NOT_USED,   //124 //   0 (according DAAD manual, internal)
	do_COPYFF,     //125 //   2
	do_COPYBF,     //126 //   2
	do_RESET       //127 //   0
};
*/

#endif //__DAAD_CONDACTS_H__