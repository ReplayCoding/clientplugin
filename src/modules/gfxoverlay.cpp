#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <memory>

#include "guifont.hpp"
#include "hook/attachmenthook.hpp"
#include "modules/gfxoverlay.hpp"
#include "modules/modules.hpp"

void GfxOverlayMod::init_imgui(SDL_Window* window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForOpenGL(window, our_context);
  ImGui_ImplOpenGL3_Init();

  auto& io = ImGui::GetIO();
  ImFontConfig font_config;
  font_config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LightHinting;
  io.Fonts->AddFontFromMemoryCompressedTTF(
      gui_font_compressed_data, gui_font_compressed_size, 16, &font_config);
}

void GfxOverlayMod::SDL_GL_SwapWindow_handler(InvocationContext context) {
  auto window = context.get_arg<SDL_Window*>(0);
  auto their_context = SDL_GL_GetCurrentContext();

  if (!have_we_inited_ui) {
    our_context = SDL_GL_CreateContext(window);

    SDL_GL_MakeCurrent(window, our_context);
    init_imgui(window);

    have_we_inited_ui = true;
  } else {
    SDL_GL_MakeCurrent(window, our_context);
  };

  ImGuiIO& io = ImGui::GetIO();

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  /* Begin drawing, this is just a test of imgui currently */
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
      ImGuiWindowFlags_NoNav;

  if (corner != -1) {
    const float PAD = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos =
        viewport->WorkPos;  // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x =
        (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
    window_pos.y =
        (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
    window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
    window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    window_flags |= ImGuiWindowFlags_NoMove;
  }

  for (auto& module : *modules) {
    if (module->should_draw_overlay()) {
      if (ImGui::Begin("", nullptr, window_flags)) {
        ImGui::SetNextWindowBgAlpha(0.75f);  // Transparent background

        module->draw_overlay();

        ImGui::End();
      }
    }
  }

  ImGui::End();
  /* End drawing */

  ImGui::Render();
  glViewport(0, 0, static_cast<int>(io.DisplaySize.x),
             static_cast<int>(io.DisplaySize.y));
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  SDL_GL_MakeCurrent(window, their_context);
}

GfxOverlayMod::GfxOverlayMod(
    std::vector<std::unique_ptr<IModule>>* modules_ref) {
  SDL_version sdlversion;
  SDL_GetVersion(&sdlversion);

  modules = modules_ref;

  sdl_gl_swapWindow_hook = std::make_unique<AttachmentHookEnter>(
      reinterpret_cast<std::uintptr_t>(SDL_GL_SwapWindow),
      [this](auto context) { SDL_GL_SwapWindow_handler(context); });
}
GfxOverlayMod::~GfxOverlayMod() {
  sdl_gl_swapWindow_hook.reset();
  // Do this AFTER we detach to avoid it being called with a deleted
  // context
  SDL_GL_DeleteContext(our_context);
}

// Manually registered so we can provide some extra stuff
