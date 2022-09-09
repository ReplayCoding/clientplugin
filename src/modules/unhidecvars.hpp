#include <convar.h>

#include "modules/modules.hpp"

class UnhideCVarsMod : public IModule {
 public:
  UnhideCVarsMod();

 private:
  static void unhide_cvars();

  ConCommand pe_unhide_cvars;
};
