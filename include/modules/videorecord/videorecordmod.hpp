#pragma once
#include <convar.h>
#include <materialsystem/MaterialSystemUtil.h>

#include <fstream>
#include <gum/interceptor.hpp>
#include <gum/x86patcher.hpp>
#include <modules/modules.hpp>
#include <sdk-excluded.hpp>
#include "x264encoder.hpp"

class VideoRecordMod : public IModule, public Listener {
 public:
  VideoRecordMod();
  virtual ~VideoRecordMod();

  virtual void on_enter(GumInvocationContext* context);
  virtual void on_leave(GumInvocationContext* context);

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

  enum HookType : int {
    SCR_UpdateScreen,
    SND_RecordBuffer,
  };

  std::unique_ptr<X86Patcher> getSoundTime_patch;
  std::unique_ptr<X86Patcher> setSoundFrameTime_patch;
};
