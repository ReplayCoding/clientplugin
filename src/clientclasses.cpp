#include <cdll_int.h>
#include <client_class.h>
#include <dt_common.h>
#include <fmt/core.h>
#include <unistd.h>
#include <cstring>

#include "clientclasses.hpp"
#include "interfaces.hpp"

ClientClassManager::ClientClassManager() {
  for (auto client_class = Interfaces::ClientDll->GetAllClasses(); client_class;
       client_class = client_class->m_pNext) {
    auto recv_tbl = client_class->m_pRecvTable;
    for (auto prop_idx = 0; prop_idx < recv_tbl->m_nProps; prop_idx++) {
      auto prop = recv_tbl->GetProp(prop_idx);
      if (prop->m_RecvType == DPT_DataTable &&
          std::strncmp(prop->m_pVarName, "baseclass", sizeof("baseclass")) !=
              0) {
        fmt::print("lol: {}->{}\n", recv_tbl->m_pNetTableName,
                   prop->m_pVarName);
        sleep(3);
      };
      datatables[prop->GetName()] = prop;
    };
  };
}
