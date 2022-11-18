#pragma once
#include <cstdint>
#include <string>

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

  const std::uintptr_t manual_address{};
};

using Offset = ManualOffset;

class SharedLibOffset : public ManualOffset {
 public:
  SharedLibOffset(std::string module, uintptr_t offset)
      : module(module), offset(offset){};

 private:
  std::uintptr_t get_address() const override;

  const std::string module;
  const std::uintptr_t offset;
};

class VtableOffset : public ManualOffset {
 public:
  VtableOffset(std::string module,
               std::string name,
               uint32_t function,
               uint16_t vftable = 0)
      : module(module), name(name), function(function), vftable(vftable){};

 private:
  std::uintptr_t get_address() const override;

  const std::string module;
  const std::string name;
  const uint16_t function;
  const uint16_t vftable;
};

class SharedLibSymbol : public ManualOffset {
 public:
  SharedLibSymbol(std::string module, std::string symbol)
      : module(module), symbol(symbol){};

 private:
  std::uintptr_t get_address() const override;

  const std::string module;
  const std::string symbol;
};

namespace offsets {
  const extern SharedLibOffset SCR_UpdateScreen;
  const extern SharedLibOffset SND_RecordBuffer;

  const extern SharedLibOffset GetSoundTime;

  const extern VtableOffset CEngineSoundServices_SetSoundFrametime;

  const extern SharedLibOffset SND_G_P;
  const extern SharedLibOffset SND_G_LINEAR_COUNT;
  const extern SharedLibOffset SND_G_VOL;

  const extern SharedLibOffset FindAndHealTargets;

  const extern SharedLibSymbol CVProfNode_EnterScope;
  const extern SharedLibSymbol CVProfNode_ExitScope;
}  // namespace offsets
