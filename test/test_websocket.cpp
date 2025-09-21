#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <gtest/gtest.h>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "detail/websockets_client.hpp"
#include "detail/websockets_endpoint.hpp"
#include "detail/websockets_server.hpp"
#include "plotly/logger.hpp"

using namespace plotly::detail;
using namespace std::chrono_literals;

TEST(WebSocketTest, ServerBasicFunctionality) {
  WebsocketServer server;

  // Test server startup
  plotly::logInfo("Starting WebsocketServer on %s:%d", "127.0.0.1", 0);
  bool started = server.serve("127.0.0.1", 0); // Use port 0 for auto-assignment
  plotly::logInfo("WebsocketServer serve returned: %s",
                  started ? "true" : "false");
  ASSERT_TRUE(started) << "WebsocketServer startup with auto port";

  if (started) {
    // Wait for port assignment
    int port = server.getPort();
    plotly::logInfo("WebsocketServer assigned port: %d", port);
    EXPECT_GT(port, 0) << "WebsocketServer port assignment";

    // Test initial client state
    bool initialHasClient = server.hasClient();
    plotly::logInfo("WebsocketServer initial hasClient: %s",
                    initialHasClient ? "true" : "false");
    EXPECT_FALSE(initialHasClient) << "WebsocketServer initial client state";
  }
}

TEST(WebSocketTest, ClientBasicFunctionality) {
  WebsocketClient client;

  // Test invalid connection (should fail gracefully)
  const char *invalidUrl = "ws://127.0.0.1:99999/test";
  plotly::logInfo("Attempting invalid client connect to %s", invalidUrl);
  bool connected = client.connect(invalidUrl);
  plotly::logInfo("Client connect returned: %s", connected ? "true" : "false");
  bool waitOk = client.waitConnection(1000ms);
  plotly::logInfo("Client waitConnection(1000ms) returned: %s",
                  waitOk ? "true" : "false");
  EXPECT_TRUE(!connected || !waitOk)
      << "WebsocketClient invalid connection handling";

  // Client will automatically stop when going out of scope
}

TEST(WebSocketTest, ServerClientConnection) {
  WebsocketServer server;
  WebsocketClient client;

  // Start server
  plotly::logInfo("Starting server for connection test on %s:%d", "127.0.0.1",
                  0);
  bool serverStarted = server.serve("127.0.0.1", 0);
  plotly::logInfo("Server started: %s", serverStarted ? "true" : "false");
  ASSERT_TRUE(serverStarted) << "Server startup for connection test";

  // Wait for server to be ready and get port
  int port = server.getPort();
  plotly::logInfo("Server listening on port: %d", port);
  ASSERT_GT(port, 0) << "Server port assignment for connection test";

  // Connect client to server
  std::string serverUrl = "ws://127.0.0.1:" + std::to_string(port);
  plotly::logInfo("Client connecting to %s", serverUrl.c_str());
  bool clientConnected = client.connect(serverUrl);
  plotly::logInfo("Client connect returned: %s",
                  clientConnected ? "true" : "false");
  EXPECT_TRUE(clientConnected) << "Client connection initiation";

  if (clientConnected) {
    // Wait for connection to establish
    bool connectionEstablished = client.waitConnection(2000ms);
    plotly::logInfo("Client waitConnection(2000ms): %s",
                    connectionEstablished ? "true" : "false");
    EXPECT_TRUE(connectionEstablished) << "Client connection establishment";

    if (connectionEstablished) {
      // Give server time to detect client
      bool serverSawConnection = server.waitConnection(2000ms);
      plotly::logInfo("Server waitConnection(2000ms): %s",
                      serverSawConnection ? "true" : "false");
      EXPECT_TRUE(serverSawConnection) << "Server client detection";
    }
  }

  // Client and server will automatically stop when going out of scope
}

TEST(WebSocketTest, Messaging) {
  WebsocketServer server;
  WebsocketClient client;

  // Start server
  plotly::logInfo("Starting server for messaging test on %s:%d", "127.0.0.1",
                  0);
  ASSERT_TRUE(server.serve("127.0.0.1", 0))
      << "Server startup for messaging test";

  int port = server.getPort();
  plotly::logInfo("Messaging test server listening on port: %d", port);
  ASSERT_GT(port, 0) << "Port assignment for messaging test";

  // Connect client
  std::string serverUrl = "ws://127.0.0.1:" + std::to_string(port);
  plotly::logInfo("Client connecting to %s", serverUrl.c_str());
  ASSERT_TRUE(client.connect(serverUrl))
      << "Client connection for messaging test";
  ASSERT_TRUE(client.waitConnection(500ms))
      << "Client connection establishment for messaging test";

  // Synchronization primitives for exclusive control and timing
  std::mutex mtx;
  std::condition_variable cv;
  bool serverReceived = false;
  bool clientReceived = false;
  std::string receivedServerMessage;
  std::string receivedClientMessage;

  // Register server message handler once
  plotly::logInfo("Registering server callback");
  server.registerCallback("test_handler",
                          [&](const std::string_view &msg) -> void {
                            std::scoped_lock lock(mtx);
                            receivedServerMessage = std::string(msg);
                            serverReceived = true;
                            cv.notify_all();
                          });

  // Run 10 iterations with client register/unregister per loop
  for (int i = 0; i < 10; ++i) {

    client.registerCallback("test_handler",
                            [&](const std::string_view &msg) -> void {
                              std::scoped_lock lock(mtx);
                              receivedClientMessage = std::string(msg);
                              clientReceived = true;
                              cv.notify_all();
                            });

    // Client -> Server
    {
      std::unique_lock<std::mutex> lock(mtx);
      serverReceived = false;
    }
    std::string testMessage1 = "Hello from client " + std::to_string(i);
    plotly::logInfo("Client sending: %s", testMessage1.c_str());
    auto startC2S = std::chrono::steady_clock::now();
    bool sentC2S = client.send(testMessage1);
    plotly::logInfo("Client send returned: %s", sentC2S ? "true" : "false");
    EXPECT_TRUE(sentC2S) << "Client message send";
    {
      std::unique_lock<std::mutex> lock(mtx);
      bool ok =
          cv.wait_for(lock, 2000ms, [&]() -> bool { return serverReceived; });
      plotly::logInfo("Server received flag: %s", ok ? "true" : "false");
      EXPECT_TRUE(ok) << "Server message reception";
      if (ok) {
        plotly::logInfo("Server received content: %s",
                        receivedServerMessage.c_str());
        EXPECT_EQ(receivedServerMessage, testMessage1)
            << "Server message content verification";
      }
    }
    auto endC2S = std::chrono::steady_clock::now();
    auto durC2S =
        std::chrono::duration_cast<std::chrono::microseconds>(endC2S - startC2S)
            .count();
    plotly::logInfo("Iteration %d client->server duration: %lld us", i,
                    static_cast<long long>(durC2S));

    // Server -> Client
    {
      std::unique_lock<std::mutex> lock(mtx);
      clientReceived = false;
    }
    std::string testMessage2 = "Hello from server " + std::to_string(i);
    plotly::logInfo("Server sending: %s", testMessage2.c_str());
    auto startS2C = std::chrono::steady_clock::now();
    bool sentS2C = server.send(testMessage2);
    plotly::logInfo("Server send returned: %s", sentS2C ? "true" : "false");
    EXPECT_TRUE(sentS2C) << "Server message send";
    {
      std::unique_lock<std::mutex> lock(mtx);
      bool ok =
          cv.wait_for(lock, 2000ms, [&]() -> bool { return clientReceived; });
      plotly::logInfo("Client received flag: %s", ok ? "true" : "false");
      EXPECT_TRUE(ok) << "Client message reception";
      if (ok) {
        plotly::logInfo("Client received content: %s",
                        receivedClientMessage.c_str());
        EXPECT_EQ(receivedClientMessage, testMessage2)
            << "Client message content verification";
      }
    }
    auto endS2C = std::chrono::steady_clock::now();
    auto durS2C =
        std::chrono::duration_cast<std::chrono::microseconds>(endS2C - startS2C)
            .count();
    plotly::logInfo("Iteration %d server->client duration: %lld us", i,
                    static_cast<long long>(durS2C));

    // Unregister client callback for this iteration
    client.unregisterCallback("test_handler");
  }

  // Client and server will automatically stop when going out of scope
}

TEST(WebSocketTest, LargeDataTransfer) {
  WebsocketServer server;
  WebsocketClient client;

  // Start server
  plotly::logInfo("Starting server for large data test on %s:%d", "127.0.0.1",
                  0);
  ASSERT_TRUE(server.serve("127.0.0.1", 0))
      << "Server startup for large data test";

  int port = server.getPort();
  plotly::logInfo("Large data test server listening on port: %d", port);
  ASSERT_GT(port, 0) << "Port assignment for large data test";

  // Connect client
  std::string serverUrl = "ws://127.0.0.1:" + std::to_string(port);
  plotly::logInfo("Client connecting to %s", serverUrl.c_str());
  ASSERT_TRUE(client.connect(serverUrl))
      << "Client connection for large data test";
  ASSERT_TRUE(client.waitConnection(1000ms))
      << "Client connection establishment for large data test";

  // Synchronization primitives
  std::mutex mtx;
  std::condition_variable cv;
  bool serverReceived = false;
  bool clientReceived = false;
  std::string receivedServerMessage;
  std::string receivedClientMessage;

  // Register message handlers
  server.registerCallback("large_data_handler",
                          [&](const std::string_view &msg) -> void {
                            std::scoped_lock lock(mtx);
                            receivedServerMessage = std::string(msg);
                            serverReceived = true;
                            cv.notify_all();
                          });

  client.registerCallback("large_data_handler",
                          [&](const std::string_view &msg) -> void {
                            std::scoped_lock lock(mtx);
                            receivedClientMessage = std::string(msg);
                            clientReceived = true;
                            cv.notify_all();
                          });

  // Test different message sizes
  std::vector<size_t> testSizes = {
      1024,                                // 1KB
      static_cast<size_t>(10 * 1024),      // 10KB
      static_cast<size_t>(100 * 1024),     // 100KB
      static_cast<size_t>(1024 * 1024),    // 1MB
      static_cast<size_t>(5 * 1024 * 1024) // 5MB
  };

  for (size_t messageSize : testSizes) {
    plotly::logInfo("Testing message size: %zu bytes", messageSize);

    // Generate large test data
    std::string largeData;
    largeData.reserve(messageSize);

    // Create pattern that can be verified
    for (size_t i = 0; i < messageSize; ++i) {
      largeData += static_cast<char>('A' + (i % 26));
    }

    // Test Client -> Server
    {
      std::unique_lock<std::mutex> lock(mtx);
      serverReceived = false;
    }

    auto startTime = std::chrono::steady_clock::now();
    bool sentC2S = client.send(largeData);
    EXPECT_TRUE(sentC2S) << "Client large message send (" << messageSize
                         << " bytes)";

    {
      std::unique_lock<std::mutex> lock(mtx);
      bool received =
          cv.wait_for(lock, 10000ms, [&]() -> bool { return serverReceived; });
      EXPECT_TRUE(received)
          << "Server large message reception (" << messageSize << " bytes)";

      if (received) {
        EXPECT_EQ(receivedServerMessage.size(), messageSize)
            << "Server received message size verification";
        EXPECT_EQ(receivedServerMessage, largeData)
            << "Server received message content verification";
      }
    }

    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);
    plotly::logInfo("Client->Server %zu bytes: %lld ms", messageSize,
                    static_cast<long long>(duration.count()));

    // Test Server -> Client
    {
      std::unique_lock<std::mutex> lock(mtx);
      clientReceived = false;
    }

    startTime = std::chrono::steady_clock::now();
    bool sentS2C = server.send(largeData);
    EXPECT_TRUE(sentS2C) << "Server large message send (" << messageSize
                         << " bytes)";

    {
      std::unique_lock<std::mutex> lock(mtx);
      bool received =
          cv.wait_for(lock, 10000ms, [&]() -> bool { return clientReceived; });
      EXPECT_TRUE(received)
          << "Client large message reception (" << messageSize << " bytes)";

      if (received) {
        EXPECT_EQ(receivedClientMessage.size(), messageSize)
            << "Client received message size verification";
        EXPECT_EQ(receivedClientMessage, largeData)
            << "Client received message content verification";
      }
    }

    endTime = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime -
                                                                     startTime);
    plotly::logInfo("Server->Client %zu bytes: %lld ms", messageSize,
                    static_cast<long long>(duration.count()));
  }

  // Client and server will automatically stop when going out of scope
}
