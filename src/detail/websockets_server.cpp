#include "websockets_server.hpp"
#include "detail/websockets_endpoint.hpp"
#include "plotly/logger.hpp"
#include "websocketpp/common/asio.hpp"
#include "websocketpp/common/connection_hdl.hpp"
#include "websocketpp/frame.hpp"
#include "websocketpp/logger/levels.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <mutex>
#include <string>
#include <string_view>

namespace plotly::detail {

WebsocketServer::WebsocketServer() {
  _portFuture = _portPromise.get_future().share();

  _server.set_access_channels(websocketpp::log::alevel::none);
  _server.clear_access_channels(websocketpp::log::alevel::none);
  _server.set_error_channels(websocketpp::log::elevel::none);
  _server.init_asio();
  _server.set_reuse_addr(true);

  // Set up handlers
  setupServerHandlers();
}

WebsocketServer::~WebsocketServer() { stop(); }

void WebsocketServer::serviceLoop() {
  try {
    _server.run();
  } catch (const std::exception &e) {
    plotly::logError("[WebsocketServer] Service loop error: %s", e.what());
  }
}

auto WebsocketServer::serve(const std::string_view address, int port) -> bool {
  try {
    // Configure server
    if (address.empty()) {
      _server.listen(port);
    } else {
      _server.listen(std::string(address), std::to_string(port));
    }

    _server.start_accept();

    // Get the port from the server
    websocketpp::lib::asio::error_code ec;
    auto localEndpoint = _server.get_local_endpoint(ec);
    if (ec) {
      plotly::logError("[WebsocketServer] Failed to get local endpoint: %s",
                       ec.message().c_str());
      return false;
    }
    _portPromise.set_value(localEndpoint.port());

    // Start threads
    startCallbackExecutor();
    _serviceThread = std::thread([this]() -> void { serviceLoop(); });
    return true;
  } catch (const std::exception &e) {
    plotly::logError("[WebsocketServer] Failed to start websocket server: %s",
                     e.what());
    return false;
  }
}

auto WebsocketServer::hasClient() const -> bool {
  std::scoped_lock lock(_connectionStateMutex);
  return !_connections.empty();
}

auto WebsocketServer::getPort() const -> int { return _portFuture.get(); }

void WebsocketServer::stop() {
  if (_serviceThread.joinable()) {
    _server.stop();
    _serviceThread.join();
  }
  WebsocketEndpointImpl::stopCallbackExecutor();
}

void WebsocketServer::waitUntilNoClient() const {
  std::unique_lock<std::mutex> lock(_connectionStateMutex);
  _connectedCv.wait(lock, [this]() -> bool { return _connections.empty(); });
}

void WebsocketServer::setupServerHandlers() {
  _server.set_socket_init_handler(
      [](const websocketpp::connection_hdl &,
         boost::asio::ip::tcp::socket &socket) -> void {
        socket.set_option(
            boost::asio::ip::tcp::no_delay(true)); // Disable Nagle
        plotly::logDebug("[WebsocketServer] Socket initialized");
      });

  _server.set_open_handler([this](const connection_hdl &hdl) -> void {
    {
      std::scoped_lock lock(_connectionStateMutex);
      _connections.insert(hdl);
    }
    _connectedCv.notify_all();
    plotly::logDebug("[WebsocketServer] Connection opened");
  });

  _server.set_close_handler([this](const connection_hdl &hdl) -> void {
    {
      std::scoped_lock lock(_connectionStateMutex);
      _connections.erase(hdl);
    }
    _connectedCv.notify_all();
    plotly::logDebug("[WebsocketServer] Connection closed");
  });
  _server.set_message_handler([this](const connection_hdl &hdl,
                                     const server_t::message_ptr &msg) -> void {
    handleMessage(msg->get_payload());
  });
}

// Interface implementations
auto WebsocketServer::waitConnection(std::chrono::milliseconds timeout) const
    -> bool {
  std::unique_lock<std::mutex> lock(_connectionStateMutex);
  return _connectedCv.wait_for(
      lock, timeout, [this]() -> bool { return !_connections.empty(); });
}

auto WebsocketServer::isConnected() const -> bool {
  std::scoped_lock lock(_connectionStateMutex);
  return !_connections.empty();
}

auto WebsocketServer::send(const std::string_view &message) -> bool {
  std::scoped_lock lock(_connectionStateMutex);
  if (_connections.empty()) {
    return false;
  }

  bool success = true;
  for (const auto &hdl : _connections) {
    try {
      _server.send(hdl, std::string(message), websocketpp::frame::opcode::text);
    } catch (const std::exception &e) {
      plotly::logWarn("[WebsocketServer] Failed to send message to client: %s",
                      e.what());
      success = false;
    }
  }
  return success;
}

auto WebsocketServer::getName() const -> std::string {
  return "WebSocketServer";
}

} // namespace plotly::detail
