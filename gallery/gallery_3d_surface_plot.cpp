
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

  // Create grid for 3D surface
  const int numPoints = 50;
  auto x = linspace(-3.0, 3.0, numPoints);
  auto y = linspace(-3.0, 3.0, numPoints);

  // Create Z matrix for surface
  std::vector<std::vector<double>> z;
  z.reserve(numPoints);

  for (int i = 0; i < numPoints; i++) {
    std::vector<double> row;
    row.reserve(numPoints);
    for (int j = 0; j < numPoints; j++) {
      // Mathematical function: sin(sqrt(x^2 + y^2)) - ripple effect
      double xi = x[i];
      double yj = y[j];
      double r = std::sqrt(xi * xi + yj * yj);
      double zval = std::sin(r) * std::exp(-r / 6.0);
      row.push_back(zval);
    }
    z.push_back(row);
  }

  // Create 3D surface trace
  plotly::Object trace = {{"type", "surface"},
                          {"x", x},
                          {"y", y},
                          {"z", z},
                          {"colorscale", "Viridis"},
                          {"showscale", true},
                          {"opacity", 0.9}};

  // Create layout with 3D scene configuration
  plotly::Object layout = {
      {"title", {{"text", "3D Surface Plot: sin(√(x² + y²)) × e^(-r/6)"}}},
      {"scene",
       {{"xaxis", {{"title", "X axis"}, {"showgrid", true}}},
        {"yaxis", {{"title", "Y axis"}, {"showgrid", true}}},
        {"zaxis", {{"title", "Z axis"}, {"showgrid", true}}},
        {"camera", {{"eye", {{"x", 1.5}, {"y", 1.5}, {"z", 1.5}}}}}}},
      {"width", 900},
      {"height", 700}};

  // Create the plot
  std::vector<plotly::Object> data = {trace};
  fig.newPlot(data, layout);

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 900},
                                {"height", 700},
                                {"filename", "3d_surface_plot"}};
    fig.downloadImage(imageOpts);
  }

  return 0;
}
