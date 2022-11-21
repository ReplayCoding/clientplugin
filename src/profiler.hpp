#include <memory>

#include "hook/attachmenthook.hpp"

class Profiler {
 public:
  Profiler();

 private:
  std::unique_ptr<AttachmentHookEnter> enter_node_hook;
  std::unique_ptr<AttachmentHookEnter> exit_node_hook;
  std::mutex m{};
};
