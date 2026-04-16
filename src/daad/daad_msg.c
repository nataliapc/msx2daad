/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include "daad.h"


/*
 * Function: _ptrToMessage
 * --------------------------------
 * Return pointer to a DDB message.
 * 
 * @param lst		Section pointer.
 * @param num		Message number to extract.
 * @return			none.
 */
char* _ptrToMessage(uint16_t *lst, uint8_t num)
{
	return &ddb[*(lst + num)];
}

/*
 * Function: getSystemMsg
 * --------------------------------
 * Extract system message.
 * 
 * @param num		Number of system message.
 * @return			none.
 */
void getSystemMsg(uint8_t num)
{
	printMsg(_ptrToMessage((uint16_t*)hdr->sysMsgPos, num), false);
}

/*
 * Function: printSystemMsg
 * --------------------------------
 * Extract system message and print it.
 * 
 * @param num		Number of system message.
 * @return			none.
 */
void printSystemMsg(uint8_t num)
{
	printMsg(_ptrToMessage((uint16_t*)hdr->sysMsgPos, num), true);
}

/*
 * Function: printUserMsg
 * --------------------------------
 * Extract user message and print it.
 * 
 * @param num		Number of user message.
 * @return			none.
 */
void printUserMsg(uint8_t num)
{
	if (num > hdr->numUsrMsg) errorCode(7);
	printMsg(_ptrToMessage((uint16_t*)hdr->usrMsgPos, num), true);
}

/*
 * Function: printLocationMsg
 * --------------------------------
 * Extract location message and print it.
 * 
 * @param num		Number of location message.
 * @return			none.
 */
void printLocationMsg(uint8_t num)
{
	if (num > hdr->numLocDsc) errorCode(1);
	printMsg(_ptrToMessage((uint16_t*)hdr->locLstPos, num), true);
}

/*
 * Function: printObjectMsg
 * --------------------------------
 * Extract object message and print it.
 * 
 * @param num		Number of object message.
 * @return			none.
 */
void printObjectMsg(uint8_t num)
{
	if (num > hdr->numObjDsc) errorCode(0);
	printMsg(_ptrToMessage((uint16_t*)hdr->objLstPos, num), true);
}

/*
 * Function: printObjectMsgModif
 * --------------------------------
 * Extract object message, change the article and print it:
 * "Un palo" -> "El palo"
 * "Una linterna" -> "La linterna"
 * "A lantern" -> "Lantern"
 *  
 * @param num		Number of object name.
 * @param modif		Modifier for uppercase.
 * @return			none.
 */
void printObjectMsgModif(uint8_t num, char modif)
{
	modif;
	static char *ini, *p;
	ini = tmpMsg;
	p = tmpMsg;

	printMsg(_ptrToMessage((uint16_t*)hdr->objLstPos, num), false);

#ifdef LANG_ES
	if (!strnicmp("un ", tmpMsg, 3)) {
		tmpMsg[0] = modif=='@'?'E':'e';
		tmpMsg[1] = 'l';
	} else
	if (!strnicmp("una ", tmpMsg, 3)) {
		ini++;
		tmpMsg[1] = modif=='@'?'L':'l';
	}
	while (*p) {
		if (*p=='.' || *p==0x0a) { *p--='\0'; }
		p++;
	}
#elif LANG_EN
	if (!strnicmp("a ", tmpMsg, 2)) {
		ini+=2;
	} else
	if (!strnicmp("an ", tmpMsg, 3)) {
		ini+=3;
	}
#endif
	printOutMsg(ini);
}
