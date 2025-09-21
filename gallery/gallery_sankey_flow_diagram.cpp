
#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include <string>
#include <vector>

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);

  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Energy flow data - from sources to end uses
  std::vector<std::string> nodeLabels = {
      // Energy Sources (0-4)
      "Coal", "Natural Gas", "Nuclear", "Hydro", "Solar/Wind",
      // Energy Sectors (5-8)
      "Electricity Generation", "Industrial", "Transportation", "Residential",
      // End Uses (9-12)
      "Lighting", "Heating", "Manufacturing", "Transportation"};

  std::vector<std::string> nodeColors = {
      // Sources - darker colors
      "#8B4513", "#4169E1", "#FF4500", "#1E90FF", "#32CD32",
      // Sectors - medium colors
      "#FFD700", "#FF6347", "#9370DB", "#20B2AA",
      // End uses - lighter colors
      "#FFFF99", "#FFA07A", "#DDA0DD", "#98FB98"};

  // Define flow connections: [source_index, target_index, value]
  std::vector<int> sources = {
      // Sources to sectors
      0, 1, 2, 3, 4, // To Electricity Generation
      1, 0,          // To Industrial
      1,             // To Transportation
      1, 5,          // To Residential
      // Sectors to end uses
      5, 5, 8, // Electricity to Lighting, Heating, Residential
      6,       // Industrial to Manufacturing
      7,       // Transportation to Transportation
      8, 8     // Residential to Heating, Lighting
  };

  std::vector<int> targets = {
      // Sources to sectors
      5, 5, 5, 5, 5, // Coal, Gas, Nuclear, Hydro, Solar/Wind to Electricity
      6, 6,          // Gas, Coal to Industrial
      7,             // Gas to Transportation
      8, 8,          // Gas, Electricity to Residential
      // Sectors to end uses
      9, 10, 10, // Electricity to Lighting, Heating, Heating
      11,        // Industrial to Manufacturing
      12,        // Transportation to Transportation
      10, 9      // Residential to Heating, Lighting
  };

  std::vector<double> values = {
      // Sources to sectors (units: arbitrary energy units)
      35, 40, 20, 15, 10, // To Electricity Generation
      25, 15,             // To Industrial
      30,                 // To Transportation
      20, 50,             // To Residential
      // Sectors to end uses
      40, 30, 20, // From Electricity
      40,         // From Industrial
      30,         // From Transportation
      35, 35      // From Residential
  };

  // Create link colors with transparency
  std::vector<std::string> linkColors;
  for (int source : sources) {
    // Use source node color with transparency
    const std::string &baseColor = nodeColors[source];
    linkColors.push_back(baseColor + "80"); // Add 50% transparency
  }

  // Create Sankey trace
  plotly::Object trace = {
      {"type", "sankey"},
      {"orientation", "h"},
      {"node",
       {{"pad", 15},
        {"thickness", 20},
        {"line", {{"color", "black"}, {"width", 0.5}}},
        {"label", nodeLabels},
        {"color", nodeColors},
        {"hovertemplate", "%{label}<br>Total Flow: %{value}<extra></extra>"}}},
      {"link",
       {{"source", sources},
        {"target", targets},
        {"value", values},
        {"color", linkColors},
        {"hovertemplate",
         "Flow: %{source.label} → %{target.label}<br>" +
             std::string("Amount: %{value} units<extra></extra>")}}}};

  // Create layout
  plotly::Object layout = {
      {"title",
       {{"text", "Energy Flow Diagram<br>" +
                     std::string("<sub>From Sources to End Uses</sub>")},
        {"font", {{"size", 18}}}}},
      {"width", 1100},
      {"height", 700},
      {"margin", {{"l", 50}, {"r", 50}, {"t", 80}, {"b", 50}}},
      {"annotations",
       {{{"text", "Flow thickness represents energy quantity"},
         {"x", 0.5},
         {"y", -0.08},
         {"xref", "paper"},
         {"yref", "paper"},
         {"showarrow", false},
         {"font", {{"size", 12}}}}}}};

  // Create the plot
  std::vector<plotly::Object> data = {trace};
  fig.newPlot(data, layout);

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 1100},
                                {"height", 700},
                                {"filename", "sankey_flow_diagram"}};
    fig.downloadImage(imageOpts);
  }

  return 0;
}
