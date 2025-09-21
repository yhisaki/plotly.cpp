
#include "plotly/plotly.hpp"
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <utility>
#include <vector>

class GameOfLife {
private:
  int _width, _height;
  std::vector<std::vector<int>> _grid;
  std::vector<std::vector<int>> _nextGrid;

public:
  GameOfLife(int w, int h) : _width(w), _height(h) {
    _grid = std::vector<std::vector<int>>(_height, std::vector<int>(_width, 0));
    _nextGrid =
        std::vector<std::vector<int>>(_height, std::vector<int>(_width, 0));
  }

  void randomize(double probability = 0.3) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(probability);

    for (int y = 0; y < _height; y++) {
      for (int x = 0; x < _width; x++) {
        _grid[y][x] = dist(gen) ? 1 : 0;
      }
    }
  }

  void addGlider(int startX, int startY) {
    // Classic glider pattern
    std::vector<std::pair<int, int>> glider = {
        {1, 0}, {2, 1}, {0, 2}, {1, 2}, {2, 2}};

    for (const auto &[dx, dy] : glider) {
      int x = startX + dx;
      int y = startY + dy;
      if (x >= 0 && x < _width && y >= 0 && y < _height) {
        _grid[y][x] = 1;
      }
    }
  }

  void addOscillator(int startX, int startY) {
    // Blinker oscillator (3 cells in a row)
    for (int i = 0; i < 3; i++) {
      int x = startX + i;
      int y = startY;
      if (x >= 0 && x < _width && y >= 0 && y < _height) {
        _grid[y][x] = 1;
      }
    }
  }

  auto countNeighbors(int x, int y) -> int {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
      for (int dx = -1; dx <= 1; dx++) {
        if (dx == 0 && dy == 0)
          continue;

        int nx = x + dx;
        int ny = y + dy;

        // Wrap around edges (toroidal topology)
        nx = (nx + _width) % _width;
        ny = (ny + _height) % _height;

        count += _grid[ny][nx];
      }
    }
    return count;
  }

  void step() {
    // Apply Conway's rules
    for (int y = 0; y < _height; y++) {
      for (int x = 0; x < _width; x++) {
        int neighbors = countNeighbors(x, y);
        int cell = _grid[y][x];

        if (cell == 1) {
          // Live cell
          if (neighbors < 2 || neighbors > 3) {
            _nextGrid[y][x] = 0; // Dies
          } else {
            _nextGrid[y][x] = 1; // Survives
          }
        } else {
          // Dead cell
          if (neighbors == 3) {
            _nextGrid[y][x] = 1; // Born
          } else {
            _nextGrid[y][x] = 0; // Stays dead
          }
        }
      }
    }

    // Swap grids
    _grid.swap(_nextGrid);
  }

  [[nodiscard]] auto getGrid() const -> const std::vector<std::vector<int>> & {
    return _grid;
  }

  [[nodiscard]] auto countLiveCells() const -> int {
    int count = 0;
    for (const auto &row : _grid) {
      for (int cell : row) {
        count += cell;
      }
    }
    return count;
  }
};

auto main() -> int {
  std::cout << "Starting Conway's Game of Life..." << '\n';
  plotly::Figure fig;
  fig.openBrowser();

  // Game parameters
  const int width = 50;
  const int height = 50;
  const int generations = 200;
  const int stepDelay = 100; // milliseconds

  GameOfLife game(width, height);

  // Initialize with interesting patterns
  game.randomize(0.15);       // Sparse random cells
  game.addGlider(5, 5);       // Moving pattern
  game.addGlider(15, 25);     // Another glider
  game.addOscillator(30, 10); // Blinking pattern
  game.addOscillator(35, 35); // Another oscillator

  // Create coordinate arrays
  std::vector<double> xCoords, yCoords;
  for (int x = 0; x < width; x++) {
    xCoords.push_back(x);
  }
  for (int y = 0; y < height; y++) {
    yCoords.push_back(y);
  }

  // Create heatmap trace
  plotly::Object trace = {
      {"type", "heatmap"},
      {"x", xCoords},
      {"y", yCoords},
      {"z", game.getGrid()},
      {"colorscale",
       {
           {0.0, "white"}, // Dead cells
           {1.0, "black"}  // Live cells
       }},
      {"showscale", false},
      {"hovertemplate", "Cell (%{x}, %{y})<br>State: %{z}<extra></extra>"}};

  // Create layout
  plotly::Object layout = {
      {"title",
       {{"text", "Conway's Game of Life<br>" +
                     std::string("<sub>Generation 0 - Live Cells: ") +
                     std::to_string(game.countLiveCells()) + "</sub>"},
        {"font", {{"size", 16}}}}},
      {"xaxis",
       {{"title", "X"}, {"showgrid", false}, {"showticklabels", false}}},
      {"yaxis",
       {
           {"title", "Y"},
           {"showgrid", false},
           {"showticklabels", false},
           {"scaleanchor", "x"},
           {"autorange", "reversed"} // Flip Y axis for better view
       }},
      {"width", 800},
      {"height", 800},
      {"margin", {{"l", 50}, {"r", 50}, {"t", 80}, {"b", 50}}}};

  // Create initial plot
  std::vector<plotly::Object> data = {trace};
  fig.newPlot(data, layout);

  std::cout << "Starting simulation with " << game.countLiveCells()
            << " initial live cells..." << '\n';
  std::cout
      << "Patterns: Gliders (moving), Oscillators (blinking), Random cells"
      << '\n';

  // Simulation loop
  for (int generation = 1; generation <= generations && fig.isOpen();
       generation++) {
    game.step();

    int liveCells = game.countLiveCells();

    // Update the plot
    fig.restyle({{"z", {game.getGrid()}}}, {0});

    // Update title with generation info
    plotly::Object newLayout = {
        {"title",
         {{"text",
           "Conway's Game of Life<br>" + std::string("<sub>Generation ") +
               std::to_string(generation) +
               " - Live Cells: " + std::to_string(liveCells) + "</sub>"},
          {"font", {{"size", 16}}}}}};
    fig.relayout(newLayout);

    std::this_thread::sleep_for(std::chrono::milliseconds(stepDelay));

    if (generation % 25 == 0) {
      std::cout << "Generation " << generation << ": " << liveCells
                << " live cells" << '\n';
    }

    // Stop if population dies out
    if (liveCells == 0) {
      std::cout << "Population died out at generation " << generation << '\n';
      break;
    }
  }

  // Final message
  fig.relayout(
      {{"title",
        {{"text", "Conway's Game of Life - SIMULATION COMPLETE<br>" +
                      std::string("<sub>Final Population: ") +
                      std::to_string(game.countLiveCells()) + " cells</sub>"},
         {"font", {{"size", 16}, {"color", "red"}}}}}});

  std::cout << "Game of Life simulation completed. Close browser to exit."
            << '\n';
  fig.waitClose();

  return 0;
}
