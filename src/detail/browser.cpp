#include "browser.hpp"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "plotly/logger.hpp"
#include "websockets_client.hpp"
#include <arpa/inet.h>
#include <array>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <functional>
#include <future>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

namespace plotly::detail {

auto isDisplayAvailable() -> bool {
  // Check if DISPLAY environment variable is set
  const char *displayEnv = std::getenv("DISPLAY");
  if (displayEnv == nullptr) {
    return false;
  }
  return true;
}

auto isChromiumAvailable() -> bool {
  // Check if chromium is installed
  return system("which chromium > /dev/null 2>&1") == 0;
}

auto isGoogleChromeAvailable() -> bool {
  // Check if google-chrome is installed
  return system("which google-chrome > /dev/null 2>&1") == 0;
}

auto openBrowser(const std::string_view url) -> bool {
  pid_t pid = fork();

  if (pid < 0) {
    plotly::logError("Error: fork failed when trying to open browser");
    return false;
  }
  if (pid == 0) {
    // In the child process
    // Detach from parent process group
    setsid();

    // Redirect stdout and stderr to /dev/null
    int devNull = open("/dev/null", O_RDWR);
    if (devNull != -1) {
      dup2(devNull, STDOUT_FILENO);
      dup2(devNull, STDERR_FILENO);
      dup2(devNull, STDIN_FILENO);
      if (devNull > STDERR_FILENO) {
        close(devNull);
      }
    }

    // Try xdg-open (Linux standard)
    execlp("xdg-open", "xdg-open", std::string(url).c_str(), nullptr);

    // If all browsers fail, exit the child process
    _exit(1);
  } else {
    // Check if child process started successfully by waiting briefly
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);

    if (result == pid && WIFEXITED(status)) {
      // Child process exited immediately, xdg-open failed
      plotly::logError("Failed to open browser - xdg-open not available");
      return false;
    }

    return true;
  }
}

auto openChromiumWithHeadlessMode(const std::string_view url,
                                  int remoteDebuggingPort)
    -> std::pair<bool, std::function<void()>> {
  pid_t pid = fork();

  if (pid < 0) {
    plotly::logError(
        "Error: fork failed when trying to open chromium in headless mode");
    return {false, []() -> void {}}; // Return failure with empty function
  }
  if (pid == 0) {

    // Redirect stdout and stderr to /dev/null
    int devNull = open("/dev/null", O_RDWR);
    if (devNull != -1) {
      dup2(devNull, STDOUT_FILENO);
      dup2(devNull, STDERR_FILENO);
      dup2(devNull, STDIN_FILENO);
      if (devNull > STDERR_FILENO) {
        close(devNull);
      }
    }
    // Try chromium with headless mode
    plotly::logTrace("Trying to open with chromium...");
    std::string portArg =
        "--remote-debugging-port=" + std::to_string(remoteDebuggingPort);
    execlp("chromium", "chromium", "--headless", "--disable-gpu",
           "--no-sandbox", "--disable-dev-shm-usage", "--disable-extensions",
           "--enable-features=NetworkService,NetworkServiceInProcess",
           portArg.c_str(), std::string(url).c_str(), nullptr);

    plotly::logTrace("Chromium not found, trying google-chrome...");
    execlp("google-chrome", "google-chrome", "--headless", "--disable-gpu",
           "--no-sandbox", "--disable-dev-shm-usage", "--disable-extensions",
           "--enable-features=NetworkService,NetworkServiceInProcess",
           portArg.c_str(), std::string(url).c_str(), nullptr);

    plotly::logTrace("Google Chrome not found, trying chromium-browser...");
    execlp("chromium-browser", "chromium-browser", "--headless",
           "--disable-gpu", "--no-sandbox", "--disable-dev-shm-usage",
           "--disable-extensions",
           "--enable-features=NetworkService,NetworkServiceInProcess",
           portArg.c_str(), std::string(url).c_str(), nullptr);

    // If all browsers fail, exit the child process
    _exit(1);
  } else {
    // Check if child process started successfully by waiting briefly
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);

    if (result == pid && WIFEXITED(status)) {
      // Child process exited immediately, all browsers failed
      plotly::logError(
          "Failed to open chromium in headless mode - no browsers available");
      return {false, []() -> void {}}; // Return failure with empty function
    }

    plotly::logDebug(
        "Chromium in headless mode opened successfully in child process "
        "(pid: %d)",
        pid);

    // Return success with a function that kills the chromium process
    return {true, [pid]() -> void {
              plotly::logDebug("Killing chromium in headless mode (pid: %d)",
                               pid);
              kill(pid, SIGKILL);
              waitpid(pid, nullptr, 0);
            }};
  }
}

auto getIpv4Addresses() -> std::vector<std::string> {
  std::vector<std::string> result;
  struct ifaddrs *ifaddr = nullptr;

  if (getifaddrs(&ifaddr) == -1) {
    return result; // failed to get interfaces
  }

  for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr) {
      continue;
    }

    if (ifa->ifa_addr->sa_family == AF_INET) { // IPv4 only
      std::array<char, INET_ADDRSTRLEN> ip{};
      auto *sa = reinterpret_cast<struct sockaddr_in *>(ifa->ifa_addr);
      if (inet_ntop(AF_INET, &(sa->sin_addr), ip.data(), INET_ADDRSTRLEN) !=
          nullptr) {
        result.emplace_back(ip.data());
      }
    }
  }

  freeifaddrs(ifaddr);
  return result;
}

auto setDownloadDirectory(const std::filesystem::path &directory,
                          int remoteDebuggingPort) -> bool {
  httplib::Client client("localhost", remoteDebuggingPort);
  plotly::logTrace("Setting download directory to %s",
                   std::filesystem::absolute(directory).c_str());

  // First get the browser info from /json/version
  if (auto response = client.Get("/json")) {
    if (response->status != 200) {
      plotly::logError("Failed to get version from browser");
      return false;
    }

    nlohmann::json responseJson = nlohmann::json::parse(response->body);

    std::string websocketDebuggerUrl =
        responseJson.back()["webSocketDebuggerUrl"];

    WebsocketClient client;
    client.connect(websocketDebuggerUrl);
    client.waitConnection(std::chrono::seconds(1));

    // Then send the Page.setDownloadBehavior command
    nlohmann::json params = {
        {"behavior", "allow"},
        {"downloadPath", std::filesystem::absolute(directory).string()}};
    nlohmann::json command = {
        {"id", 1}, {"method", "Page.setDownloadBehavior"}, {"params", params}};

    std::promise<std::string> promise;

    client.registerCallback(
        "response_handler",
        [&promise](const std::string_view &message) -> void {
          std::string messageStr(message);
          plotly::logTrace("Page.setDownloadBehavior: %s", messageStr.c_str());
          promise.set_value(std::string(message));
        });
    client.send(command.dump());

    std::string result = promise.get_future().get();
    plotly::logTrace("Result: %s", result.c_str());
    return true;
  }
  plotly::logError("Failed to get response from browser");
  return false;
}

auto getDefaultDownloadDirectory() -> std::filesystem::path {
  std::array<char, 128> buffer{};
  std::string result;
  FILE *pipe = popen("xdg-user-dir DOWNLOAD", "r");
  if (pipe == nullptr) {
    // Fallback to ~/Downloads if xdg-user-dir fails
    const char *home = std::getenv("HOME");
    if (home != nullptr) {
      return {std::filesystem::path(home) / "Downloads"};
    }
    return {"/tmp"};
  }

  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
    result += buffer.data();
  }
  pclose(pipe);

  // Remove trailing newline
  if (!result.empty() && result.back() == '\n') {
    result.pop_back();
  }

  // If result is empty, fallback to ~/Downloads
  if (result.empty()) {
    const char *home = std::getenv("HOME");
    if (home != nullptr) {
      return {std::filesystem::path(home) / "Downloads"};
    }
    return {"/tmp"};
  }

  return {result};
}

} // namespace plotly::detail
