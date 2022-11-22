#include <memory>
#include <vector>

#include "hook/attachmenthook.hpp"
#include "modules/modules.hpp"

class ProfilerMod : public IModule {
 public:
  ProfilerMod();

 private:
  std::unique_ptr<AttachmentHookEnter> enter_node_hook;
  std::unique_ptr<AttachmentHookEnter> exit_node_hook;
  std::unique_ptr<AttachmentHookEnter> frame_hook;
};
