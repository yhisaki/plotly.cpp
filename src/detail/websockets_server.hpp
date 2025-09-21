#ifndef PLOTLY_DETAILS_WEBSOCKETS_SERVER_HPP
#define PLOTLY_DETAILS_WEBSOCKETS_SERVER_HPP

#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/roles/server_endpoint.hpp"
#include "websockets_endpoint.hpp"
#include <future>
#include <set>

namespace plotly::detail {
using server_config = websocketpp::config::asio;
using server_t = websocketpp::server<server_config>;
using connection_hdl = websocketpp::connection_hdl;
using endpoint_t = server_t;

class WebsocketServer final : public WebsocketEndpointImpl {
public:
  WebsocketServer();
  ~WebsocketServer();
  WebsocketServer(const WebsocketServer &) = delete;
  auto operator=(const WebsocketServer &) -> WebsocketServer & = delete;
  WebsocketServer(WebsocketServer &&) = delete;
  auto operator=(WebsocketServer &&) -> WebsocketServer & = delete;

  auto serve(std::string_view address, int port) -> bool;
  auto hasClient() const -> bool;
  void waitUntilNoClient() const;
  auto getPort() const -> int;
  void stop() override;

public:
  auto waitConnection(std::chrono::milliseconds timeout) const -> bool override;
  auto isConnected() const -> bool override;
  auto send(const std::string_view &message) -> bool override;
  auto getName() const -> std::string override;

protected:
  void serviceLoop() override;

private:
  mutable std::condition_variable _connectedCv;
  mutable std::mutex _connectionStateMutex;
  std::set<connection_hdl, std::owner_less<connection_hdl>> _connections;

  server_t _server;
  std::promise<int> _portPromise;
  mutable std::shared_future<int> _portFuture;
  std::thread _serviceThread;

  void setupServerHandlers();
};

} // namespace plotly::detail

#endif
