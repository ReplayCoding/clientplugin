#include <cdll_int.h>
#include <client_class.h>
#include <convar.h>
#include <dt_common.h>
#include <dt_recv.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fstream>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/chunk_by.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/transform.hpp>
#include <tracy/Tracy.hpp>

#include "clientclasses.hpp"
#include "interfaces.hpp"
#include "util/generator.hpp"

Generator<std::pair<std::string, clientclasses::ClientProp>>
clientclasses::parse_tbl(RecvTable* tbl) {
  ZoneScoped;
  for (auto idx = 0; idx < tbl->GetNumProps(); idx++) {
    auto prop = tbl->GetProp(idx);

    if (prop->GetType() == SendPropType::DPT_DataTable) {
      auto subtable = prop->GetDataTable();
      auto parsed_subtable = parse_tbl(subtable);

      for (const auto& [name, subprop] : parsed_subtable) {
        const auto subprop_name =
            fmt::format("{}::{}", subtable->GetName(), name);

        co_yield std::pair(
            subprop_name,
            ClientProp(prop->GetOffset() + subprop.offset, subprop.type));
      };
    } else {
      co_yield std::pair(std::string(prop->GetName()),
                         ClientProp(prop->GetOffset(), prop->GetType()));
    }
  }
}

ClientClassManager::ClientClassManager()
    : pe_dump_props_to_file_callback([&](auto c) { dump_props_to_file(c); }),
      pe_dump_props_to_file("pe_dump_netvars_to_file",
                            &pe_dump_props_to_file_callback) {
  for (auto client_class = Interfaces::ClientDll->GetAllClasses();
       client_class != nullptr; client_class = client_class->m_pNext) {
    ZoneScoped;
    auto class_name = client_class->GetName();

    for (auto& [prop_name, prop] :
         clientclasses::parse_tbl(client_class->m_pRecvTable)) {
      clientclasses[std::pair(class_name, prop_name)] = prop;
    };
  }
}

void ClientClassManager::dump_props_to_file(const CCommand& cmd) {
  if (cmd.ArgC() != 2) {
    Warning(fmt::format("usage: {} <output file>\n", cmd[0]).c_str());
    return;
  }

  auto output_file = std::ofstream(cmd[1]);

  for (auto clientclass_rng :
       clientclasses | ranges::views::chunk_by([](auto&a, auto&b) {
         // Group by class name
         return a.first.first == b.first.first;
       })) {
    // A bit ugly...
    (void)clientclass_rng;
    // fmt::print(output_file, "CLIENTCLASS: {}\n", name);

    // for (auto prop : props) {
    //   // fmt::print(output_file, "\t{}: {:08X}\n", prop.name, prop.offset);
    // };
  }
}

std::unique_ptr<ClientClassManager> g_ClientClasses{};
