
#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include "utils/linspace.hpp"
#include <cmath>
#include <random>
#include <vector>

// Statistical distribution functions
auto normalPDF(double x, double mean, double stddev) -> double {
  double variance = stddev * stddev;
  return (1.0 / std::sqrt(2 * M_PI * variance)) *
         std::exp(-0.5 * std::pow(x - mean, 2) / variance);
}

auto exponentialPDF(double x, double lambda) -> double {
  return (x >= 0) ? lambda * std::exp(-lambda * x) : 0.0;
}

auto gammaPDF(double x, double shape, double scale) -> double {
  if (x <= 0)
    return 0.0;
  return std::pow(x, shape - 1) * std::exp(-x / scale) /
         (std::tgamma(shape) * std::pow(scale, shape));
}

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);

  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Generate x values
  auto x = linspace(-2.0, 8.0, 200);

  // Calculate PDF values for different distributions
  std::vector<double> normalY, exponentialY, gammaY;
  normalY.reserve(x.size());
  exponentialY.reserve(x.size());
  gammaY.reserve(x.size());

  for (const auto &xi : x) {
    normalY.push_back(normalPDF(xi, 2.0, 1.0));      // Normal(μ=2, σ=1)
    exponentialY.push_back(exponentialPDF(xi, 0.5)); // Exponential(λ=0.5)
    gammaY.push_back(gammaPDF(xi, 2.0, 1.5));        // Gamma(k=2, θ=1.5)
  }

  // Generate histogram data from samples
  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<double> normalDist(2.0, 1.0);

  const int numSamples = 1000;
  std::vector<double> normalSamples;
  normalSamples.reserve(numSamples);

  for (int i = 0; i < numSamples; i++) {
    normalSamples.push_back(normalDist(gen));
  }

  // Create traces for continuous distributions
  plotly::Object normalTrace = {{"type", "scatter"},
                                {"x", x},
                                {"y", normalY},
                                {"mode", "lines"},
                                {"name", "Normal(μ=2, σ=1)"},
                                {"line", {{"color", "blue"}, {"width", 3}}}};

  plotly::Object exponentialTrace = {
      {"type", "scatter"},
      {"x", x},
      {"y", exponentialY},
      {"mode", "lines"},
      {"name", "Exponential(λ=0.5)"},
      {"line", {{"color", "red"}, {"width", 3}}}};

  plotly::Object gammaTrace = {{"type", "scatter"},
                               {"x", x},
                               {"y", gammaY},
                               {"mode", "lines"},
                               {"name", "Gamma(k=2, θ=1.5)"},
                               {"line", {{"color", "green"}, {"width", 3}}}};

  // Create histogram trace for normal samples
  plotly::Object histogramTrace = {{"type", "histogram"},
                                   {"x", normalSamples},
                                   {"name", "Normal Samples (n=1000)"},
                                   {"opacity", 0.4},
                                   {"marker", {{"color", "lightblue"}}},
                                   {"yaxis", "y2"},
                                   {"histnorm", "probability density"}};

  // Create layout with dual y-axes
  plotly::Object layout = {{"title",
                            {{"text", "Statistical Distributions Comparison"},
                             {"font", {{"size", 18}}}}},
                           {"xaxis", {{"title", "x"}, {"showgrid", true}}},
                           {"yaxis",
                            {{"title", "Probability Density Function"},
                             {"showgrid", true},
                             {"domain", {0.0, 0.7}}}},
                           {"yaxis2",
                            {{"title", "Sample Frequency"},
                             {"domain", {0.75, 1.0}},
                             {"side", "right"}}},
                           {"width", 900},
                           {"height", 700},
                           {"showlegend", true},
                           {"legend", {{"x", 0.7}, {"y", 0.9}}}};

  // Create the plot
  std::vector<plotly::Object> data = {normalTrace, exponentialTrace, gammaTrace,
                                      histogramTrace};
  fig.newPlot(data, layout);

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 900},
                                {"height", 700},
                                {"filename", "statistical_distributions"}};
    fig.downloadImage(imageOpts);
  }

  return 0;
}
