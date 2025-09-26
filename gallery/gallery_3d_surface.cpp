
/**
 * @file gallery_3d_surface.cpp
 * @brief 3D Surface Plot - Three-Dimensional Visualization Example
 * @author plotly.cpp contributors
 * @date 2025
 *
 * @example gallery_3d_surface.cpp
 *
 * # 3D Surface Plot Example
 *
 * This example demonstrates how to create three-dimensional surface plots using
 * plotly.cpp, visualizing mathematical functions in 3D space.
 *
 * ## What You'll Learn
 * - Creating 3D surface plots from mathematical functions
 * - Generating 2D grid data for surface visualization
 * - Working with colorscales and surface styling
 * - Mathematical function visualization techniques
 *
 * ## Sample Output
 * The example creates a 3D surface plot of the function:
 * ```
 * z = sin(√(x² + y²))
 * ```
 * This creates a ripple effect emanating from the center, visualized with
 * the Viridis colorscale for better depth perception.
 *
 * @image html 3d_surface.gif "3D Surface Plot Example Output"
 *
 * @see plotly::Figure For the main plotting interface
 */

#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include <cmath>
#include <vector>

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);
  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Generate 3D surface data
  int size = 50;
  std::vector<std::vector<double>> z(size, std::vector<double>(size));
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      double x = (i - size / 2.0) * 0.2;
      double y = (j - size / 2.0) * 0.2;
      z[i][j] = std::sin(std::sqrt(x * x + y * y));
    }
  }

  plotly::Object trace = {
      {"z", z}, {"type", "surface"}, {"colorscale", "Viridis"}};

  fig.newPlot(plotly::Array{trace}, plotly::Object{});

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 800},
                                {"height", 600},
                                {"filename", "3d_surface"}};
    fig.downloadImage(imageOpts);
  }
  return 0;
}
