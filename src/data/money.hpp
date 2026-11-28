#ifndef ATM_SIMULATOR_DATA_MONEY_H
#define ATM_SIMULATOR_DATA_MONEY_H

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

using Money = std::uint64_t;

auto ToString(Money amount) -> std::string;

enum class ParseMoneyError : std::uint8_t {
  kEmptyInput,
  kNegativeOrZero,
  kInvalidFormat,
  kTooManyDecimals,
  kOverflow,
};

auto ParseMoney(std::string_view input) -> std::expected<Money, ParseMoneyError>;

#endif
