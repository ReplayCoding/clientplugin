#pragma once
#include <convar.h>
#include <dt_common.h>
#include <dt_recv.h>
#include <cstddef>
#include <vector>

#include "sdk/concommandwrapper.hpp"

// namespace to avoid some naming conflicts
namespace clientclasses {
  struct ClientProp {
    ClientProp(std::string name, std::ptrdiff_t offset, SendPropType type)
        : name(name), offset(offset), type(type){};

    const std::string name;
    const std::ptrdiff_t offset;
    const SendPropType type;
  };

  class ClientClass {
   public:
    ClientClass(RecvTable* tbl) : name(tbl->GetName()), props(parse_tbl(tbl)) {}
    inline const auto get_name() { return name; }
    inline const auto get_props() { return props; }

   private:
    std::vector<clientclasses::ClientProp> parse_tbl(RecvTable* tbl);

    const std::string name;
    std::vector<clientclasses::ClientProp> props;
  };
}  // namespace clientclasses

class ClientClassManager {
 public:
  ClientClassManager();

 private:
  void dump_props_to_file(const CCommand& cmd);
  std::vector<clientclasses::ClientClass> clientclasses;

  ConCommandCallbacks pe_dump_props_to_file_callback;
  ConCommand pe_dump_props_to_file;
};
