#pragma once

#include <string>

namespace plotly::detail {

/// Generates a random UUID string in the format
/// xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
/// @return A randomly generated UUID string
auto generateUUID() -> std::string;

} // namespace plotly::detail
