#pragma once
#include <cstdint>
#include <string>

namespace offsets {

  // UGLY HACK
  class ManualOffset {
   public:
    ManualOffset() = default;
    ManualOffset(std::uintptr_t address) : manual_address(address) {}

    template <typename T>
    inline ManualOffset operator+(T rhs) const {
      return ManualOffset(get_address() + rhs);
    }

    template <typename T>
    inline operator T() const {
      return reinterpret_cast<T>(get_address());
    }

   private:
    virtual std::uintptr_t get_address() const { return manual_address; };

    std::uintptr_t manual_address{};
  };
  // UGH
  using Offset = ManualOffset;

  class SharedLibOffset : public ManualOffset {
   public:
    SharedLibOffset(std::string module, uintptr_t offset)
        : module(module), offset(offset){};

   private:
    std::uintptr_t get_address() const override;

    std::string module;
    std::uintptr_t offset;
  };

  const extern SharedLibOffset FIREGAMEEVENT_OFFSET;

  const extern SharedLibOffset SCR_UPDATESCREEN_OFFSET;
  const extern SharedLibOffset SND_RECORDBUFFER_OFFSET;

  const extern SharedLibOffset GETSOUNDTIME_OFFSET;

  // This is in a vtable so we should probably fix that
  const extern SharedLibOffset CENGINESOUNDSERVICES_SETSOUNDFRAMETIME_OFFSET;

  const extern SharedLibOffset SND_G_P;
  const extern SharedLibOffset SND_G_LINEAR_COUNT;
  const extern SharedLibOffset SND_G_VOL;
}  // namespace offsets
