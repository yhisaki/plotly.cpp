#include "http_server.hpp"
#include "httplib.h"
#include "plotly/logger.hpp"
#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>
#include <thread>

namespace plotly::detail {

void HttpServer::setupCommon() {
  _server.set_keep_alive_max_count(1);
  // Add endpoint to signal successful page load
  _server.Get(
      "/loaded",
      [](const httplib::Request & /*unused*/, httplib::Response &res) -> void {
        res.set_content(R"({"status":"ok"})", "application/json");
      });
}

HttpServer::HttpServer(const std::filesystem::path &directory) : _server() {
  plotly::logDebug("[HttpServer] Setting mount point to %s",
                   directory.string().c_str());
  _server.set_mount_point("/", directory.string());
  setupCommon();
}

HttpServer::HttpServer(const std::string_view htmlContent) : _server() {
  _server.Get(
      "/",
      [htmlContent](const httplib::Request &, httplib::Response &res) -> void {
        res.set_content(std::string(htmlContent), "text/html");
      });
  setupCommon();
}

HttpServer::~HttpServer() { stop(); }

void HttpServer::start() {
  if (_serverThread.joinable()) {
    return; // Already running
  }

  // Bind to any available port on all interfaces
  _port = _server.bind_to_any_port("0.0.0.0");
  plotly::logDebug("[HttpServer] Server started at http://0.0.0.0:%d", _port);

  // Run server loop in background thread
  _serverThread =
      std::thread([this]() -> void { _server.listen_after_bind(); });

  while (!_server.is_running()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void HttpServer::stop() {
  if (!_serverThread.joinable()) {
    return; // Not running
  }
  _server.stop();
  plotly::logTrace("[HttpServer] server.stop() called");
  _serverThread.join();
  plotly::logTrace("[HttpServer] Server stopped");
}

auto HttpServer::getPort() const noexcept -> int { return _port; }

void HttpServer::setWebsocketPortRequestHandler(int wsPort) {
  plotly::logDebug("Setting websocket port request handler to %d", wsPort);
  _server.Get(
      "/ws_port",
      [wsPort](const httplib::Request &, httplib::Response &res) -> void {
        plotly::logDebug("Received websocket port request");
        res.set_content("{\"port\":" + std::to_string(wsPort) + "}",
                        "application/json");
      });
}

} // namespace plotly::detail
