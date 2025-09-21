/**
 * @file json_rpc.hpp
 * @brief JSON-RPC 2.0 WebSocket server implementation.
 */

#ifndef PLOTLY_DETAILS_JSON_RPC_HPP
#define PLOTLY_DETAILS_JSON_RPC_HPP

#include "websockets_endpoint.hpp"
#include <nlohmann/json.hpp>

#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>

namespace plotly::detail {

/**
 * @brief Enumeration of standard JSON-RPC 2.0 error codes.
 */
enum class JsonRpcErrorCode : std::int16_t {
  PARSE_ERROR = -32700,      ///< Invalid JSON was received by the server
  INVALID_REQUEST = -32600,  ///< The JSON sent is not a valid Request object
  METHOD_NOT_FOUND = -32601, ///< The method does not exist / is not available
  INVALID_PARAMS = -32602,   ///< Invalid method parameter(s)
  INTERNAL_ERROR = -32603,   ///< Internal JSON-RPC error
  SERVER_ERROR = -32000      ///< Server error
};

/**
 * @brief Structure representing a JSON-RPC error object.
 */
struct JsonRpcError {
  int code;            ///< The error code
  std::string message; ///< The error message
  nlohmann::json data; ///< Additional error data
};

/**
 * @brief Structure representing a JSON-RPC response object.
 */
struct JsonRpcResponse {
  std::optional<nlohmann::json> id; ///< Request ID (null for notifications)
  std::optional<nlohmann::json>
      result; ///< The result of the request (if successful)
  std::optional<JsonRpcError> error; ///< The error object (if request failed)
  std::string jsonrpc = "2.0";       ///< JSON-RPC version string

  /**
   * @brief Converts the response to a JSON object.
   * @return JSON object representation of the response.
   */
  [[nodiscard]] auto toJson() const -> nlohmann::json;
};

/**
 * @brief JSON-RPC 2.0 server implementation over WebSocket.
 */
class JsonRpc {
public:
  /**
   * @brief Constructs a new JSON-RPC server.
   */
  explicit JsonRpc(std::unique_ptr<WebsocketEndpointInterface> wsEndpoint);

  /**
   * @brief Destructor. Stops the server if running.
   */
  ~JsonRpc();

  // Delete copy constructor and assignment operator
  JsonRpc(const JsonRpc &) = delete;
  auto operator=(const JsonRpc &) -> JsonRpc & = delete;

  // Move constructor and assignment operator
  JsonRpc(JsonRpc &&other) noexcept;
  auto operator=(JsonRpc &&other) noexcept -> JsonRpc &;

  /**
   * @brief Registers a handler function for a specific RPC method.
   * @param method The name of the RPC method.
   * @param handler The handler function to be called when the method is
   * invoked.
   */
  void registerHandler(
      const std::string &method,
      const std::function<nlohmann::json(const nlohmann::json &)> &handler);

  void
  registerNotification(const std::string &method,
                       std::function<void(const nlohmann::json &)> handler);

  void unregisterHandler(const std::string &method);

  /**
   * @brief Registers a callback function for the WebSocket endpoint.
   * @param callbackName The name of the callback to register.
   * @param callback The callback function to be called when messages are
   * received.
   */
  void registerCallbackWithWebsocket(
      const std::string &callbackName,
      std::function<void(const std::string_view)> callback);

  /**
   * @brief Unregisters a callback function from the WebSocket endpoint.
   * @param callbackName The name of the callback to unregister.
   */
  void unregisterCallbackFromWebsocket(const std::string &callbackName);

  /**
   * @brief Unregisters all callbacks from the WebSocket endpoint.
   */
  void unregisterAllCallbacksFromWebsockets();

  /**
   * @brief Makes an asynchronous JSON-RPC call to the connected client.
   * @param method The name of the method to call.
   * @param params The parameters to pass to the method.
   * @return A pair containing a future containing the result of the RPC call
   * and a function to cancel the call.
   */
  auto call(const std::string &method, const nlohmann::json &params)
      -> std::pair<std::future<nlohmann::json>, std::function<void()>>;

  /**
   * @brief Sends a notification to all connected clients.
   * @param method The name of the method to call.
   * @param params The parameters to pass to the method.
   */
  void notify(const std::string &method, const nlohmann::json &params);

  /**
   * @brief Gets a raw pointer to the underlying websocket endpoint.
   * @return Raw pointer to the websocket endpoint (for compatibility).
   */
  [[nodiscard]] auto getWebsocketEndpoint() const
      -> WebsocketEndpointInterface *;

private:
  std::unique_ptr<WebsocketEndpointInterface> _wsEndpoint;

  void handleIncomingMessage(const std::string_view &message);
  void sendSuccessResponse(const nlohmann::json &requestId,
                           const nlohmann::json &result);
  void sendErrorResponse(const nlohmann::json &requestId,
                         JsonRpcErrorCode errorCode,
                         const std::string &message);

  std::unordered_map<std::string,
                     std::function<nlohmann::json(const nlohmann::json &)>>
      _handlers;
  std::unordered_map<std::string, std::function<void(const nlohmann::json &)>>
      _notifications;
  std::unordered_set<std::string> _registeredCallbacks;
  mutable std::mutex _registeredCallbacksMutex;
};

} // namespace plotly::detail
#endif // PLOTLY_DETAILS_JSON_RPC_HPP
