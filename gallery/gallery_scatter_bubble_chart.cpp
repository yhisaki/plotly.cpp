
#include "plotly/plotly.hpp"
#include "utils/arg_parser.hpp"
#include <algorithm>
#include <cstddef>
#include <random>
#include <string>
#include <vector>

auto main(int argc, char *argv[]) -> int {
  // Parse command line arguments
  auto args = parseGalleryArgs(argc, argv);

  plotly::Figure fig;
  fig.openBrowser(args.headless);

  // Generate synthetic multi-dimensional data representing countries
  std::vector<std::string> countries = {
      "USA",       "China",       "Japan",        "Germany", "India",
      "UK",        "France",      "Italy",        "Brazil",  "Canada",
      "Russia",    "South Korea", "Australia",    "Spain",   "Mexico",
      "Indonesia", "Netherlands", "Saudi Arabia", "Turkey",  "Taiwan"};

  const size_t n = countries.size();
  std::vector<double> gdpPerCapita, lifeExpectancy, population, happiness;
  std::vector<std::string> regions;

  // Random generators
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> gdpDist(20000, 80000);
  std::uniform_real_distribution<double> lifeDist(70, 85);
  std::uniform_real_distribution<double> popDist(10, 1400);
  std::uniform_real_distribution<double> happinessDist(4.5, 8.0);

  std::vector<std::string> regionList = {"North America", "Asia", "Europe",
                                         "South America", "Oceania"};

  // Generate data with some correlation patterns
  for (size_t i = 0; i < n; i++) {
    double gdp = gdpDist(gen);
    double life =
        65 + (gdp - 20000) / 2000 + std::normal_distribution<double>(0, 3)(gen);
    double pop = popDist(gen);
    double happy =
        3 + life / 15 + std::normal_distribution<double>(0, 0.8)(gen);

    // Clamp values to reasonable ranges
    life = std::max(65.0, std::min(85.0, life));
    happy = std::max(3.0, std::min(8.5, happy));

    gdpPerCapita.push_back(gdp);
    lifeExpectancy.push_back(life);
    population.push_back(pop);
    happiness.push_back(happy);
    regions.push_back(regionList[i % regionList.size()]);
  }

  // Create separate traces for each region
  std::vector<plotly::Object> traces;
  std::vector<std::string> colors = {"red", "blue", "green", "orange",
                                     "purple"};

  for (size_t r = 0; r < regionList.size(); r++) {
    std::vector<double> regionGDP, regionLife, regionPop, regionHappy;
    std::vector<std::string> regionCountries;

    // Filter data by region
    for (size_t i = 0; i < n; i++) {
      if (regions[i] == regionList[r]) {
        regionGDP.push_back(gdpPerCapita[i]);
        regionLife.push_back(lifeExpectancy[i]);
        regionPop.push_back(population[i]);
        regionHappy.push_back(happiness[i]);
        regionCountries.push_back(countries[i]);
      }
    }

    if (!regionGDP.empty()) {
      plotly::Object trace = {
          {"type", "scatter"},
          {"mode", "markers"},
          {"x", regionGDP},
          {"y", regionLife},
          {"text", regionCountries},
          {"name", regionList[r]},
          {"marker",
           {{"size", regionPop},
            {"sizemode", "diameter"},
            {"sizeref", 3.0},
            {"sizemin", 4},
            {"color", regionHappy},
            {"colorscale", "Viridis"},
            {"showscale", r == 0}, // Show colorbar only for first trace
            {"colorbar",
             {{"title", "Happiness Score"}, {"titleside", "right"}}},
            {"line", {{"color", colors[r]}, {"width", 2}}}}},
          {"hovertemplate",
           "%{text}<br>GDP per Capita: $%{x:,.0f}<br>" +
               std::string("Life Expectancy: %{y:.1f} years<br>") +
               "Population: %{marker.size:.0f}M<br>" +
               "Happiness: %{marker.color:.1f}<extra></extra>"}};
      traces.push_back(trace);
    }
  }

  // Create layout
  plotly::Object layout = {
      {"title",
       {{"text", "World Development Indicators<br>" +
                     std::string("<sub>Bubble size = Population (millions), ") +
                     "Color = Happiness Score</sub>"},
        {"font", {{"size", 16}}}}},
      {"xaxis",
       {{"title", "GDP per Capita (USD)"},
        {"type", "log"},
        {"showgrid", true}}},
      {"yaxis", {{"title", "Life Expectancy (years)"}, {"showgrid", true}}},
      {"width", 1000},
      {"height", 700},
      {"showlegend", true},
      {"hovermode", "closest"}};

  // Create the plot
  fig.newPlot(traces, layout);

  if (!args.headless) {
    fig.waitClose();
  } else {
    // Save image instead of opening browser
    plotly::Object imageOpts = {{"format", "png"},
                                {"width", 1000},
                                {"height", 700},
                                {"filename", "scatter_bubble_chart"}};
    fig.downloadImage(imageOpts);
  }

  return 0;
}
