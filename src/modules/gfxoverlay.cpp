#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

#include "hook/attachmenthook.hpp"
#include "modules/gfxoverlay.hpp"
#include "modules/modules.hpp"
#include "plugin.hpp"

void GfxOverlayMod::init_imgui(SDL_Window* window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForOpenGL(window, ourContext);
  ImGui_ImplOpenGL3_Init();
};

void GfxOverlayMod::SDL_GL_SwapWindow_handler(InvocationContext context) {
  auto window = context.get_arg<SDL_Window*>(0);
  auto theirContext = SDL_GL_GetCurrentContext();
  if (!haveWeInitedUI) {
    ourContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, ourContext);
    init_imgui(window);
    haveWeInitedUI = true;
  } else {
    SDL_GL_MakeCurrent(window, ourContext);
  };

  ImGuiIO& io = ImGui::GetIO();

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  /* BEGIN DRAWING */
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
  ImGui::SetNextWindowBgAlpha(0.75f);  // Transparent background
  if (ImGui::Begin("Example: Simple overlay", nullptr, window_flags)) {
    ImGui::Text(
        "Simple overlay\n"
        "in the corner of the screen.\n");
  }
  ImGui::End();
  /* END DRAWING */

  ImGui::Render();
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  SDL_GL_MakeCurrent(window, theirContext);
};

GfxOverlayMod::GfxOverlayMod() {
  SDL_version sdlversion;
  SDL_GetVersion(&sdlversion);
  sdl_gl_swapWindow_hook = std::make_unique<AttachmentHookEnter>(
      reinterpret_cast<std::uintptr_t>(SDL_GL_SwapWindow),
      std::bind(&GfxOverlayMod::SDL_GL_SwapWindow_handler, this,
                std::placeholders::_1));
  // g_Interceptor->attach(reinterpret_cast<void*>(SDL_PollEvent), this,
  //                       reinterpret_cast<void*>(HookType::SDL_PollEvent));
};
GfxOverlayMod::~GfxOverlayMod() {
  sdl_gl_swapWindow_hook.reset();
  // Do this AFTER we detach to avoid it being called with a deleted
  // context
  SDL_GL_DeleteContext(ourContext);
};

REGISTER_MODULE(GfxOverlayMod);
