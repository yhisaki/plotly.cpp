#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include <cmath>
#include <vector>

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);

  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Prepare data - equivalent to matplotlib example
  int n = 5000;
  std::vector<double> x(n), y(n), z(n), w(n, 2.0);

  for (int i = 0; i < n; ++i) {
    x[i] = i * i;
    y[i] = std::sin(2 * M_PI * i / 360.0);
    z[i] = std::log(i + 1); // +1 to avoid log(0)
  }

  // Create traces equivalent to matplotlib plots

  // First trace: plot(x, y) - automatically colored line
  plotly::Object trace1 = {{"x", x},
                           {"y", y},
                           {"type", "scatter"},
                           {"mode", "lines"},
                           {"name", "sin(2πi/360)"}};

  // Second trace: plot(x, w, "r--") - red dashed line
  plotly::Object trace2 = {
      {"x", x},
      {"y", w},
      {"type", "scatter"},
      {"mode", "lines"},
      {"line", plotly::Object{{"color", "red"}, {"dash", "dash"}}},
      {"name", "constant line (y=2)"}};

  // Third trace: named_plot("log(x)", x, z) - named line for legend
  plotly::Object trace3 = {{"x", x},
                           {"y", z},
                           {"type", "scatter"},
                           {"mode", "lines"},
                           {"name", "log(x)"}};

  // Layout configuration equivalent to matplotlib settings
  plotly::Object layout = {
      {"title", {{"text", "Sample figure"}}},
      {"xaxis",
       {
           {"title", {{"text", "X values"}}},
           {"range", plotly::Array{0, 1000000}} // xlim(0, 1000*1000)
       }},
      {"yaxis", {{"title", {{"text", "Y values"}}}}},
      {"showlegend", true}, // Enable legend
      {"width", 1200},      // Set figure size
      {"height", 780}};

  // Create the plot with all traces
  std::vector<plotly::Object> data = {trace1, trace2, trace3};
  fig.newPlot(data, layout);

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save the image equivalent to plt::save("./basic.png")
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 1200},
                                {"height", 780},
                                {"filename", "multi_trace_styling"}};
    fig.downloadImage(imageOpts);
  }

  return 0;
}
