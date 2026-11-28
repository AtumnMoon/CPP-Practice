#ifndef ATM_SIMULATOR_DATA_ACCOUNT_H
#define ATM_SIMULATOR_DATA_ACCOUNT_H

#include <cstdint>
#include <string>

#include "money.hpp"

struct Account {
  std::uint32_t id{};
  Money balance{};
  std::string name;
  std::string password;
};

#endif
