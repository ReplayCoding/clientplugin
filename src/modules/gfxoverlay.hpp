#pragma once
#include <SDL2/SDL.h>

#include "hook/attachmenthook.hpp"
#include "modules/modules.hpp"

class GfxOverlayMod : public IModule {
 public:
  GfxOverlayMod();
  virtual ~GfxOverlayMod();

 private:
  // Needed because we can't setup ourContext in the constructor, as we would
  // need the window
  bool have_we_inited_ui = false;
  SDL_GLContext our_context;

  int corner = 0;
  std::unique_ptr<AttachmentHookEnter> sdl_gl_swapWindow_hook;

  void init_imgui(SDL_Window* window);
  void SDL_GL_SwapWindow_handler(InvocationContext context);
};
