#include "plotly/plotly.hpp"
#include <iostream>
#include <vector>

auto main() -> int {
  plotly::Figure fig;
  fig.openBrowser();

  // Create plot
  std::vector<double> x = {1, 2, 3, 4, 5};
  std::vector<double> y = {1, 4, 2, 8, 5};
  plotly::Object trace = {
      {"x", x}, {"y", y}, {"type", "scatter"}, {"mode", "markers"}};
  fig.newPlot(plotly::Array{trace});

  // Register hover event handler
  fig.on("plotly_hover", [&fig](const plotly::Object &event) -> void {
    std::cout << "Hovering over point" << '\n';

    // Get point information
    auto xValue = event["points"][0]["x"];
    auto yValue = event["points"][0]["y"];

    // Add annotation with "hover" text
    plotly::Object annotation = {
        {"x", xValue},
        {"y", yValue},
        {"text", "hover"},
        {"showarrow", false},
        {"yshift", 30},
        {"font", plotly::Object{{"color", "blue"}, {"size", 20}}}};

    plotly::Object layoutUpdate = {{"annotations", plotly::Array{annotation}}};
    fig.relayout(layoutUpdate);
  });

  // Register click event handler
  fig.on("plotly_click", [&fig](const plotly::Object &event) -> void {
    std::cout << "Point clicked: x=" << event["points"][0]["x"]
              << ", y=" << event["points"][0]["y"] << '\n';

    // Get point information
    auto xValue = event["points"][0]["x"];
    auto yValue = event["points"][0]["y"];

    // Add annotation with "click" text
    plotly::Object annotation = {
        {"x", xValue},
        {"y", yValue},
        {"text", "click"},
        {"showarrow", false},
        {"yshift", 30},
        {"font", plotly::Object{{"color", "blue"}, {"size", 20}}}};

    plotly::Object layoutUpdate = {{"annotations", plotly::Array{annotation}}};
    fig.relayout(layoutUpdate);
  });

  fig.waitClose();
  return 0;
}
