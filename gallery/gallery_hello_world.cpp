
#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include <vector>

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);
  plotly::Figure fig;
  fig.openBrowser(args.headless); // Open browser explicitly

  std::vector<double> x = {1, 2, 3, 4, 5};
  std::vector<double> y = {1, 4, 2, 8, 5};

  // Same trace structure as Plotly.js!
  plotly::Object trace = {
      {"x", x}, {"y", y}, {"type", "scatter"}, {"mode", "lines+markers"}};
  plotly::Object layout = {{"title", {{"text", "Hello World!"}}}};
  fig.newPlot(plotly::Array{trace}, layout);

  if (!args.headless) {
    fig.waitClose(); // Wait until browser is closed
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 800},
                                {"height", 600},
                                {"filename", "hello_world"}};
    fig.downloadImage(imageOpts);
  }
  return 0;
}
