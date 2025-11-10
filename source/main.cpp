#include "main.h"

int main() {
  gfxInitDefault();
  consoleInit(GFX_BOTTOM, NULL);
  inf("press start to exit\n");

  static bool initialized = true;
  static bool isPlaying = false;

  static ndspWaveBuf wave;
  memset(&wave, 0, sizeof(wave));

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
        if (R_SUCCEEDED(r)) inf("recording started...");
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
        if (R_FAILED(MICU_StopSampling())) err("failed to stop sampling.");
        else ok("recording stopped.");
      }

      if (kDown & KEY_B) {
        wave.data_pcm16 = (s16*)audiobuf;
        wave.nsamples = audiobuf_pos / 2;
        wave.looping = false;
        isPlaying = true;

        inf("playback starting...");
        DSP_FlushDataCache(wave.data_pcm16, audiobuf_pos);
        ndspChnWaveBufAdd(0, &wave);
      }
    }

    if (isPlaying && wave.status == NDSP_WBUF_DONE) {
      isPlaying = false;
      ok("playback finished.");
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
