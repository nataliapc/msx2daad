#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "dos.h"
#include "utils.h"
#include "daad.h"


int main()
{
	// Platform system checks
	checkPlatformSystem();

	gfxSetScreen();

	//Load files (DDB, Font, ...)
	if (!initDAAD()) {
		die("Bad DDB file!\n");
		exit(1);
	}

	mainLoop();

	die("Done");
}
