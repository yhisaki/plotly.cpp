/**
 * @file websockets_endpoint.hpp
 * @brief WebSocket endpoint interface and implementation for plotly.cpp
 *
 * This file defines the WebSocket endpoint classes that provide a unified
 * interface for WebSocket communication in the plotly.cpp library. The design
 * supports both client and server endpoints through a common interface.
 */

#ifndef PLOTLY_DETAILS_WEBSOCKETS_ENDPOINT_HPP
#define PLOTLY_DETAILS_WEBSOCKETS_ENDPOINT_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

namespace plotly::detail {

/**
 * @class WebsocketEndpointInterface
 * @brief Abstract interface for WebSocket endpoints
 *
 * This class defines the common interface for both client and server WebSocket
 * endpoints. It provides methods for connection management, message sending,
 * event callbacks, and lifecycle management.
 *
 * The interface is designed to be thread-safe and supports asynchronous
 * operation with callback-based event handling.
 */
class WebsocketEndpointInterface {
public:
  /**
   * @brief Default constructor
   */
  WebsocketEndpointInterface();

  /**
   * @brief Virtual destructor
   */
  virtual ~WebsocketEndpointInterface();

  /**
   * @brief Copy constructor (deleted)
   */
  WebsocketEndpointInterface(const WebsocketEndpointInterface &) = delete;

  /**
   * @brief Copy assignment operator (deleted)
   */
  auto operator=(const WebsocketEndpointInterface &)
      -> WebsocketEndpointInterface & = delete;

  /**
   * @brief Wait for a connection to be established
   * @param timeout Maximum time to wait for connection
   * @return true if connection was established within timeout, false otherwise
   *
   * This method blocks until either a connection is established or the timeout
   * expires. It's useful for synchronizing with connection establishment in
   * client scenarios or waiting for incoming connections in server scenarios.
   */
  [[nodiscard]] virtual auto
  waitConnection(std::chrono::milliseconds timeout) const -> bool = 0;

  /**
   * @brief Check if the endpoint is currently connected
   * @return true if connected, false otherwise
   *
   * This method provides a non-blocking way to check the current connection
   * status of the endpoint.
   */
  [[nodiscard]] virtual auto isConnected() const -> bool = 0;

  /**
   * @brief Send a message through the WebSocket connection
   * @param message The message to send
   * @return true if message was sent successfully, false otherwise
   *
   * This method attempts to send a message through the WebSocket connection.
   * The operation may fail if the connection is not established or if there
   * are network issues.
   */
  virtual auto send(const std::string_view &message) -> bool = 0;

  /**
   * @brief Callback function type for handling incoming messages
   * @param message The received message
   */
  using callback_t = std::function<void(const std::string_view &)>;

  /**
   * @brief Register a callback for a specific event
   * @param eventName The name of the event to listen for
   * @param callback The callback function to invoke when the event occurs
   *
   * This method registers a callback function that will be called when a
   * message with the specified event name is received. Only one callback
   * can be registered per event name.
   */
  virtual void registerCallback(const std::string_view &eventName,
                                callback_t callback) = 0;

  /**
   * @brief Unregister a callback for a specific event
   * @param eventName The name of the event to stop listening for
   *
   * This method removes the callback associated with the specified event name.
   * After calling this method, no callback will be invoked for the event.
   */
  virtual void unregisterCallback(const std::string_view &eventName) = 0;

  /**
   * @brief Stop the endpoint and clean up resources
   *
   * This method stops all endpoint operations, closes connections, and
   * performs necessary cleanup. After calling this method, the endpoint
   * should not be used for further operations.
   */
  virtual void stop() = 0;

  /**
   * @brief Get the name/identifier of this endpoint
   * @return A string identifying this endpoint
   *
   * This method returns a human-readable name or identifier for the endpoint,
   * useful for logging and debugging purposes.
   */
  [[nodiscard]] virtual auto getName() const -> std::string = 0;
};

/**
 * @class WebsocketEndpointImpl
 * @brief Base implementation class for WebSocket endpoints
 *
 * This class provides a concrete implementation of the
 * WebsocketEndpointInterface with common functionality shared between client
 * and server endpoints. It handles message queuing, callback management, and
 * threading infrastructure.
 *
 * The implementation uses a multi-threaded design:
 * - A service thread for handling WebSocket operations (derived classes
 * implement serviceLoop)
 * - A callback executor thread for processing incoming messages and invoking
 * callbacks
 * - Thread-safe message queuing between the service and callback threads
 *
 * Derived classes need to implement the pure virtual serviceLoop() method to
 * provide specific client or server behavior.
 */
class WebsocketEndpointImpl : public WebsocketEndpointInterface {
protected:
  /**
   * @brief Atomic flag indicating if the endpoint is running
   */
  std::atomic<bool> running{false};

  /**
   * @brief Service thread for handling WebSocket operations
   */
  std::thread serviceThread;

  /**
   * @brief Mutex protecting the received messages queue
   */
  std::mutex recvMessagesMutex;

  /**
   * @brief Condition variable for notifying about new received messages
   */
  std::condition_variable recvMessagesCv;

  /**
   * @brief Queue of received messages waiting to be processed
   */
  std::queue<std::string> recvMessages;

  /**
   * @brief Mutex protecting the callbacks map
   */
  std::mutex callbackMutex;

  /**
   * @brief Map of event names to their associated callback functions
   */
  std::unordered_map<std::string, callback_t> callbacks;

  /**
   * @brief Thread for executing callbacks
   */
  std::thread callbackExecutorThread;

  /**
   * @brief Pure virtual method for the main service loop
   *
   * Derived classes must implement this method to provide specific WebSocket
   * client or server behavior. This method runs in the service thread and
   * should handle connection management and message reception.
   */
  virtual void serviceLoop() = 0;

  /**
   * @brief Main loop for the callback executor thread
   *
   * This method runs in the callback executor thread and processes received
   * messages by invoking the appropriate registered callbacks.
   */
  void callbackExecutorLoop();

  /**
   * @brief Handle an incoming message by queuing it for callback processing
   * @param message The received message to handle
   *
   * This method is called by the service thread to queue received messages
   * for processing by the callback executor thread.
   */
  void handleMessage(const std::string &message);

  /**
   * @brief Start the callback executor thread
   *
   * This method initializes and starts the callback executor thread that
   * processes incoming messages and invokes registered callbacks.
   */
  void startCallbackExecutor();

public:
  /**
   * @brief Stop the callback executor thread and clean up resources
   *
   * This method stops the callback executor thread and performs necessary
   * cleanup. It's called automatically by the destructor but can also be
   * called explicitly if needed.
   */
  void stopCallbackExecutor();

  /**
   * @brief Register a callback for a specific event (override)
   * @param eventName The name of the event to listen for
   * @param callback The callback function to invoke when the event occurs
   *
   * This implementation stores the callback in the callbacks map in a
   * thread-safe manner using the callback mutex.
   */
  void registerCallback(const std::string_view &eventName,
                        callback_t callback) override;

  /**
   * @brief Unregister a callback for a specific event (override)
   * @param eventName The name of the event to stop listening for
   *
   * This implementation removes the callback from the callbacks map in a
   * thread-safe manner using the callback mutex.
   */
  void unregisterCallback(const std::string_view &eventName) override;
};
} // namespace plotly::detail
#endif
