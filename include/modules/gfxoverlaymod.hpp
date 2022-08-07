#pragma once
#include <SDL2/SDL.h>

#include <gum/interceptor.hpp>
#include <modules/modules.hpp>

class GfxOverlayMod : public IModule, public Listener {
 public:
  GfxOverlayMod();
  virtual ~GfxOverlayMod();
  virtual void on_enter(GumInvocationContext* context);

  virtual void on_leave(GumInvocationContext* context);

 private:
  // Needed because we can't setup ourContext in the constructor, as we would
  // need the window
  bool haveWeInitedUI = false;
  SDL_GLContext ourContext;

  int corner = 0;

  enum class HookType : int {
    SDL_GL_SwapWindow,
    SDL_PollEvent,
  };

  void init_imgui(SDL_Window* window);
};
