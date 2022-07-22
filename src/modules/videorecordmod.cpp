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
#include <cdll_int.h>
#include <convar.h>
#include <materialsystem/imaterialsystem.h>
#include <replay/ienginereplay.h>
#include <view_shared.h>

ConVar pe_render("pe_render", 0, FCVAR_DONTRECORD, "Render using x264");

X264Encoder::X264Encoder(int width, int height, int fps) {
  if (x264_param_default_preset(&param, "medium", nullptr) < 0) {
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
  param.i_timebase_num = fps;
  param.i_timebase_den = 1;
  param.b_vfr_input = 0;

  param.i_log_level = X264_LOG_DEBUG;
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
  x264_encoder_close(encoder);
  // WHAT THE FUCK IT CRASHES MY ASS
  // x264_picture_clean(&pic_in);
};

void X264Encoder::encode_frame(uint8_t *input_buf, std::ostream *output) {
  pic_in.img.plane[0] = input_buf;
  pic_in.i_pts = current_frame++;
  auto i_frame_size =
      x264_encoder_encode(encoder, &nal, &i_nal, &pic_in, &pic_out);
  if (i_frame_size < 0) {
    throw "Failed to encode frame";
  };
  if (i_frame_size > 0) {
    output->write(nal->p_payload, i_frame_size);
  };
};

void VideoRecordMod::renderFrame() {
  // Width and height can't be set when the module is being setup
  if (encoder == nullptr) {
    encoder = new X264Encoder(width, height, 30);
  };

  constexpr auto image_format = IMAGE_FORMAT_RGB888;

  if (pe_render.GetBool()) {
    CViewSetup viewSetup{};
    CMatRenderContextPtr renderContextPtr(Interfaces.materialSystem);
    renderContextPtr->PushRenderTargetAndViewport(renderTexture, 0, 0, width,
                                                  height);
    auto gotPlayerView = Interfaces.clientDll->GetPlayerView(viewSetup);
    if (gotPlayerView) {
      Interfaces.clientDll->RenderView(
          viewSetup, VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH,
          RENDERVIEW_DRAWVIEWMODEL |
              RENDERVIEW_DRAWHUD /* This seems to be broken */);
    };
    auto mem_required =
        ImageLoader::GetMemRequired(width, height, 1, image_format, false);
    uint8_t pic_buf[mem_required];
    renderContextPtr->ReadPixels(0, 0, width, height, pic_buf, image_format);
    encoder->encode_frame(pic_buf, &ofile);
    renderContextPtr->PopRenderTargetAndViewport();
  };
};

void VideoRecordMod::on_enter(GumInvocationContext *context){};

void VideoRecordMod::on_leave(GumInvocationContext *context) { renderFrame(); };

void VideoRecordMod::initRenderTexture(IMaterialSystem *materialSystem) {
  materialSystem->BeginRenderTargetAllocation();
  renderTexture.Init(materialSystem->CreateNamedRenderTargetTextureEx2(
      "_rt_pe_rendertexture", width, height, RT_SIZE_OFFSCREEN,
      IMAGE_FORMAT_RGB888, MATERIAL_RT_DEPTH_SHARED));
  materialSystem->EndRenderTargetAllocation();
};

VideoRecordMod::VideoRecordMod() {
  // TODO: We should search signatures
  const gpointer SCR_UPDATESCREEN_OFFSET = reinterpret_cast<gpointer>(0x39ea70);
  // const gpointer SHADER_SWAPBUFFERS_OFFSET =
  // reinterpret_cast<gpointer>(0x39f660);
  // const gpointer VIDEOMODE_OFFSET = reinterpret_cast<gpointer>(0xab39e0);
  const gpointer SND_RECORDBUFFER_OFFSET = reinterpret_cast<gpointer>(0x2813d0);
  MaterialVideoMode_t mode;
  Interfaces.materialSystem->GetDisplayMode(mode);
  width = mode.m_Width;
  height = mode.m_Height;

  ofile = std::ofstream("output.h264", std::ios::binary);

  const GumAddress module_base = gum_module_find_base_address("engine.so");

  g_Interceptor->attach(module_base + SCR_UPDATESCREEN_OFFSET, this, nullptr);
};
VideoRecordMod::~VideoRecordMod() {
  g_Interceptor->detach(this);
  if (encoder != nullptr) {
    delete encoder;
  };
};
