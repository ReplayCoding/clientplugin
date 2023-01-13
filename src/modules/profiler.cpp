#define private public
#include <vprof.h>
#undef private

#include <absl/container/flat_hash_map.h>
#include <absl/container/inlined_vector.h>
#include <convar.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <imgui.h>
#include <threadtools.h>
#include <tracy/TracyC.h>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stack>
#include <string_view>
#include <tracy/Tracy.hpp>

#include "hook/hook.hpp"
#include "modules/modules.hpp"
#include "modules/telemetrystubs.hpp"
#include "offsets/offsets.hpp"

#define TELEMETRY_FORMAT(name, format)             \
  char name[256] = {0};                            \
  va_list args;                                    \
  va_start(args, format);                          \
  vsnprintf(name, sizeof(name) - 1, format, args); \
  va_end(args);

constexpr std::string_view UNAVAILABLE = "unavailable";
constexpr bool IS_TELEMETRY = true;

static ConVar profile_all_levels{"pe_profile_all_levels", 0};

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

struct TelemetryLevelState {
  uint32_t level;
  bool profile_all_levels;

  bool operator==(const TelemetryLevelState& other) const = default;
};

thread_local ContextStack vprof_ctx_stack{};
thread_local ContextStack telemetry_ctx_stack{};

TelemetryLevelState telemetry_level_state{};
thread_local TelemetryLevelState last_observed_telemetry_state_per_thread{};
std::shared_mutex telemetry_level_mutex{};

class TelemetryReplacement : TM_API_STRUCT_STUB {
 public:
  TelemetryReplacement() {
    // Brokened
    // this->tmCoreMessage = tmCoreMessage_replace;
    // this->tmCoreDynamicString = tmCoreDynamicString_replace;

    this->tmCoreEnter = tmCoreEnter_replace;
    this->tmCoreLeave = tmCoreLeave_replace;
  }

 private:
  static inline bool check_level() {
    std::shared_lock l{telemetry_level_mutex};
    if (last_observed_telemetry_state_per_thread != telemetry_level_state) {
      last_observed_telemetry_state_per_thread = telemetry_level_state;
      telemetry_ctx_stack.clear();
      return true;
    }

    return false;
  }

  static const char* tmCoreDynamicString_replace(HTELEMETRY cx, char const* s) {
    if (s == nullptr)
      return "";

    auto len = strnlen(s, 127);
    auto arr = new char[len + 1];
    strncpy(arr, s, len);
    return arr;
  };

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

    // POTENTIALLY VERSION SPECIFIC HACK
    constexpr auto WAIT = "Wait";
    if (strncmp(buf, WAIT, strlen(WAIT)) == 0)
      return;

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

  if (telemetry->Level != telemetry_level_state.level ||
      telemetry_level_state.profile_all_levels !=
          profile_all_levels.GetBool()) {
    std::unique_lock l{telemetry_level_mutex};

    if (!profile_all_levels.GetBool()) {
      std::memset(telemetry->tmContext, 0, sizeof(telemetry->tmContext));
      telemetry->tmContext[telemetry->Level] = &telemetry_replacement;
    } else {
      for (size_t l = 0; l < sizeof(telemetry->tmContext) / sizeof(HTELEMETRY);
           l++) {
        telemetry->tmContext[l] = &telemetry_replacement;
      }
    }

    fmt::print("telems level: {}, old level: {}\n", telemetry->Level,
               telemetry_level_state.level);
    telemetry_level_state.level = telemetry->Level;
    telemetry_level_state.profile_all_levels = profile_all_levels.GetBool();
  }
}

class ProfilerMod : public IModule {
 public:
  ProfilerMod();

  bool should_draw_overlay() override { return TracyIsConnected; }
  void draw_overlay() override;

 private:
  std::unique_ptr<AttachmentHookEnter> enter_node_hook;
  std::unique_ptr<AttachmentHookEnter> exit_node_hook;

  std::unique_ptr<AttachmentHookEnter> frame_hook;
  std::unique_ptr<AttachmentHookEnter> thread_name_hook;

  std::unique_ptr<ReplacementHook> telemetry_tick_hook;
};

void ProfilerMod::draw_overlay() {
  ImGui::Text(
      fmt::format("Mode: {}", IS_TELEMETRY ? "Telemetry" : "VProf").c_str());
  {
    std::shared_lock l(telemetry_level_mutex);
    ImGui::Text(fmt::format("Level: {}", telemetry_level_state.level).c_str());
  }
}

ProfilerMod::ProfilerMod() {
  telemetry_level_state = {.level = UINT32_MAX,
                           .profile_all_levels = profile_all_levels.GetBool()};

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
    telemetry_tick_hook = std::make_unique<ReplacementHook>(
        offsets::TelemetryTick,
        reinterpret_cast<uintptr_t>(TelemetryTick_replacement));
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

REGISTER_MODULE(ProfilerMod)
