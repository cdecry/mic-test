#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <3ds.h>

int main()
{
	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);

	while(aptMainLoop())
	{
		hidScanInput();
    gspWaitForVBlank();
    
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // return to launcher

		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	gfxExit();
	return 0;
}
