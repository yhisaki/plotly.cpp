#include "uuid.hpp"
#include <random>
#include <string>

namespace plotly::detail {

auto generateUUID() -> std::string {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 15);

  std::string uuid;
  const char *chars = "0123456789abcdef";

  for (int i = 0; i < 32; ++i) {
    if (i == 8 || i == 12 || i == 16 || i == 20)
      uuid += "-";
    uuid += chars[dis(gen)];
  }
  return uuid;
}

} // namespace plotly::detail
