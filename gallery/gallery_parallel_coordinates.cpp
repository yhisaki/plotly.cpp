
/**
 * @file gallery_parallel_coordinates.cpp
 * @brief Parallel Coordinates Plot with Multi-Dimensional Data
 * @author plotly.cpp contributors
 * @date 2025
 *
 * @example gallery_parallel_coordinates.cpp
 *
 * This gallery example demonstrates creating a parallel coordinates plot using
 * Plotly.cpp. Parallel coordinates are effective for visualizing
 * multi-dimensional data by representing each data point as a line connecting
 * values across multiple parallel axes.
 *
 * Features demonstrated:
 * - Multi-dimensional data visualization using parallel coordinates
 * - Custom dimension configuration with ranges and constraints
 * - Interactive brushing and filtering capabilities
 * - Custom tick values and labels for categorical data
 * - Constraint ranges for data filtering on specific dimensions
 * - Color coding of parallel lines for pattern identification
 *
 * The plot displays four dimensions (A, B, C, D) with different configurations:
 * - Dimension A: Numeric range with constraint filtering
 * - Dimension B: Custom tick positions for specific value highlighting
 * - Dimension C: Custom text labels replacing numeric values
 * - Dimension D: Standard numeric range without constraints
 *
 * @image html parallel_coordinates.png "Multi-Dimensional Parallel Coordinates
 * Plot"
 *
 */

#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include <vector>

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);

  // Create a plotly figure
  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Create the parallel coordinates trace
  plotly::Object trace = {
      {"type", "parcoords"},
      {"line", {{"color", "blue"}}},
      {"dimensions",
       {{{"range", {1, 5}},
         {"constraintrange", {1, 2}},
         {"label", "A"},
         {"values", {1, 4}}},
        {{"range", {1, 5}},
         {"label", "B"},
         {"values", {3, 1.5}},
         {"tickvals", {1.5, 3, 4.5}}},
        {{"range", {1, 5}},
         {"label", "C"},
         {"values", {2, 4}},
         {"tickvals", {1, 2, 4, 5}},
         {"ticktext", {"text 1", "text 2", "text 4", "text 5"}}},
        {{"range", {1, 5}}, {"label", "D"}, {"values", {4, 2}}}}}};

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
                                {"filename", "parallel_coordinates"}};
    fig.downloadImage(imageOpts);
  }

  return 0;
}
