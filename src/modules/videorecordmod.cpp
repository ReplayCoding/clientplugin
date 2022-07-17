#include <gum/interceptor.hpp>
#include <modules/videorecordmod.hpp>
#include <plugin.hpp>
#include <interfaces.hpp>


#include <bitmap/imageformat.h>
#include <convar.h>
#include <materialsystem/imaterialsystemhardwareconfig.h>
#include <replay/ienginereplay.h>

static IVideoMode *videomode = nullptr;
static ConVar pe_take_screenshot("pe_take_screenshot", "", FCVAR_DONTRECORD,
                                 "Takes a screenshot");

// TODO: We should search signatures
const gpointer SCR_UPDATESCREEN_OFFSET = reinterpret_cast<gpointer>(0x39ea70);
// const gpointer SHADER_SWAPBUFFERS_OFFSET =
// reinterpret_cast<gpointer>(0x39f660);
const gpointer VIDEOMODE_OFFSET = reinterpret_cast<gpointer>(0xab39e0);
const gpointer SND_RECORDBUFFER_OFFSET = reinterpret_cast<gpointer>(0x2813d0);

VideoRecordListener::VideoRecordListener() : Listener(){};
VideoRecordListener::~VideoRecordListener(){};
void VideoRecordListener::on_enter(GumInvocationContext *context) {
  // bool doSwap = g_pMaterialSystemHardwareConfig->ReadPixelsFromFrontBuffer();
  // // I have no idea what this does in this case... It doesn't even do
  // anything
  // // on linux
  // if (doSwap) {
  //   Shader_SwapBuffers();
  // };

  constexpr auto image_format = IMAGE_FORMAT_RGB888;
  auto width = videomode->GetModeStereoWidth();
  auto height = videomode->GetModeStereoHeight();
  auto mem_required =
      ImageLoader::GetMemRequired(width, height, 1, image_format, false);
  uint8_t buf[mem_required];
  videomode->ReadScreenPixels(0, 0, width, height, buf, image_format);

  auto fname = pe_take_screenshot.GetString();
  if (strncmp(fname, "", 1)) {

    videomode->TakeSnapshotTGA(fname);
    pe_take_screenshot.SetValue("");
  };

  // if (doSwap) {
  //   Shader_SwapBuffers();
  // };
};

void VideoRecordListener::on_leave(GumInvocationContext *context){};

CON_COMMAND(pe_startmovie, "Record game") {
  g_print("RECORDING!\n");
  auto engineClientReplay = Interfaces.GetEngineClientReplay();
  engineClientReplay->InitSoundRecord();
};

VideoRecordMod::VideoRecordMod() {
  listener = std::make_shared<VideoRecordListener>();

  const GumAddress module_base = gum_module_find_base_address("engine.so");
  void **videomode_ptr = (module_base + VIDEOMODE_OFFSET);
  videomode = static_cast<decltype(videomode)>(*videomode_ptr);
  g_Interceptor->attach(module_base + SCR_UPDATESCREEN_OFFSET, listener,
                        nullptr);
};
VideoRecordMod::~VideoRecordMod() { g_Interceptor->detach(listener); };
