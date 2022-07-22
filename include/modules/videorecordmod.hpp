#pragma once
#include <fstream>
#include <gum/interceptor.hpp>
#include <modules/modules.hpp>
#include <ostream>
#include <sdk-excluded.hpp>
#include <x264.h>

typedef std::string EncoderError;
class X264Encoder {
public:
  X264Encoder(int width, int height, int fps);
  ~X264Encoder();
  void encode_frame(uint8_t *input_buf, FILE *output);

private:
  x264_t *encoder{};
  x264_param_t param{};
  x264_picture_t pic_in{};
  x264_picture_t pic_out{};

  // TODO: Is this a memory leak?
  x264_nal_t *nal{};
  int i_nal{};

  uint64_t current_frame{};
};

class VideoRecordMod : public IModule, public Listener {
public:
  VideoRecordMod();
  virtual ~VideoRecordMod();

  virtual void on_enter(GumInvocationContext *context);
  virtual void on_leave(GumInvocationContext *context);

  int width{};
  int height{};
  FILE *ofile;
  X264Encoder *encoder{};

private:
  void renderFrame();
};