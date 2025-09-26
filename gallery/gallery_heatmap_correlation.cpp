
/**
 * @file gallery_heatmap_correlation.cpp
 * @brief Business Metrics Correlation Matrix Heatmap
 * @author plotly.cpp contributors
 * @date 2025
 *
 * @example gallery_heatmap_correlation.cpp
 *
 * This gallery example demonstrates creating an interactive correlation matrix
 heatmap
 * using Plotly.cpp. It generates realistic business performance data across
 multiple
 * metrics and displays their correlations using color coding and text
 annotations.
 *
 * Features demonstrated:
 * - Symmetric correlation matrix generation with realistic business
 relationships
 * - Heatmap visualization with custom colorscale (RdBu for diverging data)
 * - Text annotations showing correlation coefficients on each cell
 * - Custom colorbar with centered zero point for proper correlation display
 * - Responsive layout with rotated axis labels and proper margins
 *
 * The visualization helps identify relationships between different business
 metrics
 * such as revenue, profit, marketing spend, and customer satisfaction.
 *
 * @image html heatmap_correlation.png "Business Metrics Correlation Matrix"
 *

 * @date 2024
 */

#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);

  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Define variable names
  std::vector<std::string> variables = {"Revenue",     "Profit",
                                        "Marketing",   "R&D",
                                        "Employees",   "Customer_Satisfaction",
                                        "Market_Share"};
  const size_t n = variables.size();

  // Generate realistic correlation matrix
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> correlationDist(-0.8, 0.9);

  std::vector<std::vector<double>> correlationMatrix(n, std::vector<double>(n));

  // Fill correlation matrix (symmetric)
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      if (i == j) {
        correlationMatrix[i][j] = 1.0; // Perfect self-correlation
      } else if (i < j) {
        // Generate correlation with some business logic
        double corr = correlationDist(gen);

        // Add some realistic correlations
        if ((variables[i] == "Revenue" && variables[j] == "Profit") ||
            (variables[i] == "Marketing" && variables[j] == "Market_Share") ||
            (variables[i] == "R&D" &&
             variables[j] == "Customer_Satisfaction")) {
          corr = std::abs(corr) * 0.8 + 0.2; // Strong positive correlation
        }

        correlationMatrix[i][j] = corr;
        correlationMatrix[j][i] = corr; // Symmetric
      }
    }
  }

  // Create text annotations for correlation values
  std::vector<std::vector<std::string>> textMatrix(n,
                                                   std::vector<std::string>(n));
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      std::array<char, 10> buffer;
      std::snprintf(buffer.data(), buffer.size(), "%.2f",
                    correlationMatrix[i][j]);
      textMatrix[i][j] = buffer.data();
    }
  }

  // Create heatmap trace
  plotly::Object trace = {
      {"type", "heatmap"},
      {"x", variables},
      {"y", variables},
      {"z", correlationMatrix},
      {"text", textMatrix},
      {"texttemplate", "%{text}"},
      {"textfont", {{"size", 12}, {"color", "white"}}},
      {"colorscale", "RdBu"},
      {"zmid", 0.0},
      {"showscale", true},
      {"colorbar",
       {{"title", "Correlation Coefficient"}, {"titleside", "right"}}}};

  // Create layout
  plotly::Object layout = {
      {"title",
       {{"text", "Business Metrics Correlation Matrix"},
        {"font", {{"size", 16}}}}},
      {"xaxis",
       {{"title", "Variables"}, {"side", "bottom"}, {"tickangle", 45}}},
      {"yaxis",
       {
           {"title", "Variables"}, {"autorange", "reversed"}
           // Reverse y-axis for matrix display
       }},
      {"width", 800},
      {"height", 700},
      {"margin", {{"l", 150}, {"r", 100}, {"t", 100}, {"b", 150}}}};

  // Create the plot
  std::vector<plotly::Object> data = {trace};
  fig.newPlot(data, layout);

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 800},
                                {"height", 700},
                                {"filename", "heatmap_correlation"}};
    fig.downloadImage(imageOpts);
  }

  return 0;
}
