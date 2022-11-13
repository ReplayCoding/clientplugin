#include <cdll_int.h>
#include <client_class.h>
#include <convar.h>
#include <dt_common.h>
#include <dt_recv.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fstream>

#include "clientclasses.hpp"
#include "interfaces.hpp"

std::vector<clientclasses::ClientProp> clientclasses::ClientClass::parse_tbl(
    RecvTable* tbl) {
  std::vector<clientclasses::ClientProp> props;

  for (auto idx = 0; idx < tbl->GetNumProps(); idx++) {
    auto prop = tbl->GetProp(idx);

    if (prop->GetType() == SendPropType::DPT_DataTable) {
      auto subtable = prop->GetDataTable();
      auto parsed_subtable = parse_tbl(subtable);

      for (const auto& subprop : parsed_subtable) {
        const auto subprop_name =
            fmt::format("{}::{}", subtable->GetName(), subprop.name);
        const auto subprop_fixed = ClientProp(
            subprop_name, prop->GetOffset() + subprop.offset, subprop.type);

        props.emplace_back(subprop_fixed);
      };
    } else {
      const auto wrapped_prop = clientclasses::ClientProp(
          prop->GetName(), prop->GetOffset(), prop->GetType());

      props.emplace_back(wrapped_prop);
    }
  }
  return props;
}

ClientClassManager::ClientClassManager()
    : pe_dump_props_to_file_callback([&](auto c) { dump_props_to_file(c); }),
      pe_dump_props_to_file("pe_dump_netvars_to_file",
                            &pe_dump_props_to_file_callback) {
  for (auto client_class = Interfaces::ClientDll->GetAllClasses(); client_class;
       client_class = client_class->m_pNext) {
    auto recv_tbl = client_class->m_pRecvTable;
    auto clientclass_parsed = clientclasses::ClientClass(recv_tbl);

    clientclasses.emplace_back(clientclass_parsed);
  }
}

void ClientClassManager::dump_props_to_file(const CCommand& cmd) {
  if (cmd.ArgC() != 2) {
    Warning(fmt::format("usage: {} <output file>\n", cmd[0]).c_str());
    return;
  }

  auto output_file = std::ofstream(cmd[1]);

  for (auto clientclass : clientclasses) {
    fmt::print(output_file, "CLIENTCLASS: {}\n", clientclass.get_name());

    for (auto prop : clientclass.get_props()) {
      fmt::print(output_file, "\t{}: {:08X}\n", prop.name, prop.offset);
    };
  }
}
