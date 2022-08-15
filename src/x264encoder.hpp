#pragma once

// Must be included before x264 headers
#include <cstdint>

#include <x264.h>
#include <ostream>
#include <string>

typedef std::string EncoderError;
class X264Encoder {
 public:
  X264Encoder(int width,
              int height,
              int fps,
              std::string preset,
              std::ostream& output_stream);
  ~X264Encoder() noexcept(false);
  void encode_frame(uint8_t* input_buf);

 private:
  x264_t* encoder{};
  x264_param_t param{};
  x264_picture_t pic_in{};
  x264_picture_t pic_out{};

  // TODO: Is this a memory leak?
  x264_nal_t* nal{};
  int i_nal{};

  uint64_t current_frame{};
  std::ostream& output_stream;
};
