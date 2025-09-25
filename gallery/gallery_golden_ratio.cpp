
/**
 * @file gallery_golden_ratio.cpp
 * @brief Golden Ratio Spiral - Mathematical Beauty in Visualization
 * @author plotly.cpp contributors
 * @date 2025
 *
 * @example gallery_golden_ratio.cpp
 *
 * # Golden Ratio Spiral Example
 *
 * This example demonstrates the mathematical beauty of the golden ratio by
 * creating a logarithmic spiral overlaid on a heatmap with proportionally sized
 * rectangles. It showcases the mathematical relationship between the golden
 * ratio constant (φ ≈ 1.618) and natural spiral patterns found throughout
 * nature.
 *
 * ## What You'll Learn
 * - Mathematical visualization of the golden ratio and logarithmic spirals
 * - Complex mathematical coordinate transformations and scaling
 * - Combining multiple plot types (scatter + heatmap) in a single visualization
 * - Using mathematical constants and exponential functions for organic curves
 * - Creating proportionally sized geometric elements based on mathematical
 * ratios
 * - Advanced layout customization for mathematical and scientific presentations
 * - Working with linspace utility for generating smooth parametric curves
 *
 * ## Sample Output
 * The example creates an artistic mathematical visualization featuring:
 * - White logarithmic spiral curve generated using exponential decay
 * - Colorful heatmap background with rectangles sized according to golden ratio
 * - Viridis color scale providing smooth color transitions
 * - Square aspect ratio (700x700) emphasizing the geometric relationships
 * - Centered composition showing the mathematical harmony of proportions
 *
 * @image html golden_ratio.png "Golden Ratio Spiral with Proportional
 * Rectangles"
 *
 * @see linspace() For generating smooth parametric coordinates
 * @see parseGalleryArgs() For command line argument handling
 */

#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include "utils/linspace.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

// Helper functions for min/max of arrays
auto getMaxOfArray(const std::vector<double> &numArray) -> double {
  return *std::max_element(numArray.begin(), numArray.end());
}

auto getMinOfArray(const std::vector<double> &numArray) -> double {
  return *std::min_element(numArray.begin(), numArray.end());
}

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);

  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Number of spiral loops
  int nspiral = 2;

  // Angle
  auto th = linspace((-M_PI) / 13, (2 * M_PI * nspiral), 1000);

  // Empty Value Containers
  std::vector<double> xValues;
  std::vector<double> yValues;

  // Spiral generation
  for (double i : th) {
    double a = 1.120529;
    double b = 0.306349;
    double r = a * std::exp((-b) * i);
    double xResult = (r * std::cos(i));
    double yResult = (r * std::sin(i));
    xValues.push_back(xResult);
    yValues.push_back(yResult);
  }

  // Shift spiral north so that it is centered
  double yShift = (1.6 - (getMaxOfArray(yValues) - getMinOfArray(yValues))) / 2;

  // Transform spiral coordinates for plotting
  std::vector<double> spiralX;
  std::vector<double> spiralY;
  for (size_t i = 0; i < xValues.size(); i++) {
    spiralX.push_back(-(xValues[i]) + xValues[0]);
    spiralY.push_back(yValues[i] - yValues[0] + yShift);
  }

  // Build the rectangles as a heatmap and specify the edges of the heatmap
  // squares
  // Golden ratio constant (compatible with C++17)
  double phi = (1.0 + std::sqrt(5.0)) / 2.0;
  std::vector<double> xe = {0, 1, (1 + (1 / std::pow(phi, 4))),
                            1 + (1 / std::pow(phi, 3)), phi};
  std::vector<double> ye = {0, (1 / std::pow(phi, 3)),
                            (1 / std::pow(phi, 3)) + (1 / std::pow(phi, 4)),
                            (1 / std::pow(phi, 2)), 1};

  // Transform ye coordinates with yShift
  std::vector<double> yeShifted;
  yeShifted.reserve(ye.size());
  for (double yi : ye) {
    yeShifted.push_back(yi + yShift);
  }

  // Z values for heatmap
  std::vector<std::vector<int>> zValues = {
      {13, 3, 3, 5}, {13, 2, 1, 5}, {13, 10, 11, 12}, {13, 8, 8, 8}};

  // Create spiral trace
  plotly::Object spiralTrace = {{"x", spiralX},
                                {"y", spiralY},
                                {"type", "scatter"},
                                {"line", {{"color", "white"}, {"width", 3}}}};

  // Create heatmap trace
  plotly::Object heatmapTrace = {{"x", xe},
                                 {"y", yeShifted},
                                 {"z", zValues},
                                 {"type", "heatmap"},
                                 {"colorscale", "Viridis"}};

  // Axis template
  plotly::Object axisTemplate = {{"range", std::vector<double>{0, 1.6}},
                                 {"autorange", false},
                                 {"showgrid", false},
                                 {"zeroline", false},
                                 {"linecolor", "black"},
                                 {"showticklabels", false},
                                 {"ticks", ""}};

  // Layout configuration
  plotly::Object layout = {
      {"title", {{"text", "Heatmap with Unequal Block Sizes"}}},
      {"margin", {{"t", 200}, {"r", 200}, {"b", 200}, {"l", 200}}},
      {"xaxis", axisTemplate},
      {"yaxis", axisTemplate},
      {"showlegend", false},
      {"width", 700},
      {"height", 700},
      {"autosize", false}};

  // Create the plot with both traces
  std::vector<plotly::Object> data = {spiralTrace, heatmapTrace};

  fig.newPlot(data, layout);

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 700},
                                {"height", 700},
                                {"filename", "golden_ratio"}};
    fig.downloadImage(imageOpts);
  }

  return 0;
}
