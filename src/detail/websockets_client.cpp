#include "websockets_client.hpp"
#include "detail/websockets_endpoint.hpp"
#include "plotly/logger.hpp"
#include "websocketpp/common/connection_hdl.hpp"
#include "websocketpp/common/system_error.hpp"
#include "websocketpp/frame.hpp"
#include "websocketpp/logger/levels.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <chrono>
#include <exception>
#include <mutex>
#include <string>
#include <string_view>

namespace plotly::detail {

WebsocketClient::WebsocketClient() {
  _client.set_access_channels(websocketpp::log::alevel::none);
  _client.clear_access_channels(websocketpp::log::alevel::none);
  // Only log serious errors, exclude recoverable errors like EOF
  _client.set_error_channels(websocketpp::log::elevel::warn |
                             websocketpp::log::elevel::fatal);
  _client.init_asio();

  setupClientHandlers();
}

WebsocketClient::~WebsocketClient() { stop(); }

auto WebsocketClient::connect(const std::string_view &endpoint) -> bool {
  try {
    _endpointUri = std::string(endpoint);

    // Create connection
    websocketpp::lib::error_code ec;
    auto con = _client.get_connection(_endpointUri, ec);
    if (ec) {
      plotly::logError("[WebsocketClient] Failed to create connection: %s",
                       ec.message().c_str());
      return false;
    }

    _currentConnection = con->get_handle();
    _client.connect(con);

    // Start threads
    startCallbackExecutor();
    _serviceThread = std::thread([this]() -> void { serviceLoop(); });
    return true;
  } catch (const std::exception &e) {
    plotly::logError(
        "[WebsocketClient] Failed to connect to websocket server: %s",
        e.what());
    return false;
  }
}

void WebsocketClient::setupClientHandlers() {

  _client.set_socket_init_handler(
      [](const websocketpp::connection_hdl &,
         boost::asio::ip::tcp::socket &socket) -> void {
        socket.set_option(
            boost::asio::ip::tcp::no_delay(true)); // Disable Nagle
        plotly::logDebug("[WebsocketClient] Socket initialized");
      });

  _client.set_open_handler([this](const connection_hdl &hdl) -> void {
    {
      std::scoped_lock lock(_connectionMutex);
      _connected = true;
    }
    _connectionCv.notify_all();
    plotly::logDebug("[WebsocketClient] Connection opened");
  });

  _client.set_close_handler([this](const connection_hdl &hdl) -> void {
    {
      std::scoped_lock lock(_connectionMutex);
      _connected = false;
    }
    _connectionCv.notify_all();
    plotly::logDebug("[WebsocketClient] Connection closed");
  });

  _client.set_fail_handler(
      [this](const connection_hdl &hdl) -> void { onFail(hdl); });
  _client.set_message_handler([this](const connection_hdl &hdl,
                                     const client_t::message_ptr &msg) -> void {
    handleMessage(msg->get_payload());
  });
}

void WebsocketClient::onFail(const connection_hdl &hdl) {
  plotly::logError("[WebsocketClient] Connection failed");
  {
    std::scoped_lock lock(_connectionMutex);
    _connected = false;
  }
  _connectionCv.notify_all();
}

void WebsocketClient::stop() {
  if (_serviceThread.joinable()) {
    _client.stop();
    _serviceThread.join();
  }
  WebsocketEndpointImpl::stopCallbackExecutor();
}

void WebsocketClient::serviceLoop() {
  try {
    _client.run();
  } catch (const std::exception &e) {
    plotly::logError("[WebsocketClient] Service loop error: %s", e.what());
  }
}

// Interface implementations
auto WebsocketClient::waitConnection(std::chrono::milliseconds timeout) const
    -> bool {
  std::unique_lock<std::mutex> lock(_connectionMutex);
  return _connectionCv.wait_for(lock, timeout,
                                [this]() -> bool { return _connected.load(); });
}

auto WebsocketClient::isConnected() const -> bool { return _connected.load(); }

auto WebsocketClient::send(const std::string_view &message) -> bool {
  if (!_connected.load()) {
    return false;
  }

  try {
    _client.send(_currentConnection, std::string(message),
                 websocketpp::frame::opcode::text);
    return true;
  } catch (const std::exception &e) {
    plotly::logWarn("[WebsocketClient] Failed to send message to server: %s",
                    e.what());
    return false;
  }
}

auto WebsocketClient::getName() const -> std::string {
  return "WebSocketClient";
}

} // namespace plotly::detail
