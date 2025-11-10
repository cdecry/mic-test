#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <3ds.h>

// todo: move into utils
void ok (const char* msg){ printf("\x1b[32m[OK]\x1b[0m  %s\n", msg); }
void err(const char* msg){ printf("\x1b[31m[!!]\x1b[0m %s\n", msg); }
void inf(const char* msg){ printf("\x1b[36m[--]\x1b[0m %s\n", msg); }

int main() {
  gfxInitDefault();
  consoleInit(GFX_BOTTOM, NULL);
  inf("press start to exit\n");

  u32 micbuf_size = 0x30000;
	// u32 micbuf_pos = 0;
	u8* micbuf = static_cast<u8*>(memalign(0x1000, micbuf_size));

  inf("initializing NDSP...");
  if (R_FAILED(ndspInit())) {
    err("failed to initialize NDSP.");
  } else ok("NDSP initialized.");

  inf("initializing mic...");
  if (R_FAILED(micInit(micbuf, micbuf_size))) {
    err("failed to initialize mic.");
  } else ok("mic initialized.");

  while(aptMainLoop()) {
    hidScanInput();
    gspWaitForVBlank();

    u32 kDown = hidKeysDown();
    if (kDown & KEY_START)
    break; // return to launcher

    gfxFlushBuffers();
    gfxSwapBuffers();
  }

  micExit();
  free(micbuf);
  ndspExit();
  gfxExit();
  return 0;
}
