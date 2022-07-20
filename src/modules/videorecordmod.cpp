#include <fstream>
#include <gum/interceptor.hpp>
#include <interfaces.hpp>
#include <ios>
#include <modules/videorecordmod.hpp>
#include <ostream>
#include <plugin.hpp>
#include <string>
#include <x264.h>

#include <bitmap/imageformat.h>
#include <convar.h>
#include <materialsystem/imaterialsystemhardwareconfig.h>
#include <replay/ienginereplay.h>

X264Encoder::X264Encoder(int width, int height, int fps) {
  if (x264_param_default_preset(&param, "medium", nullptr) < 0) {
    throw EncoderError("Failed to create set params");
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

  if (x264_param_apply_profile(&param, "high") < 0) {
    throw EncoderError("Failed to apply profile");
  };

  if (x264_picture_alloc(&pic_in, param.i_csp, param.i_width, param.i_height) <
      0) {
    throw EncoderError("Failed to allocate input picture");
  };

  encoder = x264_encoder_open(&param);
  if (encoder == nullptr) {
  };
};

X264Encoder::~X264Encoder() {
  x264_encoder_close(encoder);
  x264_picture_clean(&pic_in);
};

void X264Encoder::encode_frame(uint8_t *input_buf, std::ostream os) {
  x264_picture_t pic_out{};
  pic_in.img.plane[0] = input_buf;
  pic_in.i_pts = current_frame++;
  auto i_frame_size =
      x264_encoder_encode(encoder, &nal, &i_nal, &pic_in, &pic_out);
  if (i_frame_size < 0) {
    throw EncoderError("Failed to encode frame");
  };
  x264_picture_clean(&pic_out);
};

static IVideoMode *videomode = nullptr;

void VideoRecordMod::renderFrame() {
  constexpr auto image_format = IMAGE_FORMAT_RGB888;

  auto mem_required =
      ImageLoader::GetMemRequired(width, height, 1, image_format, false);
  uint8_t pic_buf[mem_required];
  videomode->ReadScreenPixels(0, 0, width, height, pic_buf, image_format);
};

void VideoRecordMod::on_enter(GumInvocationContext *context){};

void VideoRecordMod::on_leave(GumInvocationContext *context) { renderFrame(); };

VideoRecordMod::VideoRecordMod()
    : width(videomode->GetModeStereoWidth()),
      height(videomode->GetModeStereoHeight()),
      encoder(X264Encoder(width, height, 30)) {
  // TODO: We should search signatures
  const gpointer SCR_UPDATESCREEN_OFFSET = reinterpret_cast<gpointer>(0x39ea70);
  // const gpointer SHADER_SWAPBUFFERS_OFFSET =
  // reinterpret_cast<gpointer>(0x39f660);
  const gpointer VIDEOMODE_OFFSET = reinterpret_cast<gpointer>(0xab39e0);
  const gpointer SND_RECORDBUFFER_OFFSET = reinterpret_cast<gpointer>(0x2813d0);

  ofile = std::ofstream("output.h264", std::ios::binary);

  const GumAddress module_base = gum_module_find_base_address("engine.so");
  void **videomode_ptr = (module_base + VIDEOMODE_OFFSET);
  videomode = static_cast<decltype(videomode)>(*videomode_ptr);

  g_Interceptor->attach(module_base + SCR_UPDATESCREEN_OFFSET, this, nullptr);
};
VideoRecordMod::~VideoRecordMod() {
  g_Interceptor->detach(this);
  // delete encoder;
};
