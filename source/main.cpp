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

  bool initialized = true;
  u32 micbuf_size = 0x30000;
	u32 micbuf_pos = 0;
	u8* micbuf = static_cast<u8*>(memalign(0x1000, micbuf_size));

  inf("initializing NDSP...");
  if (R_FAILED(ndspInit())) {
    initialized = false;
    err("failed to initialize NDSP.");
  } else {
    ok("NDSP initialized.");
  }

  inf("initializing mic...");
  if (R_FAILED(micInit(micbuf, micbuf_size))) {
    initialized = false;
    err("failed to initialize mic.");
  } else ok("mic initialized.");

  u32 micbuf_datasize = micGetSampleDataSize();
  u32 audiobuf_size = 0x100000;
  u32 audiobuf_pos = 0;
  u8* audiobuf = static_cast<u8*>(linearAlloc(audiobuf_size));
  
  if (initialized) {
    inf("\nmic is ready!");
  }

  while(aptMainLoop()) {
    hidScanInput();
    gspWaitForVBlank();

    u32 kDown = hidKeysDown();
    if (kDown & KEY_START)
    break; // return to launcher

    if (initialized) {
      if(kDown & KEY_A) {
        audiobuf_pos = 0;
        micbuf_pos = 0;

        // stop playback: reset and clear audio channel
        ndspChnReset(0);
        ndspChnWaveBufClear(0);
        
        // start sampling
        Result r = MICU_StartSampling(
          MICU_ENCODING_PCM16_SIGNED,
          MICU_SAMPLE_RATE_32730,
          0,
          micbuf_datasize,
          true
        );
        if (R_SUCCEEDED(r)) ok("sampling started.");
        else err("failed to start sampling.");
        }

        if ((hidKeysHeld() & KEY_A) && audiobuf_pos < audiobuf_size) {
          u32 micbuf_readpos = micbuf_pos;
          micbuf_pos = micGetLastSampleOffset();
          while (audiobuf_pos < audiobuf_size && micbuf_readpos != micbuf_pos) {
            audiobuf[audiobuf_pos] = micbuf[micbuf_readpos];
            audiobuf_pos++;
            micbuf_readpos = (micbuf_readpos + 1) % micbuf_datasize;
          }
      }

      if (hidKeysUp() & KEY_A) {
        inf("stopping sampling...");
        if (R_FAILED(MICU_StopSampling())) err("failed to stop sampling.");

        ndspWaveBuf wave;
        memset(&wave, 0, sizeof(wave));
        wave.data_pcm16 = (s16*)audiobuf;
        wave.nsamples = audiobuf_pos / 2;
        wave.looping = false;

        inf("starting playback...");
        DSP_FlushDataCache(wave.data_pcm16, audiobuf_pos);
        ndspChnWaveBufAdd(0, &wave);
      }
    }

    gfxFlushBuffers();
    gfxSwapBuffers();
  }

  linearFree(audiobuf);
  micExit();
  free(micbuf);
  ndspExit();
  gfxExit();
  return 0;
}
