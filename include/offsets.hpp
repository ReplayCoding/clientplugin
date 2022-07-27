#include <cstdint>

namespace offsets {
// TODO: We should search signatures
const uintptr_t FIREGAMEEVENT_OFFSET = static_cast<uintptr_t>(0x01150720);

const uintptr_t SCR_UPDATESCREEN_OFFSET = static_cast<uintptr_t>(0x39ea70);
// const uintptr_t SHADER_SWAPBUFFERS_OFFSET =
// static_cast<uintptr_t>(0x39f660);
// const uintptr_t VIDEOMODE_OFFSET = static_cast<uintptr_t>(0xab39e0);
const uintptr_t SND_RECORDBUFFER_OFFSET = static_cast<uintptr_t>(0x2813d0);
const uintptr_t GETSOUNDTIME_OFFSET = static_cast<uintptr_t>(0x264880);
const uintptr_t CENGINESOUNDSERVICES_SETSOUNDFRAMETIME_OFFSET =
    static_cast<uintptr_t>(0x377a10);

const uintptr_t SND_G_VOL = static_cast<uintptr_t>(0x8488f0);
const uintptr_t SND_G_P = static_cast<uintptr_t>(0x848910);
const uintptr_t SND_G_LINEAR_COUNT = static_cast<uintptr_t>(0x848900);
}; // namespace offsets
