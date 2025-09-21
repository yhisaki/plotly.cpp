
#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include <cstddef>
#include <string>
#include <vector>

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);

  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Define performance categories
  std::vector<std::string> categories = {
      "Performance", "Reliability",     "Security",      "Usability",
      "Scalability", "Maintainability", "Documentation", "Support"};

  // Performance data for different products/systems
  std::vector<std::vector<double>> performanceData = {
      {8.5, 9.2, 7.8, 8.9, 7.5, 8.1, 6.8, 8.7}, // Product A
      {7.2, 8.1, 9.5, 7.6, 8.8, 7.9, 8.5, 7.3}, // Product B
      {9.1, 7.8, 8.4, 9.3, 6.9, 9.2, 9.0, 8.2}, // Product C
      {6.8, 8.9, 7.2, 8.5, 9.1, 7.4, 7.8, 9.0}  // Product D
  };

  std::vector<std::string> productNames = {"Product A", "Product B",
                                           "Product C", "Product D"};

  std::vector<std::string> colors = {
      "rgba(255, 0, 0, 0.6)",  // Red
      "rgba(0, 255, 0, 0.6)",  // Green
      "rgba(0, 0, 255, 0.6)",  // Blue
      "rgba(255, 165, 0, 0.6)" // Orange
  };

  std::vector<std::string> lineColors = {"red", "green", "blue", "orange"};

  // Create traces for each product
  std::vector<plotly::Object> traces;

  for (size_t i = 0; i < performanceData.size(); i++) {
    // Close the polygon by adding first point at the end
    std::vector<double> values = performanceData[i];
    std::vector<std::string> theta = categories;

    values.push_back(values[0]);
    theta.push_back(categories[0]);

    plotly::Object trace = {
        {"type", "scatterpolar"},
        {"r", values},
        {"theta", theta},
        {"fill", "toself"},
        {"name", productNames[i]},
        {"line", {{"color", lineColors[i]}, {"width", 3}}},
        {"marker", {{"color", colors[i]}, {"size", 8}}},
        {"fillcolor", colors[i]},
        {"hovertemplate",
         productNames[i] + "<br>%{theta}: %{r:.1f}<extra></extra>"}};
    traces.push_back(trace);
  }

  // Create layout with polar configuration
  plotly::Object layout = {
      {"title",
       {{"text", "Product Performance Comparison<br>" +
                     std::string("<sub>Multi-dimensional Radar Chart</sub>")},
        {"font", {{"size", 18}}}}},
      {"polar",
       {{"radialaxis",
         {{"visible", true},
          {"range", {0, 10}},
          {"tickmode", "linear"},
          {"tick0", 0},
          {"dtick", 2},
          {"showticklabels", true},
          {"tickfont", {{"size", 12}}},
          {"gridcolor", "lightgray"}}},
        {"angularaxis",
         {{"tickfont", {{"size", 14}}},
          {"rotation", 90},
          {"direction", "counterclockwise"}}}}},
      {"width", 800},
      {"height", 700},
      {"showlegend", true},
      {"legend", {{"x", 1.05}, {"y", 1.0}}},
      {"annotations",
       {{{"text", "Scale: 0 (Poor) to 10 (Excellent)"},
         {"x", 0.5},
         {"y", -0.1},
         {"xref", "paper"},
         {"yref", "paper"},
         {"showarrow", false},
         {"font", {{"size", 12}}}}}}};

  // Create the plot
  fig.newPlot(traces, layout);

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 800},
                                {"height", 700},
                                {"filename", "polar_radar_chart"}};
    fig.downloadImage(imageOpts);
  }

  return 0;
}
