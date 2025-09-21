#ifndef PLOTLY_DETAILS_WEBSOCKETS_CLIENT_HPP
#define PLOTLY_DETAILS_WEBSOCKETS_CLIENT_HPP

#include "websocketpp/config/asio_no_tls_client.hpp"
#include "websocketpp/roles/client_endpoint.hpp"
#include "websockets_endpoint.hpp"

namespace plotly::detail {

using client_config = websocketpp::config::asio_client;
using client_t = websocketpp::client<client_config>;
using connection_hdl = websocketpp::connection_hdl;

class WebsocketClient : public WebsocketEndpointImpl {
public:
  WebsocketClient();
  ~WebsocketClient();
  WebsocketClient(const WebsocketClient &) = delete;
  auto operator=(const WebsocketClient &) -> WebsocketClient & = delete;
  WebsocketClient(WebsocketClient &&) = delete;
  auto operator=(WebsocketClient &&) -> WebsocketClient & = delete;
  auto connect(const std::string_view &endpoint) -> bool;
  void stop() override;

  // Interface implementations
  auto waitConnection(std::chrono::milliseconds timeout) const -> bool override;
  auto isConnected() const -> bool override;
  auto send(const std::string_view &message) -> bool override;
  auto getName() const -> std::string override;

protected:
  void serviceLoop() override;

private:
  client_t _client;
  connection_hdl _currentConnection;
  std::string _endpointUri;
  std::thread _serviceThread;
  mutable std::mutex _connectionMutex;
  mutable std::condition_variable _connectionCv;
  std::atomic<bool> _connected{false};

  void setupClientHandlers();
  void onFail(const connection_hdl &hdl);
};

} // namespace plotly::detail

#endif
