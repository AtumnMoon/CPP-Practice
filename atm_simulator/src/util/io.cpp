#include <expected>
#include <iostream>
#include <limits>
#include <string>

#include "io.hpp"

auto GetUserInt() -> std::expected<int, InputError> {
  int input = 0;

  while (true) {
    if (!(std::cin >> input)) {
      if (std::cin.eof() || std::cin.bad()) {
        return std::unexpected(InputError::kStreamFailed);
      }

      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      continue;  // Retry
    }

    return input;
  }
}

auto GetUserString() -> std::expected<std::string, InputError> {
  std::string input;

  if (!std::getline(std::cin, input)) {
    return std::unexpected(InputError::kStreamFailed);
  }

  return input;
}
