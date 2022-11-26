#define private public
#include <vprof.h>
#undef private

#include <absl/container/inlined_vector.h>
#include <fmt/core.h>
#include <tracy/TracyC.h>
#include <cstring>
#include <memory>
#include <mutex>
#include <range/v3/view/enumerate.hpp>
#include <shared_mutex>
#include <stack>
#include <string_view>

#include "hook/attachmenthook.hpp"
#include "hook/gum/interceptor.hpp"
#include "modules/modules.hpp"
#include "offsets/offsets.hpp"

template <typename T, size_t size>
using InlineStack = std::stack<T, absl::InlinedVector<T, size>>;

class ContextStack {
 public:
  ContextStack() = default;
  ~ContextStack() { clear(); }

  void push(uint32_t line,
            std::string_view source,
            std::string_view function,
            std::string_view name) {
    uint64 location = ___tracy_alloc_srcloc_name(
        line, source.data(), source.length(), function.data(),
        function.length(), name.data(), name.length());

    TracyCZoneCtx ctx = ___tracy_emit_zone_begin_alloc(location, true);
    ctx_stack.push(ctx);
  }

  void pop() {
    ___tracy_emit_zone_end(ctx_stack.top());
    ctx_stack.pop();
  }

  void clear() {
    while (!ctx_stack.empty())
      pop();
  }

 private:
  InlineStack<TracyCZoneCtx, 32> ctx_stack{};
};

// Not using SDK ver. here, because it requires the Telemetry SDK
struct Telemetry {
  /* HTELEMETRY */ void* tmContext[32];
  float flRDTSCToMilliSeconds;  // Conversion from tmFastTime() (rdtsc) to
                                // milliseconds.
  uint32_t FrameCount;      // Count of frames to capture before turning off.
  char ServerAddress[128];  // Server name to connect to.
  int playbacktick;  // GetPlaybackTick() value from demo file (or 0 if not
                     // playing a demo).
  uint32_t DemoTickStart;  // Start telemetry on demo tick #
  uint32_t DemoTickEnd;    // End telemetry on demo tick #
  uint32_t Level;  // Current Telemetry level (Use TelemetrySetLevel to modify)
};

constexpr std::string_view UNAVAILABLE = "unavailable";
constexpr bool IS_TELEMETRY = true;

static thread_local ContextStack ctx_stack{};
static uint32_t last_observed_telemetry_level;
static std::shared_mutex telemetry_mutex{};

class TelemetryReplacement {};

void TelemetryTick_replacement() {
  auto telemetry = static_cast<Telemetry*>(offsets::g_Telemetry);

  std::shared_lock rlock(telemetry_mutex);
  if (telemetry->Level != last_observed_telemetry_level) {
    rlock.unlock();
    std::unique_lock wlock(telemetry_mutex);

    if (last_observed_telemetry_level == UINT32_MAX) {
      // Clear them out just in case these somehow got inited before
      // we take control
      std::memset(telemetry->tmContext, 0, sizeof(telemetry->tmContext));
    } else {
      telemetry->tmContext[last_observed_telemetry_level] = nullptr;
    }

    fmt::print("telems level: {}, old level: {}\n", telemetry->Level,
               last_observed_telemetry_level);
    last_observed_telemetry_level = telemetry->Level;
  }
}

class ProfilerMod : public IModule {
 public:
  ProfilerMod();
  ~ProfilerMod();

 private:
  std::unique_ptr<AttachmentHookEnter> enter_node_hook;
  std::unique_ptr<AttachmentHookEnter> exit_node_hook;

  std::unique_ptr<AttachmentHookEnter> frame_hook;
};

ProfilerMod::ProfilerMod() {
  last_observed_telemetry_level = UINT32_MAX;

  if (!IS_TELEMETRY) {
    enter_node_hook = std::make_unique<AttachmentHookEnter>(
        offsets::CVProfNode_EnterScope, [](InvocationContext context) {
          auto self = context.get_arg<CVProfNode*>(0);
          ctx_stack.push(0, UNAVAILABLE, UNAVAILABLE, self->m_pszName);
        });

    exit_node_hook = std::make_unique<AttachmentHookEnter>(
        offsets::CVProfNode_ExitScope,
        [](InvocationContext context) { ctx_stack.pop(); });
  } else {
    g_Interceptor->replace(
        offsets::TelemetryTick,
        reinterpret_cast<std::uintptr_t>(TelemetryTick_replacement), nullptr);
  }

  frame_hook = std::make_unique<AttachmentHookEnter>(
      offsets::CEngine_Frame,
      [](InvocationContext context) { ___tracy_emit_frame_mark(nullptr); });
}

ProfilerMod::~ProfilerMod() {
  if (IS_TELEMETRY) {
    g_Interceptor->revert(offsets::TelemetryTick);
  }
}

REGISTER_MODULE(ProfilerMod)
