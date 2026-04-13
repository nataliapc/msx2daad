/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include "daad.h"


/*
 * Function: mainLoop
 * --------------------------------
 * DAAD main loop start.
 * 
 * @return			none.
 */
void mainLoop()
{
	initFlags();
	initializePROC();

	pushPROC(0);
	processPROC();
}
