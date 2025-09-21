#include "json_rpc.hpp"
#include "plotly/logger.hpp"
#include "uuid.hpp"
#include "websockets_endpoint.hpp"
#include <cstring>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
namespace plotly::detail {

auto JsonRpcResponse::toJson() const -> nlohmann::json {
  nlohmann::json response;
  response["jsonrpc"] = jsonrpc;
  if (id.has_value()) {
    response["id"] = id.value();
  } else {
    response["id"] = nullptr;
  }
  if (result) {
    response["result"] = *result;
  }
  if (error) {
    response["error"] = {{"code", error->code},
                         {"message", error->message},
                         {"data", error->data}};
  }
  return response;
}

// Public interface implementation - delegates to impl
JsonRpc::JsonRpc(std::unique_ptr<WebsocketEndpointInterface> wsEndpoint)
    : _wsEndpoint(std::move(wsEndpoint)) {
  // Register message handler for incoming WebSocket messages
  this->registerCallbackWithWebsocket(
      "jsonrpc_handler", [this](const std::string_view messageSv) -> void {
        handleIncomingMessage(messageSv);
      });
}

JsonRpc::~JsonRpc() {
  if (_wsEndpoint) {
    _wsEndpoint->stop();
    // Unregister all tracked callbacks
    unregisterAllCallbacksFromWebsockets();
  }
}

JsonRpc::JsonRpc(JsonRpc &&other) noexcept
    : _wsEndpoint(std::move(other._wsEndpoint)),
      _handlers(std::move(other._handlers)),
      _notifications(std::move(other._notifications)),
      _registeredCallbacks(std::move(other._registeredCallbacks)) {
  // The moved-from object is left in a valid but unspecified state
  // The websocket endpoint is now owned by this instance
}

auto JsonRpc::operator=(JsonRpc &&other) noexcept -> JsonRpc & {
  if (this != &other) {
    // Clean up current resources
    if (_wsEndpoint) {
      _wsEndpoint->stop();
      unregisterAllCallbacksFromWebsockets();
    }

    // Move resources from other
    _wsEndpoint = std::move(other._wsEndpoint);
    _handlers = std::move(other._handlers);
    _notifications = std::move(other._notifications);
    _registeredCallbacks = std::move(other._registeredCallbacks);
  }
  return *this;
}

void JsonRpc::registerHandler(
    const std::string &method,
    const std::function<nlohmann::json(const nlohmann::json &)> &handler) {
  _handlers[method] = handler;
}

void JsonRpc::registerNotification(
    const std::string &method,
    std::function<void(const nlohmann::json &)> handler) {
  _notifications[method] = std::move(handler);
  plotly::logDebug("[JsonRpc] Registered notification handler for: %s",
                   method.c_str());
}

void JsonRpc::unregisterHandler(const std::string &method) {
  _handlers.erase(method);
}

void JsonRpc::registerCallbackWithWebsocket(
    const std::string &callbackName,
    std::function<void(const std::string_view)> callback) {
  // Store the callback name in our set for tracking
  {
    std::scoped_lock lock(_registeredCallbacksMutex);
    _registeredCallbacks.insert(callbackName);
  }
  // Register with the WebSocket endpoint
  _wsEndpoint->registerCallback(callbackName, std::move(callback));
}

void JsonRpc::unregisterCallbackFromWebsocket(const std::string &callbackName) {
  // Remove from our tracking set
  {
    std::scoped_lock lock(_registeredCallbacksMutex);
    _registeredCallbacks.erase(callbackName);
  }
  // Unregister from the WebSocket endpoint
  _wsEndpoint->unregisterCallback(callbackName);
}

void JsonRpc::unregisterAllCallbacksFromWebsockets() {
  std::scoped_lock lock(_registeredCallbacksMutex);
  for (const auto &callbackName : _registeredCallbacks) {
    _wsEndpoint->unregisterCallback(callbackName);
  }
  _registeredCallbacks.clear();
}

auto JsonRpc::call(const std::string &method, const nlohmann::json &params)
    -> std::pair<std::future<nlohmann::json>, std::function<void()>> {

  static int requestId = 0;
  requestId++;

  std::shared_ptr<std::promise<nlohmann::json>> responsePromise =
      std::make_shared<std::promise<nlohmann::json>>();

  std::string requestRetriveEventName = detail::generateUUID();
  this->registerCallbackWithWebsocket(
      requestRetriveEventName,
      [responsePromise, this, requestRetriveEventName, requestId = requestId,
       method = method](const std::string_view messageSv) -> void {
        nlohmann::json response = nlohmann::json::parse(messageSv);
        if (response["id"] == requestId) {
          plotly::logDebug(
              "[JsonRpc] Received response for %s method, response: %s",
              method.c_str(), response.dump().c_str());
          responsePromise->set_value(response["result"]);
          this->unregisterCallbackFromWebsocket(requestRetriveEventName);
        }
      });

  nlohmann::json request;
  request["jsonrpc"] = "2.0";
  request["method"] = method;
  request["params"] = params;
  request["id"] = requestId;
  _wsEndpoint->send(request.dump());
  plotly::logDebug("[JsonRpc] Called %s method", method.c_str());

  std::function<void()> cancel = [responsePromise, this,
                                  requestRetriveEventName]() -> void {
    responsePromise->set_value(nlohmann::json());
    this->unregisterCallbackFromWebsocket(requestRetriveEventName);
  };

  return {responsePromise->get_future(), cancel};
}

void JsonRpc::notify(const std::string &method, const nlohmann::json &params) {
  nlohmann::json request;
  request["jsonrpc"] = "2.0";
  request["method"] = method;
  request["params"] = params;
  _wsEndpoint->send(request.dump());
}

auto JsonRpc::getWebsocketEndpoint() const -> WebsocketEndpointInterface * {
  if (_wsEndpoint == nullptr) {
    throw std::runtime_error("Websocket endpoint is not initialized");
  }
  return _wsEndpoint.get();
}

void JsonRpc::handleIncomingMessage(const std::string_view &message) {
  try {
    // Parse the incoming JSON message
    nlohmann::json request = nlohmann::json::parse(message);

    // Validate JSON-RPC format
    if (!request.contains("jsonrpc") || request["jsonrpc"] != "2.0" ||
        !request.contains("method") || !request["method"].is_string()) {
      // Invalid request format
      if (request.contains("id")) {
        sendErrorResponse(request["id"], JsonRpcErrorCode::INVALID_REQUEST,
                          "Invalid JSON-RPC request format");
      }
      return;
    }

    std::string method = request["method"];
    nlohmann::json params =
        request.contains("params") ? request["params"] : nlohmann::json();

    // Check if this is a notification (no id field)
    bool isNotification = !request.contains("id");

    if (isNotification) {
      // Handle notification
      auto notificationIt = _notifications.find(method);
      if (notificationIt != _notifications.end()) {
        notificationIt->second(params);
      }
      // No response sent for notifications
      return;
    }

    // Handle method call
    nlohmann::json requestId = request["id"];
    auto handlerIt = _handlers.find(method);

    if (handlerIt == _handlers.end()) {
      // Method not found
      sendErrorResponse(requestId, JsonRpcErrorCode::METHOD_NOT_FOUND,
                        "Method not found: " + method);
      return;
    }

    try {
      // Call the handler and send the result
      nlohmann::json result = handlerIt->second(params);
      sendSuccessResponse(requestId, result);
    } catch (const std::exception &e) {
      // Handler threw an exception
      plotly::logError("Handler for method %s threw exception: %s",
                       method.c_str(), e.what());
      sendErrorResponse(requestId, JsonRpcErrorCode::INTERNAL_ERROR,
                        "Internal error: " + std::string(e.what()));
    }

  } catch (const std::exception &e) {
    // JSON parsing error
    plotly::logError("JSON-RPC parsing error: %s", e.what());
    sendErrorResponse(nlohmann::json(nullptr), JsonRpcErrorCode::PARSE_ERROR,
                      "Parse error: " + std::string(e.what()));
  }
}

void JsonRpc::sendSuccessResponse(const nlohmann::json &requestId,
                                  const nlohmann::json &result) {
  JsonRpcResponse response;
  response.id = requestId;
  response.result = result;
  _wsEndpoint->send(response.toJson().dump());
}

void JsonRpc::sendErrorResponse(const nlohmann::json &requestId,
                                JsonRpcErrorCode errorCode,
                                const std::string &message) {
  JsonRpcResponse response;
  response.id = requestId;
  JsonRpcError error;
  error.code = static_cast<int>(errorCode);
  error.message = message;
  error.data = nlohmann::json();
  response.error = error;
  _wsEndpoint->send(response.toJson().dump());
}

} // namespace plotly::detail
