#include "plotly/logger.hpp"
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>

namespace plotly {

namespace {
// ANSI color codes
const char *reset = "\033[0m";
const char *red = "\033[31m";
const char *green = "\033[32m";
const char *yellow = "\033[33m";
const char *blue = "\033[34m";
const char *magenta = "\033[35m";

class Logger {
public:
  static auto instance() -> Logger & {
    static Logger logger;
    return logger;
  }

  void setLevel(LogLevel level) { _currentLevel = level; }

  void log(LogLevel level, const std::string &message) {
    if (level < _currentLevel) {
      return;
    }

    std::scoped_lock lock(_mutex);

    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) %
              1000;

    std::stringstream ss;
    ss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
       << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
    ss << "[plotly] ";

    // Add color based on level
    switch (level) {
    case LogLevel::TRACE:
      ss << magenta << "[TRACE]" << reset;
      break;
    case LogLevel::DEBUG:
      ss << blue << "[DEBUG]" << reset;
      break;
    case LogLevel::INFO:
      ss << green << "[INFO]" << reset;
      break;
    case LogLevel::WARN:
      ss << yellow << "[WARN]" << reset;
      break;
    case LogLevel::ERROR:
      ss << red << "[ERROR]" << reset;
      break;
    }

    ss << " " << message << '\n';
    std::cout << ss.str();
  }

private:
  Logger() = default;

  LogLevel _currentLevel{LogLevel::INFO};
  std::mutex _mutex;
};

} // namespace

// Helper function to format string with variadic arguments
auto formatString(const std::string_view fmt, va_list args) -> std::string {
  // Convert string_view to string to ensure null termination
  std::string fmtStr(fmt);

  // Calculate buffer size needed
  va_list argsCopy;
  va_copy(argsCopy, args);
  const int size = vsnprintf(nullptr, 0, fmtStr.c_str(), argsCopy) + 1;
  va_end(argsCopy);

  // Format the string
  std::string result(size, '\0');
  vsnprintf(result.data(), size, fmtStr.c_str(), args);

  // Remove the null terminator that vsnprintf adds
  result.resize(size - 1);
  return result;
}

void setLogLevel(LogLevel level) { Logger::instance().setLevel(level); }

void logTrace(const std::string_view fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::string formatted = formatString(fmt, args);
  va_end(args);

  Logger::instance().log(LogLevel::TRACE, formatted);
}

void logDebug(const std::string_view fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::string formatted = formatString(fmt, args);
  va_end(args);

  Logger::instance().log(LogLevel::DEBUG, formatted);
}

void logInfo(const std::string_view fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::string formatted = formatString(fmt, args);
  va_end(args);

  Logger::instance().log(LogLevel::INFO, formatted);
}

void logWarn(const std::string_view fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::string formatted = formatString(fmt, args);
  va_end(args);

  Logger::instance().log(LogLevel::WARN, formatted);
}

void logError(const std::string_view fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::string formatted = formatString(fmt, args);
  va_end(args);

  Logger::instance().log(LogLevel::ERROR, formatted);
}

} // namespace plotly
