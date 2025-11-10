#include "audio.h"
#include <malloc.h>
#include <cstring>

namespace audio {

bool init(State& s) {
  memset(&s.wave, 0, sizeof(s.wave));

  s.micbuf_size = 0x30000;
  s.micbuf_pos = 0;
  s.micbuf = static_cast<u8*>(memalign(0x1000, s.micbuf_size));

  if (R_FAILED(ndspInit()))
    return s.initialized = false;
  if (R_FAILED(micInit(s.micbuf, s.micbuf_size))) {
    s.initialized = false;
    free(s.micbuf);
    s.micbuf = nullptr;
    ndspExit();
  }

  s.micbuf_datasize = micGetSampleDataSize();
  s.audiobuf_size = 0x100000;
  s.audiobuf_pos = 0;
  s.audiobuf = static_cast<u8*>(linearAlloc(s.audiobuf_size));
  if (!s.audiobuf) {
    s.initialized = false;
    micExit();
    free(s.micbuf);
    s.micbuf = nullptr;
    ndspExit();
  }

  return s.initialized;
}

void shutdown(State& s) {
  if (!s.initialized) return;

  ndspChnReset(0);
  ndspChnWaveBufClear(0);

  if (s.audiobuf) linearFree(s.audiobuf), s.audiobuf = nullptr;
  micExit();
  if (s.micbuf) free(s.micbuf), s.micbuf = nullptr;
  ndspExit();
  s.initialized = false;
}

bool startRecording(State& s) {
  if (!s.initialized) return false;
  s.audiobuf_pos  = 0;
  s.micbuf_pos = 0;

  ndspChnReset(0);
  ndspChnWaveBufClear(0);

  Result r = MICU_StartSampling(
    MICU_ENCODING_PCM16_SIGNED,
    MICU_SAMPLE_RATE_32730,
    0,
    s.micbuf_datasize,
    true
  );

  return R_SUCCEEDED(r);
}

bool stopRecording(State& s) {
  if (!s.initialized) return false;
  return R_SUCCEEDED(MICU_StopSampling());
}

bool pumpMicToPcm(State& s) {
  if (!s.initialized) return false;
  u32 micbuf_readpos = s.micbuf_pos;
  s.micbuf_pos = micGetLastSampleOffset();
  while (s.audiobuf_pos < s.audiobuf_size && micbuf_readpos != s.micbuf_pos) {
    s.audiobuf[s.audiobuf_pos] = s.micbuf[micbuf_readpos];
    s.audiobuf_pos++;
    micbuf_readpos = (micbuf_readpos + 1) % s.micbuf_datasize;
  }
  // return true if limit reached
  return s.audiobuf_pos == s.audiobuf_size;
}

bool startPlayback(State& s) {
  if (!s.initialized || s.audiobuf_pos == 0) return false;

  std::memset(&s.wave, 0, sizeof(s.wave));
  s.wave.data_pcm16 = reinterpret_cast<s16*>(s.audiobuf);
  s.wave.nsamples = s.audiobuf_pos / 2;
  s.wave.looping = false;

  DSP_FlushDataCache(s.wave.data_pcm16, s.audiobuf_pos);
  ndspChnWaveBufClear(0);
  ndspChnWaveBufAdd(0, &s.wave);
  s.playing = true;
  return true;
}

bool isPlaybackDone(State& s) {
  if (!s.initialized || !s.playing) return false;
  if (s.wave.status == NDSP_WBUF_DONE) {
    s.playing = false;
    return true;
  }
  return false;
}

}
