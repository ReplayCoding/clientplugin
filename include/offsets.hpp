#include <cstdint>

namespace offsets {
// TODO: We should search signatures
// CLIENT
const uintptr_t FIREGAMEEVENT_OFFSET = static_cast<uintptr_t>(0x1150830);

// ENGINE
const uintptr_t SCR_UPDATESCREEN_OFFSET = static_cast<uintptr_t>(0x39eab0);
const uintptr_t SND_RECORDBUFFER_OFFSET = static_cast<uintptr_t>(0x281410);

const uintptr_t GETSOUNDTIME_OFFSET = static_cast<uintptr_t>(0x2648c0);

// This is in a vtable so we should probably fix that
const uintptr_t CENGINESOUNDSERVICES_SETSOUNDFRAMETIME_OFFSET =
    static_cast<uintptr_t>(0x00387a50 - 0x10000);

const uintptr_t SND_G_P = static_cast<uintptr_t>(0x00858910 - 0x10000);
const uintptr_t SND_G_LINEAR_COUNT = static_cast<uintptr_t>(0x00858900 - 0x10000);
const uintptr_t SND_G_VOL = static_cast<uintptr_t>(0x008588f0 - 0x10000);
}; // namespace offsets
