#include <algorithm>
#include <fstream>
#include <gum/interceptor.hpp>
#include <gum/x86patcher.hpp>
#include <interfaces.hpp>
#include <ios>
#include <memory>
#include <modules/videorecordmod.hpp>
#include <offsets.hpp>
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
  if (x264_param_default_preset(&param, preset.c_str(), nullptr) < 0) {
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
      output_stream.write(reinterpret_cast<const char *>(nal->p_payload),
                          i_frame_size);
    }
  }

  // WHAT THE FUCK IT CRASHES MY ASS
  // this is a memory leak. deal with it
  // x264_picture_clean(&pic_in);

  x264_encoder_close(encoder);
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
    output_stream.write(reinterpret_cast<const char *>(nal->p_payload),
                        i_frame_size);
  };
};

void VideoRecordMod::renderAudioFrame() {
  int16_t clipped_samples[*snd_linear_count];
  for (auto i = 0; i < (*snd_linear_count); i++) {
    auto sample = (*snd_p)[i] * (*snd_vol) >> 8;

    // Thank fucking god the AM SDK exists
    auto clipped_sample = std::clamp(sample, -0x7fff, 0x7fff);
    clipped_samples[i] = clipped_sample;
  };
  o_audfile.write(reinterpret_cast<char *>(clipped_samples),
                  (*snd_linear_count) * sizeof(int16_t));
};

void VideoRecordMod::renderVideoFrame() {
  // Width and height can't be set when the module is being setup
  if (encoder == nullptr) {
    encoder =
        std::make_unique<X264Encoder>(width, height, 30, "medium", o_vidfile);
  };

  constexpr auto image_format = IMAGE_FORMAT_RGB888;

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
  // o_vidfile.write(reinterpret_cast<const char *>(pic_buf), mem_required);
  renderContextPtr->PopRenderTargetAndViewport();
};

void VideoRecordMod::on_enter(GumInvocationContext *context){};

void VideoRecordMod::on_leave(GumInvocationContext *context) {
  if (pe_render.GetBool()) {
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
  MaterialVideoMode_t mode;
  Interfaces.materialSystem->GetDisplayMode(mode);
  width = mode.m_Width;
  height = mode.m_Height;

  o_vidfile = std::ofstream("output.h264", std::ios::binary);
  o_audfile = std::ofstream("output.aud", std::ios::binary);

  const GumAddress module_base = gum_module_find_base_address("engine.so");

  snd_vol = reinterpret_cast<int *>(module_base + offsets::SND_G_VOL);
  snd_p = reinterpret_cast<int **>(module_base + offsets::SND_G_P);
  snd_linear_count =
      reinterpret_cast<int *>(module_base + offsets::SND_G_LINEAR_COUNT);

  getSoundTime_patch = std::make_unique<X86Patcher>(
      module_base + offsets::GETSOUNDTIME_OFFSET + 0x69, 2,
      [](auto x86writer) { gum_x86_writer_put_nop_padding(x86writer, 2); });

  setSoundFrameTime_patch = std::make_unique<X86Patcher>(
      module_base + offsets::CENGINESOUNDSERVICES_SETSOUNDFRAMETIME_OFFSET + 6,
      7, [](auto x86writer) { gum_x86_writer_put_nop_padding(x86writer, 7); });

  Interfaces.engineClientReplay->InitSoundRecord();
  g_Interceptor->attach(module_base + offsets::SCR_UPDATESCREEN_OFFSET, this,
                        reinterpret_cast<void *>(HookType::SCR_UpdateScreen));
  g_Interceptor->attach(module_base + offsets::SND_RECORDBUFFER_OFFSET, this,
                        reinterpret_cast<void *>(HookType::SND_RecordBuffer));
};
VideoRecordMod::~VideoRecordMod() { g_Interceptor->detach(this); };
