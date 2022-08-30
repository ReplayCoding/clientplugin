#pragma once
#include <convar.h>
#include <materialsystem/MaterialSystemUtil.h>

#include <fstream>

#include "hook/attachmenthook.hpp"
#include "hook/gum/x86patcher.hpp"
#include "modules/modules.hpp"
#include "sdk/concommandwrapper.hpp"
#include "x264encoder.hpp"

class VideoRecordMod : public IModule {
 public:
  VideoRecordMod();
  virtual ~VideoRecordMod();

 private:
  void renderVideoFrame();
  void renderAudioFrame();

  void startRender(const CCommand& c);
  void stopRender(const CCommand& c);

  void initRenderTexture();

  int width{};
  int height{};

  std::unique_ptr<X264Encoder> encoder;

  std::unique_ptr<std::ofstream> o_vidfile;
  std::unique_ptr<std::ofstream> o_audfile;
  CTextureReference renderTexture;

  ConCommandCallbacks pe_render_start_cbk;
  ConCommandCallbacks pe_render_stop_cbk;

  ConCommand pe_render_start;
  ConCommand pe_render_stop;

  bool isRendering{false};

  // Globals extracted from engine
  int** snd_p{};

  int* snd_vol{};
  int* snd_linear_count{};

  std::unique_ptr<X86Patcher> getSoundTime_patch;
  std::unique_ptr<X86Patcher> setSoundFrameTime_patch;
  std::unique_ptr<AttachmentHookLeave> scr_updateScreen_hook;
  std::unique_ptr<AttachmentHookLeave> snd_recordBuffer_hook;
};
