
/**
 * @file gallery_hello_world.cpp
 * @brief Hello World - Basic Plotly.cpp Example
 * @author plotly.cpp contributors
 * @date 2025
 *
 * @example gallery_hello_world.cpp
 *
 * # Hello World Example
 *
 * This example demonstrates the most basic usage of the plotly.cpp library by
 * creating a simple scatter plot with both line and marker visualization.
 *
 * ## What You'll Learn
 * - Basic plot creation with scatter traces
 * - Layout configuration with titles
 * - Interactive browser display vs headless image export
 * - Command line argument parsing for different execution modes
 *
 * ## Sample Output
 * The example creates a scatter plot with the following data points:
 * - X coordinates: [1, 2, 3, 4, 5]
 * - Y coordinates: [1, 4, 2, 8, 5]
 *
 * The resulting plot shows both line connections and individual markers,
 * with the title "Hello World!" displayed at the top.
 *
 * @image html hello-world.png "Hello World Example Output"
 *
 * @see plotly::Figure For the main plotting interface
 * @see parseGalleryArgs() For command line argument handling
 */

#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include <vector>

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments for headless/interactive mode
  auto args = parseGalleryArgs(argc, argv);

  // Create a new figure instance
  plotly::Figure fig;
  fig.openBrowser(args.headless); // Configure browser opening behavior

  // Sample data points for demonstration
  std::vector<double> x = {1, 2, 3, 4, 5}; ///< X-axis coordinates
  std::vector<double> y = {1, 4, 2, 8, 5}; ///< Y-axis coordinates

  // Create trace object with Plotly.js-compatible structure
  plotly::Object trace = {
      {"x", x}, {"y", y}, {"type", "scatter"}, {"mode", "lines+markers"}};

  // Create layout object with title
  plotly::Object layout = {{"title", {{"text", "Hello World!"}}}};

  // Generate the plot with trace data and layout configuration
  fig.newPlot(plotly::Array{trace}, layout);

  // Handle output based on execution mode
  if (!args.headless) {
    fig.waitClose(); // Wait until browser is closed
  } else {
    plotly::Object imageOpts = {
        {"format", "png"},          ///< Image format (png, jpg, svg, pdf)
        {"width", 800},             ///< Image width in pixels
        {"height", 600},            ///< Image height in pixels
        {"filename", "hello_world"} ///< Output filename (without extension)
    };
    fig.downloadImage(imageOpts);
  }

  return 0; ///< Success exit code
}
