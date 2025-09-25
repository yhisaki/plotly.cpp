/**
 * @file gallery_sin_curve.cpp
 * @brief Simple Sine Wave Curve Visualization
 * @author plotly.cpp contributors
 * @date 2025
 *
 * @example gallery_sin_curve.cpp
 * This gallery example demonstrates creating a basic sine wave plot using
 * Plotly.cpp. It showcases fundamental plotting capabilities with mathematical
 * function visualization over a continuous domain.
 *
 * Features demonstrated:
 * - Mathematical function plotting using scatter trace type
 * - Continuous curve generation with linspace utility
 * - Basic trigonometric function visualization (sine wave)
 * - Simple plot creation with minimal configuration
 * - Standard domain range (-2π to 2π) for complete wave cycles
 * - High-resolution curve with 200 data points for smooth visualization
 *
 * Mathematical concepts:
 * - Sine function: y = sin(x)
 * - Domain: [-2π, 2π] covering two complete oscillation cycles
 * - Amplitude: 1.0 (standard unit amplitude)
 * - Period: 2π (standard sine wave period)
 *
 * This example serves as an introduction to basic mathematical function
 * plotting and demonstrates the ease of creating smooth curves with
 * Plotly.cpp's scatter trace functionality.
 *
 * @image html sin_curve.png "Sine Wave Mathematical Function"
 *
 */

#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include "utils/linspace.hpp"
#include <cmath>
#include <vector>

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);

  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Generate x values from -2π to 2π
  auto x = linspace(-2 * M_PI, 2 * M_PI, 200);

  // Calculate sin values
  std::vector<double> y;
  y.reserve(x.size());
  for (const auto &xi : x) {
    y.push_back(std::sin(xi));
  }

  // Create scatter trace for sin curve
  plotly::Object trace = {
      {"x", x},
      {"y", y},
      {"type", "scatter"},
  };

  // Create the plot
  std::vector<plotly::Object> data = {trace};
  fig.newPlot(data);

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 800},
                                {"height", 600},
                                {"filename", "sin_curve"}};
    fig.downloadImage(imageOpts);
  }

  return 0;
}
