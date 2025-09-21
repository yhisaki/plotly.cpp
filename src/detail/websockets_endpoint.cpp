#include "websockets_endpoint.hpp"
#include "plotly/logger.hpp"
#include <exception>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

namespace plotly::detail {

WebsocketEndpointInterface::WebsocketEndpointInterface() = default;
WebsocketEndpointInterface::~WebsocketEndpointInterface() = default;

void WebsocketEndpointImpl::callbackExecutorLoop() {
  while (running.load()) {
    std::unique_lock<std::mutex> lock(recvMessagesMutex);
    recvMessagesCv.wait(lock, [this]() -> bool {
      return !recvMessages.empty() || !running.load();
    });

    if (!running.load())
      break;

    if (recvMessages.empty())
      continue;

    std::string message = recvMessages.front();
    recvMessages.pop();
    lock.unlock();

    std::unordered_map<std::string, callback_t> callbacksCopy;
    {
      std::scoped_lock callbackLock(callbackMutex);
      callbacksCopy = this->callbacks;
    }

    if (running.load()) {
      for (const auto &[_, callback] : callbacksCopy) {
        if (!running.load())
          break;
        try {
          callback(message);
        } catch (const std::exception &e) {
          plotly::logError(
              "[%s] [WebsocketEndpoint] Callback execution failed: %s",
              getName().c_str(), e.what());
        }
      }
    }
  }
}

void WebsocketEndpointImpl::handleMessage(const std::string &message) {
  std::scoped_lock lock(recvMessagesMutex);
  recvMessages.emplace(message);
  recvMessagesCv.notify_all();
}

void WebsocketEndpointImpl::startCallbackExecutor() {
  running = true;
  callbackExecutorThread =
      std::thread([this]() -> void { callbackExecutorLoop(); });
}

void WebsocketEndpointImpl::stopCallbackExecutor() {
  if (!running.load())
    return;

  running.store(false);

  {
    std::scoped_lock lock(recvMessagesMutex);
    recvMessagesCv.notify_all();
  }

  if (callbackExecutorThread.joinable()) {
    callbackExecutorThread.join();
  }
}

void WebsocketEndpointImpl::registerCallback(const std::string_view &eventName,
                                             callback_t callback) {
  std::scoped_lock lock(callbackMutex);
  callbacks[std::string(eventName)] = std::move(callback);
}

void WebsocketEndpointImpl::unregisterCallback(
    const std::string_view &eventName) {
  std::scoped_lock lock(callbackMutex);
  callbacks.erase(std::string(eventName));
}

} // namespace plotly::detail
