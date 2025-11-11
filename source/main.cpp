#include "main.h"

int main() {
  gfxInitDefault();
  consoleInit(GFX_BOTTOM, NULL);
  inf("press start to exit\n");

  SocketService socketService;
  socketService.initSocketService();
  inf("initializing audio...");
  audio::State a;
  if (!audio::init(a)) {
    err("audio init failed");
    gfxExit();
    return 1;
  }
  ok("audio ready.");

  while(aptMainLoop()) {
    hidScanInput();
    gspWaitForVBlank();

    u32 kDown = hidKeysDown();
    if (kDown & KEY_START)
    break; // return to launcher

    if(kDown & KEY_A) {
      if (audio::startRecording(a)) inf("recording started...");
      else err("failed to start sampling.");
    }
    if ((hidKeysHeld() & KEY_A)) {
      if (audio::pumpMicToPcm(a)) inf("recording limit reached.");
    }
    if (hidKeysUp() & KEY_A) {
      if (audio::stopRecording(a)) ok("recording stopped.");
      else err("failed to stop recording.");
    }
    if (kDown & KEY_B) {
      std::string url = "https://jsonplaceholder.typicode.com/posts";
      std::string json = R"({"title":"foo","body":"bar","userId":1})";

      std::string resp = sendHTTPPost(
      url,
      json,
      {"Content-Type: application/json; charset=UTF-8"}  // headers
      );
      ok(resp.c_str());
      if (audio::startPlayback(a)) inf("playback started...");
      else err("failed to start playback.");
    }
    if (audio::isPlaybackDone(a)) ok("playback finished");

    gfxFlushBuffers();
    gfxSwapBuffers();
  }

  audio::shutdown(a);
  gfxExit();
  return 0;
}
