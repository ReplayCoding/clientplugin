#define private public
#include <vprof.h>
#undef private

#include <fmt/core.h>
#include <tracy/TracyC.h>
#include <deque>
#include <memory>
#include <stack>

#include "hook/attachmenthook.hpp"
#include "modules/modules.hpp"
#include "modules/profiler.hpp"
#include "offsets/offsets.hpp"

#define UNAVAILABLE "unavailable"
#define UNAVAILABLE_LEN strlen(UNAVAILABLE)

thread_local std::stack<TracyCZoneCtx> ctx_stack{};

ProfilerMod::ProfilerMod() {
  enter_node_hook = std::make_unique<AttachmentHookEnter>(
      offsets::CVProfNode_EnterScope,
      [this](InvocationContext context) {
        auto self = context.get_arg<CVProfNode*>(0);

        uint64 location = ___tracy_alloc_srcloc_name(
            0, UNAVAILABLE, UNAVAILABLE_LEN, UNAVAILABLE, UNAVAILABLE_LEN,
            self->m_pszName, strlen(self->m_pszName));

        m.lock();

        TracyCZoneCtx ctx = ___tracy_emit_zone_begin_alloc(location, true);
        ctx_stack.push(ctx);

        m.unlock();
      });

  exit_node_hook = std::make_unique<AttachmentHookEnter>(
      offsets::CVProfNode_ExitScope,
      [this](auto context) {
        m.lock();

        ___tracy_emit_zone_end(ctx_stack.top());
        ctx_stack.pop();

        m.unlock();
      });
}

REGISTER_MODULE(ProfilerMod)
