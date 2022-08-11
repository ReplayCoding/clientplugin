#pragma once
#include <convar.h>
#include <materialsystem/MaterialSystemUtil.h>

#include <fstream>
#include <gum/interceptor.hpp>
#include <gum/x86patcher.hpp>
#include <modules/modules.hpp>
#include <sdk-excluded.hpp>
#include <hook/attachmenthook.hpp>
#include "x264encoder.hpp"

class VideoRecordMod : public IModule {
 public:
  VideoRecordMod();
  virtual ~VideoRecordMod();

 private:
  void renderVideoFrame();
  void renderAudioFrame();
  void initRenderTexture(IMaterialSystem* materialSystem);

  int width{};
  int height{};
  std::unique_ptr<X264Encoder> encoder;

  std::ofstream o_vidfile;
  std::ofstream o_audfile;
  CTextureReference renderTexture;

  ConVar pe_render;

  int** snd_p{};

  int* snd_vol{};
  int* snd_linear_count{};

  std::unique_ptr<X86Patcher> getSoundTime_patch;
  std::unique_ptr<X86Patcher> setSoundFrameTime_patch;
  std::unique_ptr<AttachmentHookLeave> scr_updateScreen_hook;
  std::unique_ptr<AttachmentHookLeave> snd_recordBuffer_hook;
};
