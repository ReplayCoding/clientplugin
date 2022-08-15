#include <cstdint>
// cstdint MUST be included before x264
#include <x264.h>
#include <ostream>
#include <string>

#include "x264encoder.hpp"
#include "util.hpp"

X264Encoder::X264Encoder(int width,
                         int height,
                         int fps,
                         std::string preset,
                         std::ostream& output_stream)
    : output_stream(output_stream) {
  if (x264_param_default_preset(&param, preset.c_str(), nullptr) < 0) {
    throw StringError("Failed to create set params");
  };
  // We assume Packed RGB 8:8:8
  param.i_bitdepth = 8;
  param.i_csp = X264_CSP_RGB;
  param.i_width = width;
  param.i_height = height;

  // This isn't able to handle floating point framerates but it's close enough
  // for now
  param.i_fps_num = fps;
  param.i_fps_den = 1;
  param.b_vfr_input = 0;

  param.i_log_level = X264_LOG_DEBUG;
  // The engine gets really unstable with this any higher, and if its too high,
  // we run out of memory.
  param.i_threads = 1;

  if (x264_param_apply_profile(&param, "high444") < 0) {
    throw StringError("Failed to apply profile");
  };

  if (x264_picture_alloc(&pic_in, param.i_csp, param.i_width, param.i_height) <
      0) {
    throw StringError("Failed to allocate input picture");
  };

  encoder = x264_encoder_open(&param);
  if (encoder == nullptr) {
    throw StringError("Failed to open encoder");
  };
}

X264Encoder::~X264Encoder() noexcept(false) {
  while (x264_encoder_delayed_frames(encoder)) {
    auto i_frame_size =
        x264_encoder_encode(encoder, &nal, &i_nal, NULL, &pic_out);
    if (i_frame_size < 0) {
      throw StringError("Failed to encode delayed frames");
    };
    if (i_frame_size > 0) {
      output_stream.write(reinterpret_cast<const char*>(nal->p_payload),
                          i_frame_size);
    }
  }

  // WHAT THE FUCK IT CRASHES MY ASS
  // this is a memory leak. deal with it
  // x264_picture_clean(&pic_in);

  x264_encoder_close(encoder);
}

void X264Encoder::encode_frame(uint8_t* input_buf) {
  pic_in.img.plane[0] = input_buf;
  pic_in.i_pts = current_frame++;
  auto i_frame_size =
      x264_encoder_encode(encoder, &nal, &i_nal, &pic_in, &pic_out);
  if (i_frame_size < 0) {
    throw StringError("Failed to encode frame");
  };
  if (i_frame_size > 0) {
    output_stream.write(reinterpret_cast<const char*>(nal->p_payload),
                        i_frame_size);
  };
}
