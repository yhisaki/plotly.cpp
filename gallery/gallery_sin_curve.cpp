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
