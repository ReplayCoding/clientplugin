#define private public
#include <vprof.h>
#undef private

#include <absl/container/flat_hash_map.h>
#include <absl/container/inlined_vector.h>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <imgui.h>
#include <threadtools.h>
#include <tracy/TracyC.h>
#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <range/v3/view/enumerate.hpp>
#include <shared_mutex>
#include <stack>
#include <string_view>
#include <thread>
#include <tracy/Tracy.hpp>

#include "hook/attachmenthook.hpp"
#include "hook/gum/interceptor.hpp"
#include "modules/modules.hpp"
#include "modules/telemetrystubs.hpp"
#include "offsets/offsets.hpp"

#define TELEMETRY_FORMAT(name, format)        \
  char name[256] = {};                        \
  va_list args;                               \
  va_start(args, format);                     \
  vsnprintf(name, sizeof(buf), format, args); \
  va_end(args);

constexpr std::string_view UNAVAILABLE = "unavailable";
constexpr bool IS_TELEMETRY = true;

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
    // Ugly hack
    if (ctx_stack.empty())
      return;

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

thread_local ContextStack vprof_ctx_stack{};
thread_local ContextStack telemetry_ctx_stack{};

uint32_t last_observed_telemetry_level{};
thread_local uint32_t last_observed_telemetry_level_per_thread{};
std::shared_mutex telemetry_level_mutex{};

class TelemetryReplacement : TM_API_STRUCT_STUB {
 public:
  TelemetryReplacement() {
    this->tmCoreEnter = tmCoreEnter_replace;
    this->tmCoreLeave = tmCoreLeave_replace;
    this->tmCoreMessage = tmCoreMessage_replace;
  }

 private:
  static inline bool check_level() {
    std::shared_lock l{telemetry_level_mutex};
    if (last_observed_telemetry_level_per_thread !=
        last_observed_telemetry_level) {
      last_observed_telemetry_level_per_thread = last_observed_telemetry_level;
      telemetry_ctx_stack.clear();
      return true;
    }

    return false;
  }

  static void tmCoreMessage_replace(HTELEMETRY cx,
                                    TmU32 const kThreadId,
                                    TmU32 const kFlags,
                                    TmFormatCode* pFmtCode,
                                    char const* kpFmt,
                                    ...) {
    TELEMETRY_FORMAT(buf, kpFmt)
    ___tracy_emit_message(buf, sizeof(buf), 0);
  }

  static void tmCoreEnter_replace(HTELEMETRY cx,
                                  TmU64* matchid,
                                  TmU32 const kThreadId,
                                  TmU64 const kThreshold,
                                  TmU32 const kFlags,
                                  char const* kpLocation,
                                  TmU32 const kLine,
                                  TmFormatCode* pFmtCode,
                                  char const* kpFmt,
                                  ...) {
    if (check_level())
      return;

    std::string_view file_sv{kpLocation};
    TELEMETRY_FORMAT(buf, kpFmt)

    telemetry_ctx_stack.push(kLine, file_sv, UNAVAILABLE, buf);
  }

  static void tmCoreLeave_replace(HTELEMETRY cx,
                                  TmU64 const kMatchID,
                                  TmU32 const kThreadId,
                                  char const* kpLocation,
                                  int const kLine) {
    if (check_level())
      return;

    telemetry_ctx_stack.pop();
  }
};

TelemetryReplacement telemetry_replacement{};

// Not using SDK ver. here, because it requires the Telemetry SDK
struct TelemetryData {
  HTELEMETRY tmContext[32];
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

void TelemetryTick_replacement() {
  auto telemetry = static_cast<TelemetryData*>(offsets::g_Telemetry);

  if (telemetry->Level != last_observed_telemetry_level) {
    std::unique_lock l{telemetry_level_mutex};
    // If this is set normally, something is very wrong anyways, so just take
    // the performance hit.
    if (last_observed_telemetry_level == UINT32_MAX) {
      // Clear them out just in case these somehow got inited before
      // we take control
      std::memset(telemetry->tmContext, 0, sizeof(telemetry->tmContext));
    } else {
      telemetry->tmContext[last_observed_telemetry_level] = nullptr;
    }

    telemetry->tmContext[telemetry->Level] = &telemetry_replacement;

    fmt::print("telems level: {}, old level: {}\n", telemetry->Level,
               last_observed_telemetry_level);
    last_observed_telemetry_level = telemetry->Level;
  }
}

class ProfilerMod : public IModule {
 public:
  ProfilerMod();
  ~ProfilerMod();

  bool should_draw_overlay() override {
    return TracyIsConnected;
  }
  void draw_overlay() override;

 private:
  std::unique_ptr<AttachmentHookEnter> enter_node_hook;
  std::unique_ptr<AttachmentHookEnter> exit_node_hook;

  std::unique_ptr<AttachmentHookEnter> frame_hook;
  std::unique_ptr<AttachmentHookEnter> thread_name_hook;
};

std::atomic_flag have_thread_names_inited{};

void ProfilerMod::draw_overlay() {
  ImGui::Text(
      fmt::format("Tracy ({})", IS_TELEMETRY ? "Telemetry" : "VProf").c_str());
}

ProfilerMod::ProfilerMod() {
  last_observed_telemetry_level = UINT32_MAX;

  if (!IS_TELEMETRY) {
    enter_node_hook = std::make_unique<AttachmentHookEnter>(
        offsets::CVProfNode_EnterScope, [](InvocationContext context) {
          auto self = context.get_arg<CVProfNode*>(0);
          vprof_ctx_stack.push(0, UNAVAILABLE, UNAVAILABLE, self->m_pszName);
        });

    exit_node_hook = std::make_unique<AttachmentHookEnter>(
        offsets::CVProfNode_ExitScope,
        [](InvocationContext context) { vprof_ctx_stack.pop(); });
  } else {
    g_Interceptor->replace(
        offsets::TelemetryTick,
        reinterpret_cast<std::uintptr_t>(TelemetryTick_replacement), nullptr);
  }

  frame_hook = std::make_unique<AttachmentHookEnter>(
      offsets::CEngine_Frame,
      [this](InvocationContext context) { ___tracy_emit_frame_mark(nullptr); });

  thread_name_hook = std::make_unique<AttachmentHookEnter>(
      offsets::ThreadSetDebugName, [](InvocationContext context) {
        auto name = context.get_arg<char*>(1);
        auto id = context.get_arg<ThreadId_t>(0);
        if (id != static_cast<ThreadId_t>(-1)) {
          fmt::print(
              "Thread {} (name: {}) is being set from separate thread, will "
              "not save.\n",
              id, name);
          return;
        };

        tracy::SetThreadName(name);
      });
}

ProfilerMod::~ProfilerMod() {
  if (IS_TELEMETRY) {
    g_Interceptor->revert(offsets::TelemetryTick);
  }
}

REGISTER_MODULE(ProfilerMod)
