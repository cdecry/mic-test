#pragma once
#include <3ds.h>
#include <cstring>
#include "../utils/common.h"

namespace audio {

  struct State {
    u8*  micbuf          = nullptr;
    u32  micbuf_size     = 0;
    u32  micbuf_pos      = 0;
    u8*  audiobuf        = nullptr;
    u32  audiobuf_size   = 0;
    u32  audiobuf_pos    = 0;
    u32  micbuf_datasize = 0;
    ndspWaveBuf wave     = {};
    bool playing         = false;
    bool initialized     = true;
  };

  // init NDSP + MIC and allocate buffers
  bool init(State& s);

  // stop everything and free resources
  void shutdown(State& s);

  // recording lifecycle
  bool startRecording(State& s);
  bool pumpMicToPcm(State& s);
  bool stopRecording(State& s);

  // playback lifecycle
  bool startPlayback(State& s);
  bool isPlaybackDone(State& s);
}