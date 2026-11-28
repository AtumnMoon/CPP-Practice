#include <charconv>
#include <expected>
#include <format>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>

#include "money.hpp"

namespace {

auto ParseDigits(std::string_view str)
    -> std::expected<Money, ParseMoneyError> {
  if (str.empty()) {
    return 0;
  }

  Money value = 0;
  const char* begin = std::to_address(str.begin());
  const char* end = std::to_address(str.end());
  auto [ptr, ec] = std::from_chars(begin, end, value);

  if (ec == std::errc::result_out_of_range) {
    return std::unexpected(ParseMoneyError::kOverflow);
  }
  if (ec != std::errc{} || ptr != end) {
    return std::unexpected(ParseMoneyError::kInvalidFormat);
  }
  return value;
}

}  // namespace

auto ToString(Money amount) -> std::string {
  constexpr int kHundred = 100;
  Money whole = amount / kHundred;
  Money decimal = amount % kHundred;
  return std::format("{}.{:02}", whole, decimal);
}

auto ParseMoney(std::string_view input)
    -> std::expected<Money, ParseMoneyError> {
  if (input.empty()) {
    return std::unexpected(ParseMoneyError::kEmptyInput);
  }
  if (input.front() == '-') {
    return std::unexpected(ParseMoneyError::kNegativeOrZero);
  }

  const auto dot_position = input.find('.');
  const std::string_view whole_str = input.substr(0, dot_position);
  const std::string_view decimal_str = (dot_position == std::string_view::npos)
                                           ? std::string_view{}
                                           : input.substr(dot_position + 1);

  if (whole_str.empty() && decimal_str.empty()) {
    return std::unexpected(ParseMoneyError::kInvalidFormat);
  }
  if (decimal_str.size() > 2) {
    return std::unexpected(ParseMoneyError::kTooManyDecimals);
  }

  auto whole = ParseDigits(whole_str);
  if (!whole) {
    return std::unexpected(whole.error());
  }

  auto decimal = ParseDigits(decimal_str);
  if (!decimal) {
    return std::unexpected(decimal.error());
  }

  // Multiple it if decimal is ".5" to 50 instead of just 5
  if (decimal_str.size() == 1) {
    const int tenths = 10;
    *decimal *= tenths;
  }

  // Multiply the money by hundred since Money starts from cents
  constexpr Money kHundred = 100;
  if (*whole > (std::numeric_limits<Money>::max() - *decimal) / kHundred) {
    return std::unexpected(ParseMoneyError::kOverflow);
  }

  const Money total = (*whole * kHundred) + *decimal;
  if (total == 0) {
    return std::unexpected(ParseMoneyError::kNegativeOrZero);
  }

  return total;
}
