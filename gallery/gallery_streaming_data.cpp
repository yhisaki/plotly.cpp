
/**
 * @file gallery_streaming_data.cpp
 * @brief Real-Time Data Streaming and Animation
 *
 * This gallery example demonstrates real-time data streaming capabilities using
 * Plotly.cpp's extendTraces functionality. It creates a live-updating sine wave
 * that streams data points continuously, showcasing dynamic visualization
 * techniques for time-series data.
 *
 * Features demonstrated:
 * - Real-time data streaming using extendTraces API
 * - Live plot animation with controlled frame rate
 * - Rolling window data display with maximum point limits
 * - Thread-safe plot updates in real-time loops
 * - Continuous mathematical function generation (sine wave)
 * - Browser connection monitoring for graceful termination
 * - High-frequency data updates (20 Hz) for smooth animation
 *
 * Technical concepts:
 * - Plotly.js extendTraces equivalent implementation in C++
 * - Time-series data streaming with automatic old data removal
 * - Multi-threaded visualization with controlled timing
 * - Memory-efficient streaming with bounded data windows
 * - Real-time mathematical function evaluation
 *
 * The streaming visualization creates a smooth animated sine wave that
 * continuously scrolls across the screen, demonstrating the capability
 * to handle live data feeds and sensor readings in real-time applications.
 *
 * @image html streaming_data.gif "Real-Time Streaming Data Animation"
 *
 */

#include "plotly/plotly.hpp"
#include <chrono>
#include <cmath>
#include <thread>
#include <vector>

auto main() -> int {
  plotly::Figure fig;
  fig.openBrowser();
  std::vector<double> x, y;

  // Create initial empty trace (same structure as Plotly.js)
  plotly::Object trace = {
      {"x", x}, {"y", y}, {"type", "scatter"}, {"mode", "lines"}};
  std::vector<plotly::Object> data = {trace};
  fig.newPlot(data);

  // Stream data in real-time using Plotly.js extendTraces pattern
  for (double t = 0; t < 100 && fig.isOpen(); t += 0.1) {
    // Direct equivalent of Plotly.extendTraces()
    fig.extendTraces({{"x", {std::vector<double>{t}}},
                      {"y", {std::vector<double>{std::sin(t)}}}},
                     {0}, 100 // trace index 0, max 100 points
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  return 0;
}
