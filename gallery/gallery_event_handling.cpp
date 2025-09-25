/**
 * @file gallery_event_handling.cpp
 * @brief Interactive Event Handling - User Interaction with Plots
 * @author plotly.cpp contributors
 * @date 2025
 *
 * @example gallery_event_handling.cpp
 *
 * # Interactive Event Handling Example
 *
 * This example demonstrates how to handle user interactions with plots using
 * Plotly.js event callbacks. The program responds to mouse hover and click
 * events on plot elements in real-time.
 *
 * ## What You'll Learn
 * - Event-driven programming with interactive plots
 * - Handling plotly_hover and plotly_click events
 * - Dynamic plot annotation based on user interactions
 * - Bidirectional communication between C++ backend and browser
 * - Real-time plot updates using relayout() function
 *
 * ## Sample Output
 * The example creates an interactive scatter plot where:
 * - Hovering over points displays "hover" annotation at the point location
 * - Clicking on points displays "click" annotation and prints coordinates to
 * console
 * - Both interactions dynamically update the plot with blue text annotations
 *
 * @image html event_handling.gif "Interactive Event Handling Example"
 *
 * @see plotly::Figure::on() For event callback registration
 * @see plotly::Figure::relayout() For dynamic layout updates
 */

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
