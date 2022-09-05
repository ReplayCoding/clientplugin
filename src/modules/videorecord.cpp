#include <bitmap/imageformat.h>
#include <cdll_int.h>
#include <convar.h>
#include <dbg.h>
#include <materialsystem/imaterialsystem.h>
#include <replay/ienginereplay.h>
#include <view_shared.h>

#include <algorithm>
#include <fstream>
#include <ios>
#include <memory>
#include <ostream>
#include <string>

#include "hook/attachmenthook.hpp"
#include "hook/gum/x86patcher.hpp"
#include "interfaces.hpp"
#include "modules/modules.hpp"
#include "modules/videorecord.hpp"
#include "offsets.hpp"
#include "util/error.hpp"
#include "x264encoder.hpp"

void VideoRecordMod::renderAudioFrame() {
  if (!isRendering)
    return;

  int16_t clipped_samples[*snd_linear_count];
  for (auto i = 0; i < (*snd_linear_count); i++) {
    auto sample = ((*snd_p)[i] * (*snd_vol)) >> 8;

    // Thank fucking god the AM SDK exists
    clipped_samples[i] = std::clamp(sample, -0x7fff, 0x7fff);
  };
  o_audfile->write(reinterpret_cast<char*>(clipped_samples),
                   (*snd_linear_count) * sizeof(int16_t));
}

void VideoRecordMod::renderVideoFrame() {
  if (!isRendering)
    return;

  CViewSetup viewSetup{};
  CMatRenderContextPtr renderContextPtr(Interfaces::MaterialSystem);
  renderContextPtr->PushRenderTargetAndViewport(renderTexture, 0, 0, width,
                                                height);
  auto gotPlayerView = Interfaces::ClientDll->GetPlayerView(viewSetup);
  if (gotPlayerView) {
    Interfaces::ClientDll->RenderView(viewSetup,
                                      VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH,
                                      RENDERVIEW_DRAWVIEWMODEL | RENDERVIEW_DRAWHUD /* This seems to be broken when the menu is up */);
  } else {
    throw StringError("Couldn't obtain player view? Whats going on!");
  };

  // constexpr auto image_format = IMAGE_FORMAT_RGB888;
  // auto mem_required =
  //     ImageLoader::GetMemRequired(width, height, 1, image_format, false);
  // uint8_t pic_buf[mem_required];
  // renderContextPtr->ReadPixels(0, 0, width, height, pic_buf, image_format);
  // encoder->encode_frame(pic_buf);
  // o_vidfile.write(reinterpret_cast<const char *>(pic_buf), mem_required);
  renderContextPtr->PopRenderTargetAndViewport();
}

void VideoRecordMod::initRenderTexture() {
  Interfaces::MaterialSystem->BeginRenderTargetAllocation();
  renderTexture.Init(
      Interfaces::MaterialSystem->CreateNamedRenderTargetTextureEx2(
          "_rt_pe_rendertexture", width, height, RT_SIZE_OFFSCREEN,
          IMAGE_FORMAT_RGB888, MATERIAL_RT_DEPTH_SHARED));
  Interfaces::MaterialSystem->EndRenderTargetAllocation();
}

void VideoRecordMod::startRender(const CCommand& c) {
  MaterialVideoMode_t mode;
  Interfaces::MaterialSystem->GetDisplayMode(mode);
  Interfaces::EngineClientReplay->InitSoundRecord();
  width = mode.m_Width;
  height = mode.m_Height;

  o_vidfile = std::make_unique<std::ofstream>("output.h264", std::ios::binary);
  o_audfile = std::make_unique<std::ofstream>("output.aud", std::ios::binary);

  // encoder =
  //     std::make_unique<X264Encoder>(width, height, 30, "medium", *o_vidfile);

  getSoundTime_patch->Enable();
  setSoundFrameTime_patch->Enable();

  isRendering = true;
}

void VideoRecordMod::stopRender(const CCommand& c) {
  getSoundTime_patch->Disable();
  setSoundFrameTime_patch->Disable();

  isRendering = false;
}

VideoRecordMod::VideoRecordMod()
    : pe_render_start_cbk([&](auto c) { startRender(c); }),
      pe_render_stop_cbk([&](auto c) { stopRender(c); }),
      pe_render_start("pe_render_stop", &pe_render_stop_cbk),
      pe_render_stop("pe_render_start", &pe_render_start_cbk) {
  snd_vol = offsets::SND_G_VOL;
  snd_p = offsets::SND_G_P;
  snd_linear_count = offsets::SND_G_LINEAR_COUNT;

  initRenderTexture();

  getSoundTime_patch = std::make_unique<X86Patcher>(
      offsets::GetSoundTime + 0x69, 2,
      [](auto x86writer) { gum_x86_writer_put_nop_padding(x86writer, 2); },
      false);

  setSoundFrameTime_patch = std::make_unique<X86Patcher>(
      offsets::CEngineSoundServices_SetSoundFrametime + 6, 7,
      [](auto x86writer) { gum_x86_writer_put_nop_padding(x86writer, 7); },
      false);

  scr_updateScreen_hook = std::make_unique<AttachmentHookLeave>(
      offsets::SCR_UpdateScreen,
      [this](InvocationContext) { renderVideoFrame(); });
  snd_recordBuffer_hook = std::make_unique<AttachmentHookLeave>(
      offsets::SND_RecordBuffer,
      [this](InvocationContext) { renderAudioFrame(); });
}
VideoRecordMod::~VideoRecordMod() {}

REGISTER_MODULE(VideoRecordMod)
