#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include <cmath>
#include <utility>
#include <vector>

/// Creates a star shape with the given parameters
/// @param cx Center X coordinate
/// @param cy Center Y coordinate
/// @param rOuter Outer radius (tips of star)
/// @param rInner Inner radius (valleys of star)
auto createStarShape(double cx, double cy, double rOuter, double rInner)
    -> std::pair<std::vector<double>, std::vector<double>> {
  std::vector<double> x;
  std::vector<double> y;
  const double angle = M_PI / 5.0; // 36 degrees

  for (int i = 0; i < 10; i++) {
    double r = (i % 2 == 0) ? rOuter : rInner;
    double theta = (i * angle) + M_PI / 2.0; // start from top
    double xPoint = cx + (r * std::cos(theta));
    double yPoint = cy + (r * std::sin(theta));
    x.push_back(xPoint);
    y.push_back(yPoint);
  }

  // Close the path by adding the first point again
  x.push_back(x[0]);
  y.push_back(y[0]);

  return std::make_pair(x, y);
}

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);

  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Create star shape with center at (0,0), outer radius 1, inner radius 0.4
  auto [x, y] = createStarShape(0.0, 0.0, 1.0, 0.4);

  fig.newPlot({{{"x", x},
                {"y", y},
                {"type", "scatter"},
                {"mode", "lines+markers"},
                {"line", {{"shape", "linear"}, {"color", "gold"}}},
                {"marker", {{"color", "red"}, {"size", 8}}}}},
              {{"title", {{"text", "Star Shape Plot"}}},
               {"xaxis", {{"scaleanchor", "y"}, {"range", {-1.5, 1.5}}}},
               {"yaxis", {{"range", {-1.5, 1.5}}}},
               {"showlegend", false}});

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 800},
                                {"height", 600},
                                {"filename", "star"}};
    fig.downloadImage(imageOpts);
  }

  return 0;
}
