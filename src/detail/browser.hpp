#ifndef PLOTLY_DETAILS_SYSTEM_UTILS_HPP
#define PLOTLY_DETAILS_SYSTEM_UTILS_HPP

#include <filesystem>
#include <functional>
#include <string_view>
#include <vector>
namespace plotly::detail {

auto isDisplayAvailable() -> bool;

auto isChromiumAvailable() -> bool;

auto isGoogleChromeAvailable() -> bool;

auto openBrowser(std::string_view url) -> bool;

auto openChromiumWithHeadlessMode(std::string_view url,
                                  int remoteDebuggingPort = 9222)
    -> std::pair<bool, std::function<void()>>;

auto setDownloadDirectory(const std::filesystem::path &directory,
                          int remoteDebuggingPort = 9222) -> bool;

auto getIpv4Addresses() -> std::vector<std::string>;

auto getDefaultDownloadDirectory() -> std::filesystem::path;

} // namespace plotly::detail

#endif // PLOTLY_DETAILS_SYSTEM_UTILS_HPP
