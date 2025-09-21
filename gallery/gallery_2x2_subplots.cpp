#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include <cmath>
#include <vector>

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);
  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Generate x values for all traces
  std::vector<double> x;
  for (double t = 0; t < 4 * M_PI; t += 0.1) {
    x.push_back(t);
  }

  // Generate y values for each trace with different phases
  std::vector<double> y1, y2, y3, y4;
  for (double t : x) {
    y1.push_back(std::sin(t));                // Phase 0
    y2.push_back(std::sin(t + M_PI / 4));     // Phase π/4
    y3.push_back(std::sin(t + M_PI / 2));     // Phase π/2
    y4.push_back(std::sin(t + 3 * M_PI / 4)); // Phase 3π/4
  }

  plotly::Object trace1 = {{"x", x},
                           {"y", y1},
                           {"type", "scatter"},
                           {"mode", "lines"},
                           {"name", "sin(t)"}};

  plotly::Object trace2 = {{"x", x},
                           {"y", y2},
                           {"xaxis", "x2"},
                           {"yaxis", "y2"},
                           {"type", "scatter"},
                           {"mode", "lines"},
                           {"name", "sin(t + π/4)"}};

  plotly::Object trace3 = {{"x", x},
                           {"y", y3},
                           {"xaxis", "x3"},
                           {"yaxis", "y3"},
                           {"type", "scatter"},
                           {"mode", "lines"},
                           {"name", "sin(t + π/2)"}};

  plotly::Object trace4 = {{"x", x},
                           {"y", y4},
                           {"xaxis", "x4"},
                           {"yaxis", "y4"},
                           {"type", "scatter"},
                           {"mode", "lines"},
                           {"name", "sin(t + 3π/4)"}};

  plotly::Array data = {trace1, trace2, trace3, trace4};

  plotly::Object layout = {
      {"title", {{"text", "2x2 Subplot Grid - Phase-shifted Sine Waves"}}},
      {"grid", {{"rows", 2}, {"columns", 2}, {"pattern", "independent"}}},
      {"showlegend", false}};

  fig.newPlot(data, layout);

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 800},
                                {"height", 600},
                                {"filename", "2x2_subplots"}};
    fig.downloadImage(imageOpts);
  }
  return 0;
}
