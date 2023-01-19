#include <convar.h>
#include <edict.h>
#include <filesystem.h>
#include <fmt/core.h>
#include <game/server/iplayerinfo.h>
#include <strtools.h>
#include <tier2.h>
#include <memory>

#include "hook/hook.hpp"
#include "interfaces.hpp"
#include "modules/modules.hpp"
#include "offsets/offsets.hpp"

class NavWorkshopMod : public IModule {
 public:
  NavWorkshopMod();

 private:
  std::unique_ptr<ReplacementHook> get_nav_data_hook;
};

const char* GetCleanMapName(const char* inp, char* out)

{
  const char* clean_name;

  clean_name = StringAfterPrefixCaseSensitive(inp, "maps/workshop/");
  if ((clean_name == NULL) &&
      (clean_name = StringAfterPrefixCaseSensitive(inp, "maps\\workshop\\"),
       clean_name == NULL)) {
    clean_name = StringAfterPrefixCaseSensitive(inp, "workshop/");
    if ((clean_name == NULL) &&
        (clean_name = StringAfterPrefixCaseSensitive(inp, "workshop\\"),
         clean_name == NULL)) {
      return inp;
    }
    V_strncpy(out, clean_name, 0x100);
  } else {
    V_strncpy(out, "maps/", 0x100);
    V_strncat(out, clean_name, 0x100, -1);
  }

  auto ugc_loc = strstr(out, ".ugc");
  if (ugc_loc != nullptr) {
    *ugc_loc = '\0';
  }

  return out;
}

enum class NAV_ERROR_TYPE : int { OK = 0, CANT_ACCESS_FILE };

NAV_ERROR_TYPE GetNavDataFromFile_replace(void* this_,
                                          CUtlBuffer& outBuffer,
                                          bool* pNavDataFromBSP) {
  char cleaned_map_name[256];

  auto mapname =
      Interfaces::PlayerInfoManager->GetGlobalVars()->mapname.ToCStr();
  // if (mapname == nullptr) {
  //   mapname = "";
  // }

  char nav_path[260];
  V_snprintf(nav_path, sizeof(nav_path), "maps\\%s.nav", mapname);
  // Try to load "maps/<NON-clean map name>" from mod files, ignoring files in
  // BSP.
  if (!g_pFullFileSystem->ReadFile(nav_path, "MOD", outBuffer, 0, 0, 0)) {
    char clean_nav_path[260];
    GetCleanMapName(mapname, cleaned_map_name);
    V_snprintf(clean_nav_path, sizeof(clean_nav_path), "maps\\%s.nav",
               cleaned_map_name);
    // Couldn't find a navfile in mod files, fallback to "maps/<clean map
    // name>.nav" file in BSP
    if (!g_pFullFileSystem->ReadFile(clean_nav_path, "BSP", outBuffer, 0, 0,
                                     0)) {
      // Couldn't find "maps/<clean map name>.nav" in BSP, fallback to
      // "maps/embed.nav"
      if (!g_pFullFileSystem->ReadFile("maps\\embed.nav", "BSP", outBuffer, 0,
                                       0, 0)) {
        return NAV_ERROR_TYPE::CANT_ACCESS_FILE;
      }
    }

    if (pNavDataFromBSP != 0) {
      *pNavDataFromBSP = true;
    }
  }
  return NAV_ERROR_TYPE::OK;
}

NavWorkshopMod::NavWorkshopMod() {
  get_nav_data_hook = std::make_unique<ReplacementHook>(
      offsets::CNavMesh_GetNavDataFromFile, GetNavDataFromFile_replace);
}

REGISTER_MODULE(NavWorkshopMod)
