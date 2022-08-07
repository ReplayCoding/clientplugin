#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

#include <gum/interceptor.hpp>
#include <modules/gfxoverlaymod.hpp>
#include <plugin.hpp>

void GfxOverlayMod::init_imgui(SDL_Window* window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForOpenGL(window, ourContext);
  ImGui_ImplOpenGL3_Init();
};

void GfxOverlayMod::on_enter(GumInvocationContext* context) {
  auto hookType = reinterpret_cast<int>(
      gum_invocation_context_get_listener_function_data(context));
  if (hookType != static_cast<int>(HookType::SDL_GL_SwapWindow))
    return;

  auto window = static_cast<SDL_Window*>(
      gum_invocation_context_get_nth_argument(context, 0));
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
    ImGui::Separator();
    if (ImGui::IsMousePosValid())
      ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
    else
      ImGui::Text("Mouse Position: <invalid>");
  }
  ImGui::End();
  /* END DRAWING */

  ImGui::Render();
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  SDL_GL_MakeCurrent(window, theirContext);
};

void GfxOverlayMod::on_leave(GumInvocationContext* context) {
  auto hookType = reinterpret_cast<int>(
      gum_invocation_context_get_listener_function_data(context));
  if (hookType != static_cast<int>(HookType::SDL_PollEvent))
    return;

  auto doProcessEvent =
      reinterpret_cast<int>(gum_invocation_context_get_return_value(context));

  if (haveWeInitedUI && doProcessEvent) {
    // ImGuiIO &io = ImGui::GetIO();
    SDL_Event* event = static_cast<SDL_Event*>(
        gum_invocation_context_get_nth_argument(context, 0));
    ImGui_ImplSDL2_ProcessEvent(event);
  };
};

GfxOverlayMod::GfxOverlayMod() : Listener() {
  SDL_version sdlversion;
  SDL_GetVersion(&sdlversion);
  g_Interceptor->attach(reinterpret_cast<void*>(SDL_GL_SwapWindow), this,
                        reinterpret_cast<void*>(HookType::SDL_GL_SwapWindow));
  g_Interceptor->attach(reinterpret_cast<void*>(SDL_PollEvent), this,
                        reinterpret_cast<void*>(HookType::SDL_PollEvent));
};
GfxOverlayMod::~GfxOverlayMod() {
  g_Interceptor->detach(this);
  // Do this AFTER we detach listener to avoid it being called with a deleted
  // context
  SDL_GL_DeleteContext(ourContext);
};
