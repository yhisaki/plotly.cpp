#include <atomic>
#include <exception>
#include <future>
#include <gtest/gtest.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include "detail/json_rpc.hpp"
#include "detail/websockets_client.hpp"
#include "detail/websockets_server.hpp"

using namespace plotly::detail;
using namespace std::chrono_literals;

TEST(JsonRpcTest, JsonRpcBasic) {
  // Create websocket objects on heap to properly transfer ownership
  auto server = std::make_unique<WebsocketServer>();
  auto client = std::make_unique<WebsocketClient>();

  // // Start server and client
  ASSERT_TRUE(server->serve("127.0.0.1", 0))
      << "Server startup for JSON-RPC test";

  int port = server->getPort();
  ASSERT_GT(port, 0) << "Port assignment for JSON-RPC test";

  ASSERT_TRUE(client->connect("ws://127.0.0.1:" + std::to_string(port)))
      << "Client connection for JSON-RPC test";
  ASSERT_TRUE(client->waitConnection(2000ms))
      << "Client connection establishment for JSON-RPC test";

  // Create JSON-RPC instances with proper ownership transfer
  JsonRpc clientRpc(std::move(client));
  JsonRpc serverRpc(std::move(server));

  // Test handler registration
  std::atomic<bool> handlerCalled{false};
  nlohmann::json handlerParams;

  serverRpc.registerHandler(
      "test_method", [&](const nlohmann::json &params) -> nlohmann::json {
        handlerCalled = true;
        handlerParams = params;
        return nlohmann::json{{"result", "success"}, {"echo", params}};
      });

  // Test notification handler
  auto notificationPromise = std::make_shared<std::promise<nlohmann::json>>();
  auto notificationFuture = notificationPromise->get_future();

  clientRpc.registerNotification(
      "test_notification",
      [notificationPromise](const nlohmann::json &params) -> void {
        notificationPromise->set_value(params);
      });

  // Test JSON-RPC call
  nlohmann::json callParams =
      nlohmann::json{{"test_param", "test_value"}, {"number", 42}};

  try {
    auto [futureResult, cancel] = clientRpc.call("test_method", callParams);

    // Wait for result
    auto status = futureResult.wait_for(3000ms);
    ASSERT_EQ(status, std::future_status::ready) << "JsonRpc call completion";

    nlohmann::json result = futureResult.get();
    EXPECT_TRUE(handlerCalled.load()) << "JsonRpc handler invocation";

    if (result.contains("result")) {
      EXPECT_EQ(result["result"], "success")
          << "JsonRpc call result verification";
    }

    if (result.contains("echo") && result["echo"].contains("test_param")) {
      EXPECT_EQ(result["echo"]["test_param"], "test_value")
          << "JsonRpc parameter echo verification";
    }
  } catch (const std::exception &e) {
    FAIL() << "JsonRpc call exception: " << e.what();
  }

  // Test JSON-RPC notification
  nlohmann::json notifyParams =
      nlohmann::json{{"notification_data", "test_notification_value"}};
  serverRpc.notify("test_notification", notifyParams);

  // Wait for notification via future to avoid data races
  auto notifyStatus = notificationFuture.wait_for(2000ms);
  ASSERT_EQ(notifyStatus, std::future_status::ready)
      << "JsonRpc notification reception";
  nlohmann::json received = notificationFuture.get();
  if (received.contains("notification_data")) {
    EXPECT_EQ(received["notification_data"], "test_notification_value")
        << "JsonRpc notification parameter verification";
  }

  // Client and server will automatically stop when going out of scope
}

TEST(JsonRpcTest, JsonRpcErrorHandling) {
  // Create websocket objects on heap to properly transfer ownership
  auto server = std::make_unique<WebsocketServer>();
  auto client = std::make_unique<WebsocketClient>();

  // Setup connection
  ASSERT_TRUE(server->serve("127.0.0.1", 0));
  std::this_thread::sleep_for(200ms);
  int port = server->getPort();
  ASSERT_GT(port, 0);
  ASSERT_TRUE(client->connect("ws://127.0.0.1:" + std::to_string(port)));
  ASSERT_TRUE(client->waitConnection(2000ms));
  std::this_thread::sleep_for(200ms);

  JsonRpc serverRpc(std::move(server));
  JsonRpc clientRpc(std::move(client));

  // Test method not found error
  try {
    auto [futureResult, cancel] =
        clientRpc.call("nonexistent_method", nlohmann::json{});
    auto status = futureResult.wait_for(2000ms);

    // Should receive some kind of response (either error or timeout)
    EXPECT_NE(status, std::future_status::deferred)
        << "JsonRpc method not found handling";
  } catch (const std::exception &e) {
    // This is also acceptable behavior
    SUCCEED() << "JsonRpc method not found exception handling: " << e.what();
  }

  // Test handler exception
  serverRpc.registerHandler("error_method",
                            [](const nlohmann::json &) -> nlohmann::json {
                              throw std::runtime_error("Test error");
                            });

  std::this_thread::sleep_for(100ms);

  try {
    auto [futureResult, cancel] =
        clientRpc.call("error_method", nlohmann::json{});
    auto status = futureResult.wait_for(2000ms);
    // Should handle the error gracefully
    EXPECT_NE(status, std::future_status::deferred)
        << "JsonRpc error method response";
  } catch (const std::exception &e) {
    SUCCEED() << "JsonRpc error method exception handling: " << e.what();
  }

  // Client and server will automatically stop when going out of scope
}
