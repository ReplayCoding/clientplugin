#include <tracy/TracyC.h>
#include <cwchar>

#include "hook/hook.hpp"
#include "interfaces.hpp"
#include "modules/modules.hpp"
#include "offsets/offsets.hpp"

class PartyChatMod : public IModule {
 public:
  PartyChatMod();

 private:
  // std::unique_ptr<AttachmentHookBoth> party_name_hook;
  std::unique_ptr<ReplacementHook> party_name_hook;
};

wchar_t* GetPlayerNameForSteamID_replace(wchar_t* name,
                                         uint32_t len,
                                         uint64_t* id) {
  wcsncpy(name, L"TEST", len / sizeof(wchar_t));

  return name;
}

thread_local TracyCZoneCtx party_ctx;

PartyChatMod::PartyChatMod() {
  // party_name_hook = std::make_unique<AttachmentHookBoth>(
  //     offsets::GetPlayerNameForSteamID,
  //     [](InvocationContext context) {
  //       TracyCZoneN(ctx, "GetPlayerNameForSteamID", true);
  //       party_ctx = ctx;
  //       // auto name = context.get_arg<wchar_t*>(0);
  //       // printf("%ls ----\n", name);
  //     },
  //     [](InvocationContext context) { TracyCZoneEnd(party_ctx); });
  party_name_hook = std::make_unique<ReplacementHook>(
      offsets::GetPlayerNameForSteamID, GetPlayerNameForSteamID_replace);
}

REGISTER_MODULE(PartyChatMod)
