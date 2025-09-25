#ifndef PLOTLY_DETAILS_HTTP_SERVER_HPP
#define PLOTLY_DETAILS_HTTP_SERVER_HPP

#include "httplib.h"
#include <filesystem>
#include <string_view>
#include <thread>

/**
 * @file http_server.hpp
 * @brief Simple HTTP server implementation for serving Plotly visualizations
 *
 * This file contains the http_server class which provides functionality to
 * serve HTML content or files from a directory over HTTP. It's primarily used
 * for hosting Plotly visualizations in a local web server.
 */

namespace plotly::detail {

/**
 * @class http_server
 * @brief A lightweight HTTP server for serving Plotly visualizations
 *
 * The http_server class provides functionality to serve either static files
 * from a directory or direct HTML content over HTTP. It's designed to be used
 * for hosting Plotly visualizations locally, allowing them to be viewed in a
 * web browser.
 */
class HttpServer {
private:
  httplib::Server _server;
  int _port = 0;
  std::thread _serverThread;

  void setupCommon();

public:
  /**
   * @brief Construct a server serving files from a directory
   * @param directory Path to the directory containing files to serve
   */
  explicit HttpServer(const std::filesystem::path &directory);

  /**
   * @brief Construct a server serving a single HTML content
   * @param htmlContent The HTML content to serve
   */
  explicit HttpServer(std::string_view htmlContent);

  /**
   * @brief Destructor
   */
  ~HttpServer();

  // Prevent copying
  HttpServer(const HttpServer &) = delete;
  auto operator=(const HttpServer &) -> HttpServer & = delete;

  // Prevent moving (due to thread member)
  HttpServer(HttpServer &&) = delete;
  auto operator=(HttpServer &&) -> HttpServer & = delete;

  /**
   * @brief Start the HTTP server
   *
   * This method starts the HTTP server on a randomly allocated port.
   * The server runs in a background thread.
   */
  void start();

  /**
   * @brief Get the port number the server is running on
   * @return The port number
   */
  [[nodiscard]] auto getPort() const noexcept -> int;

  /**
   * @brief Configure the server to respond with a specific WebSocket port
   * @param wsPort The WebSocket port to report
   *
   * Sets up an endpoint that responds to WebSocket port requests with the
   * specified port number. This is used for WebSocket communication between
   * the browser and the application.
   */
  void setWebsocketPortRequestHandler(int wsPort);

  /**
   * @brief Stop the HTTP server
   *
   * This method stops the HTTP server and joins the server thread.
   */
  void stop();
};

} // namespace plotly::detail

#endif // PLOTLY_DETAILS_HTTP_SERVER_HPP
