#ifndef ATM_SIMULATOR_DATA_TRANSACTION_H
#define ATM_SIMULATOR_DATA_TRANSACTION_H

#include <cstdint>
#include <string>
#include <string_view>

#include "money.hpp"

enum class ActionType : std::uint8_t {
  kDeposit,
  kWithdraw,
  kBalanceInquiry,
};

struct Transaction {
  std::uint32_t id{};
  ActionType type{};
  Money amount{};
  Money balance_after{};
  std::string timestamp;
};

constexpr auto ToString(ActionType action) -> std::string_view {
  switch (action) {
    case ActionType::kDeposit:
      return "DEPOSIT";
    case ActionType::kWithdraw:
      return "WITHDRAW";
    case ActionType::kBalanceInquiry:
      return "BALANCE INQUIRY";
  }
  return "UNKNOWN";
}

#endif
