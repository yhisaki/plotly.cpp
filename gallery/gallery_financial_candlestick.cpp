
/**
 * @file gallery_financial_candlestick.cpp
 * @brief Financial Candlestick Chart - OHLC Stock Data Visualization
 * @author plotly.cpp contributors
 * @date 2025
 *
 * @example gallery_financial_candlestick.cpp
 *
 * # Financial Candlestick Chart Example
 *
 * This example demonstrates how to create professional financial charts using
 * candlestick traces with volume indicators. It generates synthetic stock
 * market data and visualizes it with the standard OHLC (Open, High, Low, Close)
 * format used in financial analysis.
 *
 * ## What You'll Learn
 * - Creating candlestick charts for financial data visualization
 * - Generating realistic synthetic stock data with random price movements
 * - Implementing dual y-axis layouts for price and volume data
 * - Customizing candlestick colors for bullish/bearish patterns
 * - Working with time-series data and date formatting
 * - Adding volume bars as complementary indicators
 *
 * ## Sample Output
 * The example creates a comprehensive financial chart featuring:
 * - 60-day candlestick chart with OHLC price data
 * - Green candles for price increases, red candles for decreases
 * - Volume bars displayed below the main price chart
 * - Interactive hover information showing detailed price data
 * - Professional styling suitable for financial analysis
 *
 * @image html financial_candlestick.png "Financial Candlestick Chart Example"
 *
 * @see plotly::Object For candlestick trace configuration
 * @see parseGalleryArgs() For command line argument handling
 */

#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include <cstdlib>
#include <random>
#include <string>
#include <vector>

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);

  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Generate synthetic stock data
  const int numDays = 60;
  std::vector<std::string> dates;
  std::vector<double> open, high, low, close, volume;

  // Random number generation for realistic stock data
  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<double> priceChange(0.0, 2.0);
  std::uniform_real_distribution<double> volumeDist(100000, 1000000);

  double currentPrice = 100.0;

  for (int i = 0; i < numDays; i++) {
    // Generate date string (simplified)
    dates.push_back("2024-" + std::to_string((i / 30) + 1) + "-" +
                    std::to_string((i % 30) + 1));

    // Generate OHLC data
    double dayOpen = currentPrice;
    double priceRange = std::abs(priceChange(gen));
    double dayHigh = dayOpen + priceRange * 0.8;
    double dayLow = dayOpen - priceRange * 0.6;
    double dayClose =
        dayLow + (dayHigh - dayLow) *
                     std::uniform_real_distribution<double>(0.2, 0.8)(gen);

    open.push_back(dayOpen);
    high.push_back(dayHigh);
    low.push_back(dayLow);
    close.push_back(dayClose);
    volume.push_back(volumeDist(gen));

    currentPrice = dayClose + priceChange(gen) * 0.5;
    if (currentPrice < 50)
      currentPrice = 50;
    if (currentPrice > 200)
      currentPrice = 200;
  }

  // Create candlestick trace
  plotly::Object candlestickTrace = {
      {"type", "candlestick"},
      {"x", dates},
      {"open", open},
      {"high", high},
      {"low", low},
      {"close", close},
      {"name", "Stock Price"},
      {"yaxis", "y"},
      {"increasing",
       {{"fillcolor", "#00ff00"}, {"line", {{"color", "#00aa00"}}}}},
      {"decreasing",
       {{"fillcolor", "#ff0000"}, {"line", {{"color", "#aa0000"}}}}}};

  // Create volume bar trace
  plotly::Object volumeTrace = {{"type", "bar"},
                                {"x", dates},
                                {"y", volume},
                                {"name", "Volume"},
                                {"yaxis", "y2"},
                                {"opacity", 0.3},
                                {"marker", {{"color", "blue"}}}};

  // Create layout with dual y-axes
  plotly::Object layout = {
      {"title", {{"text", "Stock Price Candlestick Chart with Volume"}}},
      {"xaxis", {{"title", "Date"}, {"rangeslider", {{"visible", false}}}}},
      {"yaxis", {{"title", "Price ($)"}, {"domain", {0.3, 1.0}}}},
      {"yaxis2",
       {{"title", "Volume"}, {"domain", {0.0, 0.25}}, {"side", "right"}}},
      {"width", 1000},
      {"height", 700},
      {"showlegend", true}};

  // Create the plot
  std::vector<plotly::Object> data = {candlestickTrace, volumeTrace};
  fig.newPlot(data, layout);

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 1000},
                                {"height", 700},
                                {"filename", "financial_candlestick"}};
    fig.downloadImage(imageOpts);
  }

  return 0;
}
