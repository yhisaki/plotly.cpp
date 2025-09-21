
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
