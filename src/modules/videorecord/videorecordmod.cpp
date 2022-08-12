#include <bitmap/imageformat.h>
#include <cdll_int.h>
#include <convar.h>
#include <materialsystem/imaterialsystem.h>
#include <replay/ienginereplay.h>
#include <view_shared.h>

#include <algorithm>
#include <fstream>
#include <hook/attachmenthook.hpp>
#include <hook/gum/x86patcher.hpp>
#include <interfaces.hpp>
#include <ios>
#include <memory>
#include <modules/videorecord/videorecordmod.hpp>
#include <modules/videorecord/x264encoder.hpp>
#include <offsets.hpp>
#include <ostream>
#include <plugin.hpp>
#include <string>
#include "modules/modules.hpp"

void VideoRecordMod::renderAudioFrame() {
  if (!pe_render.GetBool())
    return;

  int16_t clipped_samples[*snd_linear_count];
  for (auto i = 0; i < (*snd_linear_count); i++) {
    auto sample = (*snd_p)[i] * (*snd_vol) >> 8;

    // Thank fucking god the AM SDK exists
    auto clipped_sample = std::clamp(sample, -0x7fff, 0x7fff);
    clipped_samples[i] = clipped_sample;
  };
  o_audfile.write(reinterpret_cast<char*>(clipped_samples),
                  (*snd_linear_count) * sizeof(int16_t));
};

void VideoRecordMod::renderVideoFrame() {
  if (!pe_render.GetBool())
    return;

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

void VideoRecordMod::initRenderTexture(IMaterialSystem* materialSystem) {
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
  Interfaces.engineClientReplay->InitSoundRecord();
  width = mode.m_Width;
  height = mode.m_Height;

  o_vidfile = std::ofstream("output.h264", std::ios::binary);
  o_audfile = std::ofstream("output.aud", std::ios::binary);

  const GumAddress module_base = gum_module_find_base_address("engine.so");

  snd_vol = reinterpret_cast<int*>(module_base + offsets::SND_G_VOL);
  snd_p = reinterpret_cast<int**>(module_base + offsets::SND_G_P);
  snd_linear_count =
      reinterpret_cast<int*>(module_base + offsets::SND_G_LINEAR_COUNT);

  getSoundTime_patch = std::make_unique<X86Patcher>(
      module_base + offsets::GETSOUNDTIME_OFFSET + 0x69, 2,
      [](auto x86writer) { gum_x86_writer_put_nop_padding(x86writer, 2); });

  setSoundFrameTime_patch = std::make_unique<X86Patcher>(
      module_base + offsets::CENGINESOUNDSERVICES_SETSOUNDFRAMETIME_OFFSET + 6,
      7, [](auto x86writer) { gum_x86_writer_put_nop_padding(x86writer, 7); });

  scr_updateScreen_hook = std::make_unique<AttachmentHookLeave>(
      module_base + offsets::SCR_UPDATESCREEN_OFFSET,
      [this](InvocationContext) { renderVideoFrame(); });
  snd_recordBuffer_hook = std::make_unique<AttachmentHookLeave>(
      module_base + offsets::SND_RECORDBUFFER_OFFSET,
      [this](InvocationContext) { renderAudioFrame(); });
};
VideoRecordMod::~VideoRecordMod(){};

REGISTER_MODULE(VideoRecordMod)
