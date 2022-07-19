#include <fstream>
#include <gum/interceptor.hpp>
#include <interfaces.hpp>
#include <ios>
#include <modules/videorecordmod.hpp>
#include <plugin.hpp>
#include <string>

#include <bitmap/imageformat.h>
#include <convar.h>
#include <materialsystem/imaterialsystemhardwareconfig.h>
#include <replay/ienginereplay.h>

static IVideoMode *videomode = nullptr;
static ConVar pe_render("pe_render", 0, FCVAR_DONTRECORD | FCVAR_HIDDEN,
                        "enables recording");

// TODO: We should search signatures
const gpointer SCR_UPDATESCREEN_OFFSET = reinterpret_cast<gpointer>(0x39ea70);
// const gpointer SHADER_SWAPBUFFERS_OFFSET =
// reinterpret_cast<gpointer>(0x39f660);
const gpointer VIDEOMODE_OFFSET = reinterpret_cast<gpointer>(0xab39e0);
const gpointer SND_RECORDBUFFER_OFFSET = reinterpret_cast<gpointer>(0x2813d0);

void takeScreenshot(const std::string fname) {
  std::ofstream file{fname, std::ios_base::binary};
  constexpr auto image_format = IMAGE_FORMAT_RGB888;
  auto width = videomode->GetModeStereoWidth();
  auto height = videomode->GetModeStereoHeight();
  auto mem_required =
      ImageLoader::GetMemRequired(width, height, 1, image_format, false);
  uint8_t buf[mem_required];
  videomode->ReadScreenPixels(0, 0, width, height, buf, image_format);
  file.write(buf, sizeof buf);
};

VideoRecordListener::VideoRecordListener() : Listener(){};
VideoRecordListener::~VideoRecordListener(){};
void VideoRecordListener::on_enter(GumInvocationContext *context){};

void VideoRecordListener::on_leave(GumInvocationContext *context) {
  if (pe_render.GetBool()) {
    const std::string fname{"image-"};
    takeScreenshot(fname + std::to_string(framenum++));
  };
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
