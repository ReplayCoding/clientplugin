#pragma once
#include <absl/container/btree_map.h>
#include <convar.h>
#include <dt_common.h>
#include <dt_recv.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

#include "sdk/concommandwrapper.hpp"
#include "util/generator.hpp"

// namespace to avoid some naming conflicts
namespace clientclasses {
  struct ClientProp {
    ClientProp() = default;
    ClientProp(std::ptrdiff_t offset, SendPropType type)
        : offset(offset), type(type){};

    std::ptrdiff_t offset{};
    SendPropType type{};
  };

  Generator<std::pair<std::string, clientclasses::ClientProp>> parse_tbl(
      RecvTable* tbl);
}  // namespace clientclasses

class ClientClassManager {
 public:
  ClientClassManager();
  std::ptrdiff_t get_prop_offset(std::string classname, std::string propname) {
    return clientclasses.at(std::pair(classname, propname)).offset;
  }

 private:
  void dump_props_to_file(const CCommand& cmd);
  // class name, prop name = prop
  absl::btree_map<std::pair<std::string, std::string>,
                  clientclasses::ClientProp>
      clientclasses;

  ConCommandCallbacks pe_dump_props_to_file_callback;
  ConCommand pe_dump_props_to_file;
};

extern std::unique_ptr<ClientClassManager> g_ClientClasses;
