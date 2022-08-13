#pragma once

#include <convar.h>

// Why isn't this in the SDK?
class CCvar {
 public:
  static void SetFlags(ConCommandBase* var, int flags) {
    var->m_nFlags = flags;
  };
  static int GetFlags(ConCommandBase* var) { return var->m_nFlags; }
};
