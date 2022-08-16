#pragma once
#include <map>

class RecvProp;
class ClientClassManager {
 public:
  ClientClassManager();

 private:
  std::map<const std::string, RecvProp*> datatables;
};
