#ifndef PLOTLY_LOGGER_HPP
#define PLOTLY_LOGGER_HPP

#include <cstdint>
#include <string_view>

namespace plotly {

enum class LogLevel : std::uint8_t {
  TRACE,
  DEBUG,
  INFO,
  WARN,
  ERROR,
};

void setLogLevel(LogLevel level);

// argument is a format string and a variable number of arguments
void logTrace(std::string_view fmt, ...);
void logDebug(std::string_view fmt, ...);
void logInfo(std::string_view fmt, ...);
void logWarn(std::string_view fmt, ...);
void logError(std::string_view fmt, ...);

} // namespace plotly

#endif // PLOTLY_LOGGER_HPP
