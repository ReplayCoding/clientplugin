#pragma once
#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>
#include <bit>
#include <boost/algorithm/hex.hpp>
#include <cstdint>
#include <elfio/elfio.hpp>
#include <forward_list>
#include <iterator>
#include <string>

#include "offsets/rtti.hpp"
#include "util/data_range_checker.hpp"
#include "util/error.hpp"

using Vtables = absl::flat_hash_map<std::string_view, Vftables>;
using ModuleVtables = absl::node_hash_map<std::string, Vtables>;
using ModuleRangeMap = absl::flat_hash_map<std::string, DataRange>;
using EhFrameRanges = absl::node_hash_map<std::string, std::vector<DataRange>>;

void init_offsets();

class Offset;
extern std::forward_list<Offset*> g_offset_list;

class Offset {
  friend void init_offsets();

 public:
  Offset() { g_offset_list.push_front(this); };
  template <typename T>
  inline auto operator+(T rhs) const {
    return Offset(cached_address + rhs);
  }

  template <typename T>
  inline operator T() const {
    return std::bit_cast<T>(cached_address);
  }

 private:
  Offset(uintptr_t addr) : cached_address(addr){};

  virtual uintptr_t get_address(ModuleRangeMap& modules,
                                ModuleVtables& vtables,
                                EhFrameRanges& eh_frame) const = 0;
  virtual void cache_address(ModuleRangeMap& modules,
                             ModuleVtables& vtables,
                             EhFrameRanges& eh_frame) {
    cached_address = get_address(modules, vtables, eh_frame);
    if (cached_address == 0)
      throw StringError("Error while getting offset ({}), got nullptr",
                        typeid(*this).name());
  };

  uintptr_t cached_address{};
};

class ManualOffset : public Offset {
 public:
  ManualOffset(uintptr_t address) : Offset(), manual_address(address) {}

 private:
  virtual uintptr_t get_address(ModuleRangeMap& modules,
                                ModuleVtables& vtables,
                                EhFrameRanges& eh_frame) const override {
    return manual_address;
  };

  const uintptr_t manual_address{};
};

class SharedLibOffset : public Offset {
 public:
  SharedLibOffset(std::string&& module, uintptr_t offset)
      : Offset(), module(module), offset(offset){};

 private:
  uintptr_t get_address(ModuleRangeMap& modules,
                        ModuleVtables& vtables,
                        EhFrameRanges& eh_frame) const override;

  const std::string module;
  const uintptr_t offset;
};

class VtableOffset : public Offset {
 public:
  VtableOffset(std::string&& module,
               std::string&& name,
               uint32_t function,
               uint16_t vftable = 0)
      : Offset(),
        module(module),
        name(name),
        function(function),
        vftable(vftable){};

 private:
  uintptr_t get_address(ModuleRangeMap& modules,
                        ModuleVtables& vtables,
                        EhFrameRanges& eh_frame) const override;

  const std::string module;
  const std::string name;
  const uint16_t function;
  const uint16_t vftable;
};

class SharedLibSymbol : public Offset {
 public:
  SharedLibSymbol(std::string&& module, std::string&& symbol)
      : Offset(), module(module), symbol(symbol){};

 private:
  uintptr_t get_address(ModuleRangeMap& modules,
                        ModuleVtables& vtables,
                        EhFrameRanges& eh_frame) const override;

  const std::string module;
  const std::string symbol;
};

struct Signature {
  std::vector<uint8_t> bytes;
  std::vector<uint8_t> mask;
};

class SharedLibSignature : public Offset {
 public:
  SharedLibSignature(std::string&& module,
                     std::string&& signature_str,
                     std::string&& mask_str)
      : Offset(), module_name(module) {
    boost::algorithm::unhex(signature_str.begin(), signature_str.end(),
                            std::back_inserter(signature));
    boost::algorithm::unhex(mask_str.begin(), mask_str.end(),
                            std::back_inserter(mask));

    if (signature.size() != mask.size())
      throw StringError("signature and mask must be same size");
    if (signature.size() == 0)
      throw StringError("signature must be nonzero size");
  }

 private:
  uintptr_t get_address(ModuleRangeMap& modules,
                        ModuleVtables& vtables,
                        EhFrameRanges& eh_frame) const override;

  const std::string module_name;
  std::vector<uint8_t> signature;
  std::vector<uint8_t> mask;
};

namespace offsets {
  const extern SharedLibSymbol SDL_GL_SwapWindow;

  const extern SharedLibSignature FindAndHealTargets;
  const extern SharedLibSignature CNavMesh_GetNavDataFromFile;

  const extern SharedLibSymbol g_Telemetry;
  const extern SharedLibSymbol TelemetryTick;
  const extern SharedLibSymbol ThreadSetDebugName;
  const extern SharedLibSymbol CVProfNode_EnterScope;
  const extern SharedLibSymbol CVProfNode_ExitScope;
  const extern VtableOffset CEngine_Frame;

  const extern VtableOffset CTFCleaver_CreateJarProjectile;
  const extern VtableOffset CGameServer_FinishCertificateCheck;
}  // namespace offsets
