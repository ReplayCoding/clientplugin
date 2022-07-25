#include <algorithm>
#include <fstream>
#include <gum/interceptor.hpp>
#include <interfaces.hpp>
#include <ios>
#include <memory>
#include <modules/videorecordmod.hpp>
#include <ostream>
#include <plugin.hpp>
#include <string>
#include <x264.h>

#include <bitmap/imageformat.h>
#include <cdll_int.h>
#include <convar.h>
#include <materialsystem/imaterialsystem.h>
#include <replay/ienginereplay.h>
#include <view_shared.h>

X264Encoder::X264Encoder(int width, int height, int fps, std::string preset,
                         std::ostream &output_stream)
    : output_stream(output_stream) {
  if (x264_param_default_preset(&param, "ultrafast", nullptr) < 0) {
    throw "Failed to create set params";
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
  // For some weird reason, the engine crashes if I set this any higher
  param.i_threads = 1;

  if (x264_param_apply_profile(&param, "high444") < 0) {
    throw "Failed to apply profile";
  };

  if (x264_picture_alloc(&pic_in, param.i_csp, param.i_width, param.i_height) <
      0) {
    throw "Failed to allocate input picture";
  };

  encoder = x264_encoder_open(&param);
  if (encoder == nullptr) {
    throw "Failed to open encoder";
  };
};

X264Encoder::~X264Encoder() {
  while (x264_encoder_delayed_frames(encoder)) {
    auto i_frame_size =
        x264_encoder_encode(encoder, &nal, &i_nal, NULL, &pic_out);
    if (i_frame_size < 0) {
      throw "Failed to encode delayed frames";
    };
    if (i_frame_size > 0) {
      output_stream.write(nal->p_payload, i_frame_size);
    }
  }

  x264_encoder_close(encoder);
  // WHAT THE FUCK IT CRASHES MY ASS
  // x264_picture_clean(&pic_in);
};

void X264Encoder::encode_frame(uint8_t *input_buf) {
  pic_in.img.plane[0] = input_buf;
  pic_in.i_pts = current_frame++;
  auto i_frame_size =
      x264_encoder_encode(encoder, &nal, &i_nal, &pic_in, &pic_out);
  if (i_frame_size < 0) {
    throw "Failed to encode frame";
  };
  if (i_frame_size > 0) {
    output_stream.write(nal->p_payload, i_frame_size);
  };
};

void VideoRecordMod::renderAudioFrame() {
  for (auto i = 0; i < (*snd_linear_count); i++) {
    auto sample = (*snd_p)[i] * (*snd_vol) >> 8;
    auto clipped_sample = std::clamp(sample, -0x7fff, 0x7fff);
    g_print("%d ", clipped_sample);
  };
  g_print("\n");
};

void VideoRecordMod::renderVideoFrame() {
  // Width and height can't be set when the module is being setup
  if (encoder == nullptr) {
    encoder =
        std::make_unique<X264Encoder>(width, height, 30, "ultrafast", ofile);
  };

  constexpr auto image_format = IMAGE_FORMAT_RGB888;

  if (pe_render.GetBool()) {
    CViewSetup viewSetup{};
    CMatRenderContextPtr renderContextPtr(Interfaces.materialSystem);
    renderContextPtr->PushRenderTargetAndViewport(renderTexture, 0, 0, width,
                                                  height);
    auto gotPlayerView = Interfaces.clientDll->GetPlayerView(viewSetup);
    if (gotPlayerView) {
      Interfaces
          .clientDll->RenderView(viewSetup, VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH,
                                 RENDERVIEW_DRAWVIEWMODEL | RENDERVIEW_DRAWHUD /* This seems to be broken when the menu is up */);
    };
    auto mem_required =
        ImageLoader::GetMemRequired(width, height, 1, image_format, false);
    uint8_t pic_buf[mem_required];
    renderContextPtr->ReadPixels(0, 0, width, height, pic_buf, image_format);
    encoder->encode_frame(pic_buf);
    renderContextPtr->PopRenderTargetAndViewport();
  };
};

void VideoRecordMod::on_enter(GumInvocationContext *context){};

void VideoRecordMod::on_leave(GumInvocationContext *context) {
  auto hookType = reinterpret_cast<int>(
      gum_invocation_context_get_listener_function_data(context));
  switch (hookType) {
    case SCR_UpdateScreen:
      renderVideoFrame();
      break;
    case SND_RecordBuffer:
      renderAudioFrame();
      break;
  };
};

void VideoRecordMod::initRenderTexture(IMaterialSystem *materialSystem) {
  materialSystem->BeginRenderTargetAllocation();
  renderTexture.Init(materialSystem->CreateNamedRenderTargetTextureEx2(
      "_rt_pe_rendertexture", width, height, RT_SIZE_OFFSCREEN,
      IMAGE_FORMAT_RGB888, MATERIAL_RT_DEPTH_SHARED));
  materialSystem->EndRenderTargetAllocation();
};

VideoRecordMod::VideoRecordMod()
    : pe_render("pe_render", 0, FCVAR_DONTRECORD, "Render using x264") {
  // TODO: We should search signatures
  const uintptr_t SCR_UPDATESCREEN_OFFSET = static_cast<uintptr_t>(0x39ea70);
  // const uintptr_t SHADER_SWAPBUFFERS_OFFSET =
  // static_cast<uintptr_t>(0x39f660);
  // const uintptr_t VIDEOMODE_OFFSET = static_cast<uintptr_t>(0xab39e0);
  const uintptr_t SND_RECORDBUFFER_OFFSET = static_cast<uintptr_t>(0x2813d0);
  const uintptr_t SND_G_VOL = static_cast<uintptr_t>(0x8488f0);
  const uintptr_t SND_G_P = static_cast<uintptr_t>(0x848910);
  const uintptr_t SND_G_LINEAR_COUNT = static_cast<uintptr_t>(0x848900);
  MaterialVideoMode_t mode;
  Interfaces.materialSystem->GetDisplayMode(mode);
  width = mode.m_Width;
  height = mode.m_Height;

  ofile = std::ofstream("output.h264", std::ios::binary);

  const GumAddress module_base = gum_module_find_base_address("engine.so");

  snd_vol = reinterpret_cast<int *>(module_base + SND_G_VOL);
  snd_p = reinterpret_cast<void **>(module_base + SND_G_P);
  snd_linear_count = reinterpret_cast<int *>(module_base + SND_G_LINEAR_COUNT);
  g_Interceptor->attach(module_base + SCR_UPDATESCREEN_OFFSET, this,
                        reinterpret_cast<void *>(HookType::SCR_UpdateScreen));
  g_Interceptor->attach(module_base + SND_RECORDBUFFER_OFFSET, this,
                        reinterpret_cast<void *>(HookType::SND_RecordBuffer));
};
VideoRecordMod::~VideoRecordMod() { g_Interceptor->detach(this); };
