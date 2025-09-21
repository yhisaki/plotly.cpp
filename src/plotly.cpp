#include "plotly/plotly.hpp"
#include "detail/browser.hpp"
#include "detail/http_server.hpp"
#include "detail/json_rpc.hpp"
#include "detail/uuid.hpp"
#include "detail/websockets_server.hpp"
#include "plotly/logger.hpp"

#include <chrono>
#include <exception>
#include <filesystem>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace plotly {

// Constants to replace magic numbers
namespace {

using namespace std::chrono_literals;
constexpr int AUTO_SELECT_PORT = 0;
constexpr int CHROME_DEVTOOLS_PORT = 9222;
constexpr int BROWSER_STARTUP_DELAY_SECONDS = 1;
constexpr auto WEBSOCKET_CONNECTION_TIMEOUT_SECONDS = 3000ms;
constexpr auto RPC_CALL_TIMEOUT_SECONDS = 200ms;
constexpr std::string_view WEBSOCKET_BIND_ADDRESS = "0.0.0.0";

} // namespace

class Figure::Impl {
public:
  std::unique_ptr<detail::JsonRpc> jsonRpc;
  std::unique_ptr<detail::HttpServer> httpServer;
  bool isHeadless = false;
  std::optional<std::filesystem::path> downloadDirectoryOfHeadlessMode;
  std::optional<std::function<void()>> closeChromiumFunction;
  mutable bool isOnceConnected = false;

  // Event management members
  std::unordered_map<std::string, std::function<void(plotly::Object)>>
      eventCallbacks;
  std::unordered_map<std::string, std::vector<std::string>> eventNameToIds;

  Impl(const std::filesystem::path &webappPath) {
    httpServer = std::make_unique<detail::HttpServer>(webappPath);
    auto websocketsServer = std::make_unique<detail::WebsocketServer>();
    websocketsServer->serve(WEBSOCKET_BIND_ADDRESS, AUTO_SELECT_PORT);
    httpServer->setWebsocketPortRequestHandler(websocketsServer->getPort());
    jsonRpc = std::make_unique<detail::JsonRpc>(std::move(websocketsServer));
    httpServer->start();

    auto ips = detail::getIpv4Addresses();

    std::cout << "📊 Plotly figure created at "
              << "http://localhost:" << httpServer->getPort() << " (";
    bool first = true;
    for (const auto &ip : ips) {
      if (!first)
        std::cout << " ";
      std::cout << "http://" << ip << ":" << httpServer->getPort();
      first = false;
    }
    std::cout << ")\n" << std::flush;
  }

  ~Impl() {
    if (closeChromiumFunction) {
      closeChromiumFunction.value()();
    }
  }

  void waitConnection() const {
    if (isOnceConnected) {
      return;
    }
    isOnceConnected = true;
    while (!jsonRpc->getWebsocketEndpoint()->waitConnection(
        WEBSOCKET_CONNECTION_TIMEOUT_SECONDS)) {
      plotly::logWarn("[Figure] Waiting for open browser");
    }
  }

  auto openBrowser(bool headless = false) -> bool {
    bool isDisplayAvailable = detail::isDisplayAvailable();
    auto url = "http://localhost:" + std::to_string(httpServer->getPort());
    if (!headless && isDisplayAvailable) {
      bool success = detail::openBrowser(url);
      if (!success) {
        plotly::logError("[Figure] Failed to open browser");
        return false;
      }
    } else if (headless) {
      auto [success, closeChromiumFunction] =
          detail::openChromiumWithHeadlessMode(url, CHROME_DEVTOOLS_PORT);
      if (!success) {
        plotly::logError("[Figure] Failed to open chromium with headless mode");
        return false;
      }
      this->closeChromiumFunction = closeChromiumFunction;
      auto defaultDownloadDirectory = detail::getDefaultDownloadDirectory();
      // obtain home directory
      std::this_thread::sleep_for(
          std::chrono::seconds(BROWSER_STARTUP_DELAY_SECONDS));
      success = detail::setDownloadDirectory(defaultDownloadDirectory,
                                             CHROME_DEVTOOLS_PORT);
      if (!success) {
        plotly::logError("[Figure] Failed to set download directory");
        return false;
      }
      downloadDirectoryOfHeadlessMode = defaultDownloadDirectory;
    } else {
      plotly::logError(
          "[Figure] Display is not available, skipping browser opening");
      return false;
    }
    isHeadless = headless;
    waitConnection();
    return true;
  }

  auto callPlotly(const std::string &method, const Object &params) const
      -> std::optional<plotly::Object> {
    waitConnection();
    auto [future, cancel] = jsonRpc->call(method, params);

    // Wait for 1 second for the call
    if (future.wait_for(std::chrono::duration<double>(
            RPC_CALL_TIMEOUT_SECONDS)) == std::future_status::ready) {
      return future.get();
    }

    // Call timed out, cancel it and retry
    cancel();
    return std::nullopt;
  }

  auto isOpen() const -> bool {
    return dynamic_cast<detail::WebsocketServer *>(
               jsonRpc->getWebsocketEndpoint())
        ->hasClient();
  }

  void waitClose() const {
    dynamic_cast<detail::WebsocketServer *>(jsonRpc->getWebsocketEndpoint())
        ->waitUntilNoClient();
  }

  auto newPlot(const Object &data, const Object &layout,
               const Object &config) const -> bool {
    auto result =
        callPlotly("Plotly.newPlot",
                   {{"data", data}, {"layout", layout}, {"config", config}});
    return result.has_value();
  }

  /// Updates the figure with new trace and layout data
  /// @param traceUpdate Updates to apply to trace data
  /// @param layoutUpdate Updates to apply to layout configuration
  auto update(const Object &traceUpdate, const Object &layoutUpdate) const
      -> bool {
    Object params;
    params["traceUpdate"] = traceUpdate;
    params["layoutUpdate"] = layoutUpdate;

    auto result = callPlotly("Plotly.update", params);
    return result.has_value();
  }

  auto downloadImage(const Object &opts) const -> bool {
    auto result = callPlotly("Plotly.downloadImage", {{"opts", opts}});
    if (!result.has_value()) {
      return false;
    }

    if (isHeadless && downloadDirectoryOfHeadlessMode.has_value()) {
      std::string fileName;
      try {
        fileName = result->at("fileName").get<std::string>();
      } catch (const std::exception &e) {
        plotly::logError("Failed to get file name from response: %s", e.what());
        return false;
      }
      std::filesystem::path filePath =
          *downloadDirectoryOfHeadlessMode / fileName;
      // Poll for file existence every 0.1 seconds, timeout after 5 seconds
      const auto startTime = std::chrono::steady_clock::now();
      const auto timeoutDuration = std::chrono::seconds(5);
      const auto pollInterval = std::chrono::milliseconds(100);

      bool fileExists = false;
      while (std::chrono::steady_clock::now() - startTime < timeoutDuration) {
        if (std::filesystem::exists(filePath)) {
          fileExists = true;
          break;
        }
        std::this_thread::sleep_for(pollInterval);
      }

      if (!fileExists) {
        plotly::logError("File download timeout: %s",
                         filePath.string().c_str());
        return false;
      }

      plotly::logDebug("File successfully downloaded: %s",
                       filePath.string().c_str());
    }
    return true;
  }

  /// Updates the figure layout
  /// @param layout Layout updates to apply
  auto relayout(const Object &layout) const -> bool {
    auto result = callPlotly("Plotly.relayout", {{"layout", layout}});
    return result.has_value();
  }

  /// Redraws the plot
  auto redraw() const -> bool {
    auto result = callPlotly("Plotly.redraw", Object());
    return result.has_value();
  }

  /// Purges the plot, removing all data and layout
  auto purge() const -> bool {
    auto result = callPlotly("Plotly.purge", Object());
    return result.has_value();
  }

  /// Restyles existing traces
  /// @param aobj Style updates to apply
  /// @param traces Optional trace indices to target
  auto restyle(const Object &aobj, const Object &traces) const -> bool {
    auto result =
        callPlotly("Plotly.restyle", {{"aobj", aobj}, {"traces", traces}});
    return result.has_value();
  }

  /// Adds new traces to the plot
  /// @param traces New trace data to add
  /// @param newIndices Optional indices where to insert the traces
  auto addTraces(const Object &traces, const Object &newIndices) const -> bool {
    auto result = callPlotly("Plotly.addTraces",
                             {{"traces", traces}, {"newIndices", newIndices}});
    return result.has_value();
  }

  /// Deletes traces from the plot
  /// @param indices Indices of traces to delete
  auto deleteTraces(const Object &indices) const -> bool {
    auto result = callPlotly("Plotly.deleteTraces", {{"indices", indices}});
    return result.has_value();
  }

  /// Moves traces to new positions
  /// @param currentIndices Current indices of traces to move
  /// @param newIndices New indices for the traces
  auto moveTraces(const Object &currentIndices, const Object &newIndices) const
      -> bool {
    auto result =
        callPlotly("Plotly.moveTraces", {{"currentIndices", currentIndices},
                                         {"newIndices", newIndices}});
    return result.has_value();
  }

  /// Extends existing traces with new data
  /// @param update Data to append to traces
  /// @param indices Trace indices to extend
  /// @param maxPoints Optional maximum number of points to keep
  auto extendTraces(const Object &update, const Object &indices,
                    const Object &maxPoints) const -> bool {
    auto result = callPlotly(
        "Plotly.extendTraces",
        {{"update", update}, {"indices", indices}, {"maxPoints", maxPoints}});
    return result.has_value();
  }

  /// Prepends data to existing traces
  /// @param update Data to prepend to traces
  /// @param indices Trace indices to prepend to
  auto prependTraces(const Object &update, const Object &indices) const
      -> bool {
    auto result = callPlotly("Plotly.prependTraces",
                             {{"update", update}, {"indices", indices}});
    return result.has_value();
  }

  /// React-style update of the plot
  /// @param data New data array
  /// @param layout Optional layout specification
  /// @param config Optional configuration
  auto react(const Object &data, const Object &layout,
             const Object &config) const -> bool {
    auto result =
        callPlotly("Plotly.react",
                   {{"data", data}, {"layout", layout}, {"config", config}});
    return result.has_value();
  }

  /// Adds animation frames to the plot
  /// @param frames Frame data to add
  auto addFrames(const Object &frames) const -> bool {
    auto result = callPlotly("Plotly.addFrames", {{"frames", frames}});
    return result.has_value();
  }

  /// Deletes animation frames from the plot
  /// @param frames Frame names or indices to delete
  auto deleteFrames(const Object &frames) const -> bool {
    auto result = callPlotly("Plotly.deleteFrames", {{"frames", frames}});
    return result.has_value();
  }

  /// Animates the plot
  /// @param frameOrGroupNameOrFrameList Frame specification for animation
  /// @param opts Animation options
  auto animate(const Object &frameOrGroupNameOrFrameList,
               const Object &opts) const -> bool {
    auto result = callPlotly("Plotly.animate", {{"frameOrGroupNameOrFrameList",
                                                 frameOrGroupNameOrFrameList},
                                                {"opts", opts}});
    return result.has_value();
  }

  /// Sets the download directory for the browser
  /// @param directory Path to the directory where downloads should be saved
  /// @param remoteDebuggingPort Port for Chrome DevTools remote debugging
  /// @return true if successful, false otherwise
  auto setDownloadDirectory(const std::filesystem::path &directory,
                            int remoteDebuggingPort = CHROME_DEVTOOLS_PORT)
      -> bool {
    if (!isHeadless) {
      plotly::logWarn(
          "setting download directory is only available in headless mode");
      return false;
    }
    downloadDirectoryOfHeadlessMode = directory;
    return detail::setDownloadDirectory(directory, remoteDebuggingPort);
  }
};

Figure::Figure(const std::filesystem::path &webappPath)
    : _pimpl(std::make_unique<Impl>(webappPath)) {}

Figure::~Figure() = default;

auto Figure::isOpen() const -> bool { return _pimpl->isOpen(); }

void Figure::waitClose() const { return _pimpl->waitClose(); }

auto Figure::newPlot(const Object &data, const Object &layout,
                     const Object &config) -> bool {
  return _pimpl->newPlot(data, layout, config);
}

auto Figure::downloadImage(const Object &opts) -> bool {
  return _pimpl->downloadImage(opts);
}

auto Figure::update(const Object &traceUpdate, const Object &layoutUpdate)
    -> bool {
  return _pimpl->update(traceUpdate, layoutUpdate);
}

auto Figure::relayout(const Object &layout) -> bool {
  return _pimpl->relayout(layout);
}

auto Figure::redraw() -> bool { return _pimpl->redraw(); }

auto Figure::purge() -> bool { return _pimpl->purge(); }

auto Figure::restyle(const Object &aobj, const Object &traces) -> bool {
  return _pimpl->restyle(aobj, traces);
}

auto Figure::addTraces(const Object &traces, const Object &newIndices) -> bool {
  return _pimpl->addTraces(traces, newIndices);
}

auto Figure::deleteTraces(const Object &indices) -> bool {
  return _pimpl->deleteTraces(indices);
}

auto Figure::moveTraces(const Object &currentIndices, const Object &newIndices)
    -> bool {
  return _pimpl->moveTraces(currentIndices, newIndices);
}

auto Figure::extendTraces(const Object &update, const Object &indices,
                          const Object &maxPoints) -> bool {
  return _pimpl->extendTraces(update, indices, maxPoints);
}

auto Figure::prependTraces(const Object &update, const Object &indices)
    -> bool {
  return _pimpl->prependTraces(update, indices);
}

auto Figure::react(const Object &data, const Object &layout,
                   const Object &config) -> bool {
  return _pimpl->react(data, layout, config);
}

auto Figure::addFrames(const Object &frames) -> bool {
  return _pimpl->addFrames(frames);
}

auto Figure::deleteFrames(const Object &frames) -> bool {
  return _pimpl->deleteFrames(frames);
}

auto Figure::animate(const Object &frameOrGroupNameOrFrameList,
                     const Object &opts) -> bool {
  return _pimpl->animate(frameOrGroupNameOrFrameList, opts);
}

auto Figure::on(const std::string &event,
                const std::function<void(plotly::Object)> &callback) -> bool {
  // Generate UUID for event ID
  std::string eventId = detail::generateUUID();

  // Store the callback
  _pimpl->eventCallbacks[eventId] = callback;

  // Add event ID to event name mapping
  _pimpl->eventNameToIds[event].push_back(eventId);

  // Register JSON-RPC notification handler
  _pimpl->jsonRpc->registerNotification(
      eventId, [this, eventId](const Object &eventData) -> void {
        auto it = _pimpl->eventCallbacks.find(eventId);
        if (it != _pimpl->eventCallbacks.end()) {
          it->second(eventData);
        }
      });

  // Notify frontend to register event listener
  try {
    auto result = _pimpl->callPlotly("Plotly.on",
                                     {{"event", event}, {"eventId", eventId}});
    if (result.has_value()) {
      plotly::logDebug("Event listener registered for: %s", event.c_str());
      return true;
    }
    return false;
  } catch (const std::exception &e) {
    plotly::logError("Failed to register event listener for %s: %s",
                     event.c_str(), e.what());
    return false;
  }
}

auto Figure::removeAllListeners(const std::string &event) -> bool {
  // Notify frontend to remove all listeners
  auto result =
      _pimpl->callPlotly("Plotly.removeAllListeners", {{"event", event}});

  // Remove all callbacks for this event
  auto it = _pimpl->eventNameToIds.find(event);
  if (it != _pimpl->eventNameToIds.end()) {
    for (const auto &eventId : it->second) {
      _pimpl->eventCallbacks.erase(eventId);
      _pimpl->jsonRpc->unregisterHandler(eventId);
    }
    _pimpl->eventNameToIds.erase(it);
  }

  return result.has_value();
}

auto Figure::openBrowser(bool headless) -> bool {
  return _pimpl->openBrowser(headless);
}

auto Figure::setDownloadDirectory(const std::filesystem::path &directory,
                                  int remoteDebuggingPort) -> bool {
  return _pimpl->setDownloadDirectory(directory, remoteDebuggingPort);
}

} // namespace plotly
