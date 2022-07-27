#pragma once
#include <gum/x86patcher.hpp>
#include <fstream>
#include <gum/interceptor.hpp>
#include <modules/modules.hpp>
#include <ostream>
#include <sdk-excluded.hpp>
#include <x264.h>

#include <convar.h>
#include <materialsystem/MaterialSystemUtil.h>

typedef std::string EncoderError;
class X264Encoder {
public:
  X264Encoder(int width, int height, int fps, std::string preset,
              std::ostream &output_stream);
  ~X264Encoder();
  void encode_frame(uint8_t *input_buf);

private:
  x264_t *encoder{};
  x264_param_t param{};
  x264_picture_t pic_in{};
  x264_picture_t pic_out{};

  // TODO: Is this a memory leak?
  x264_nal_t *nal{};
  int i_nal{};

  uint64_t current_frame{};
  std::ostream &output_stream;
};

class VideoRecordMod : public IModule, public Listener {
public:
  VideoRecordMod();
  virtual ~VideoRecordMod();

  virtual void on_enter(GumInvocationContext *context);
  virtual void on_leave(GumInvocationContext *context);

private:
  void renderVideoFrame();
  void renderAudioFrame();
  void initRenderTexture(IMaterialSystem *materialSystem);

  int width{};
  int height{};
  std::unique_ptr<X264Encoder> encoder;

  std::ofstream o_vidfile;
  std::ofstream o_audfile;
  CTextureReference renderTexture;

  ConVar pe_render;

  int **snd_p{};

  int *snd_vol{};
  int *snd_linear_count{};

  enum HookType : int {
    SCR_UpdateScreen,
    SND_RecordBuffer,
  };

  std::unique_ptr<X86Patcher> getSoundTime_patch;
  std::unique_ptr<X86Patcher> setSoundFrameTime_patch;
};
