#pragma once
#include <SDL2/SDL.h>

#include <gum/interceptor.hpp>
#include <modules/modules.hpp>
#include "hook/attachmenthook.hpp"

class GfxOverlayMod : public IModule {
 public:
  GfxOverlayMod();
  virtual ~GfxOverlayMod();

 private:
  // Needed because we can't setup ourContext in the constructor, as we would
  // need the window
  bool haveWeInitedUI = false;
  SDL_GLContext ourContext;

  int corner = 0;
  std::unique_ptr<AttachmentHookEnter> sdl_gl_swapWindow_hook;

  void init_imgui(SDL_Window* window);
  void SDL_GL_SwapWindow_handler(GumInvocationContext* context);
};
