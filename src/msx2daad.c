/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	If you sell a product containing or based in this software you must provide 
	to the copyright holder (licensor) a single copy of your physical product or,
	if not exists, a copy of your final software product. You can contact using the 
	email provided above.

	Sublicense must maintain the original authors attributions and subproducts
	maintains sell conditions.

	The name of the licensors may not be used to endorse or promote products derived
	from this software without specific prior written permission.

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.

	===============================================================================

	DAAD is a trademark of Andrés Samudio.
*/
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "dos.h"
#include "utils.h"
#include "daad.h"

const char *version = VERSION_STR;

int main(char** argv, int argc)
{
	// Platform system checks
	checkPlatformSystem();

	sfxInit();
	gfxSetScreen();

	//Load files (DDB, Font, ...)
	if (!initDAAD(argc, argv)) {
		die("Bad DDB file!\n");
	}

	mainLoop();
}
