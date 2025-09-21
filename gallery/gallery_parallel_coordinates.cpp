
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
