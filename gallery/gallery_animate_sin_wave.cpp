
/**
 * @file gallery_animate_sin_wave.cpp
 * @brief Animated Sin Wave - Real-time Animation with Plotly Frames
 * @author plotly.cpp contributors
 * @date 2025
 *
 * @example gallery_animate_sin_wave.cpp
 *
 * # Animated Sin Wave Example
 *
 * This example demonstrates real-time animation capabilities using Plotly's
 * frame-based animation system to visualize a phase-shifting sine wave with
 * smooth transitions and interactive playback controls.
 *
 * ## What You'll Learn
 * - Creating smooth animations with Plotly's addFrames() and animate()
 * functions
 * - Implementing phase-shifting mathematical functions for dynamic
 * visualization
 * - Adding interactive play/pause controls to animated plots
 * - Configuring animation timing and transition parameters
 * - Building frame-based animations with multiple data points per frame
 *
 * ## Sample Output
 * The example creates a continuously animated sine wave that shifts phase over
 * time:
 * - X range: -4π to 4π with 400 sample points for smooth curves
 * - 60 animation frames showing phase progression from 0 to 12π
 * - Interactive controls allowing users to play, pause, and control animation
 * speed
 * - Blue sine wave with equation sin(x + φ) where φ increases with each frame
 *
 * @image html animate_sin_wave.gif "Animated Sin Wave Example Output"
 *
 * @see plotly::Figure For the main plotting interface
 * @see plotly::Figure::addFrames() For adding animation frames
 * @see plotly::Figure::animate() For controlling animation playback
 */

#include "plotly/plotly.hpp"
#include "utils/linspace.hpp"
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

auto main() -> int {
  std::cout << "Starting animated sin wave with animate function..." << '\n';
  plotly::Figure fig;
  fig.openBrowser();

  // Parameters for the animation
  const double xMin = -4 * M_PI;
  const double xMax = 4 * M_PI;
  const int numPoints = 400;
  const int numFrames = 60;     // Number of animation frames
  const double phaseStep = 0.2; // Phase increment per frame

  // Generate x values
  auto x = linspace(xMin, xMax, numPoints);

  // Create initial trace with first frame data
  std::vector<double> yInitial;
  yInitial.reserve(x.size());
  for (const auto &xi : x) {
    yInitial.push_back(std::sin(xi));
  }

  plotly::Object initialTrace = {{"x", x},
                                 {"y", yInitial},
                                 {"type", "scatter"},
                                 {"mode", "lines"},
                                 {"line", {{"color", "blue"}, {"width", 3}}},
                                 {"name", "sin(x + φ)"}};

  // Layout configuration with animation controls
  plotly::Object layout = {
      {"title", {{"text", "Animated Sin Wave - Using animate() function"}}},
      {"xaxis",
       {{"title", "x"},
        {"range", std::vector<double>{xMin, xMax}},
        {"showgrid", true}}},
      {"yaxis",
       {{"title", "sin(x + φ)"},
        {"range", std::vector<double>{-1.5, 1.5}},
        {"showgrid", true}}},
      {"showlegend", true},
      {"width", 800},
      {"height", 600},
      {"updatemenus",
       std::vector<plotly::Object>{
           {{"type", "buttons"},
            {"direction", "left"},
            {"showactive", false},
            {"x", 0.1},
            {"y", 0},
            {"xanchor", "right"},
            {"yanchor", "top"},
            {"buttons",
             std::vector<plotly::Object>{
                 {{"label", "Play"},
                  {"method", "animate"},
                  {"args",
                   std::vector<plotly::Object>{
                       plotly::Object(), // null for all frames
                       {{"frame", {{"duration", 100}, {"redraw", false}}},
                        {"transition", {{"duration", 0}}},
                        {"fromcurrent", true},
                        {"mode", "immediate"}}}}},
                 {{"label", "Pause"},
                  {"method", "animate"},
                  {"args",
                   std::vector<plotly::Object>{
                       std::vector<plotly::Object>(), // empty array to pause
                       {{"frame", {{"duration", 0}, {"redraw", false}}},
                        {"transition", {{"duration", 0}}},
                        {"mode", "immediate"}}}}}}}}}}};

  // Create initial plot
  std::vector<plotly::Object> data = {initialTrace};
  fig.newPlot(data, layout);

  // Generate animation frames
  std::vector<plotly::Object> frames;
  frames.reserve(numFrames);

  for (int frame = 0; frame < numFrames; frame++) {
    double phase = frame * phaseStep;

    // Calculate y values for this frame
    std::vector<double> yFrame;
    yFrame.reserve(x.size());
    for (const auto &xi : x) {
      yFrame.push_back(std::sin(xi + phase));
    }

    // Create frame object
    plotly::Object frameObj = {
        {"name", std::to_string(frame)},
        {"data", std::vector<plotly::Object>{
                     {{"x", x}, {"y", yFrame}, {"type", "scatter"}}}}};

    frames.push_back(frameObj);
  }

  // Add frames to the plot
  std::cout << "Adding " << frames.size() << " animation frames..." << '\n';
  fig.addFrames(frames);

  // Start the animation
  std::cout << "Starting animation. Use the Play/Pause buttons to control."
            << '\n';
  plotly::Object animationOpts = {
      {"frame", {{"duration", 100}, {"redraw", false}}},
      {"transition", {{"duration", 50}}},
      {"fromcurrent", true},
      {"mode", "immediate"}};

  // Auto-start the animation
  fig.animate(plotly::Object(), animationOpts);

  fig.waitClose();

  return 0;
}
