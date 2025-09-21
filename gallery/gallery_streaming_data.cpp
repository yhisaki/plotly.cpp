
#include "plotly/plotly.hpp"
#include <chrono>
#include <cmath>
#include <thread>
#include <vector>

auto main() -> int {
  plotly::Figure fig;
  fig.openBrowser();
  std::vector<double> x, y;

  // Create initial empty trace (same structure as Plotly.js)
  plotly::Object trace = {
      {"x", x}, {"y", y}, {"type", "scatter"}, {"mode", "lines"}};
  std::vector<plotly::Object> data = {trace};
  fig.newPlot(data);

  // Stream data in real-time using Plotly.js extendTraces pattern
  for (double t = 0; t < 100 && fig.isOpen(); t += 0.1) {
    // Direct equivalent of Plotly.extendTraces()
    fig.extendTraces({{"x", {std::vector<double>{t}}},
                      {"y", {std::vector<double>{std::sin(t)}}}},
                     {0}, 100 // trace index 0, max 100 points
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  return 0;
}
