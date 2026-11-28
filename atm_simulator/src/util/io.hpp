#ifndef ATM_SIMULATOR_UTIL_IO_H
#define ATM_SIMULATOR_UTIL_IO_H

#include <cstdint>
#include <expected>
#include <string>

enum class InputError : std::uint8_t {
  kStreamFailed,
};

auto GetUserInt() -> std::expected<int, InputError>;
auto GetUserString() -> std::expected<std::string, InputError>;

#endif
