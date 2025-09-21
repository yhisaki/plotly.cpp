#include "data_generators.hpp"
#include <cmath>
#include <utility>
#include <vector>

auto generateSineWave(double start, double end, double step, double phase)
    -> std::pair<std::vector<double>, std::vector<double>> {
  std::vector<double> x, y;

  for (double t = start; t < end; t += step) {
    x.push_back(t);
    y.push_back(std::sin(t + phase));
  }

  return {x, y};
}

auto generateSurfaceData(int size, double range)
    -> std::vector<std::vector<double>> {
  std::vector<std::vector<double>> z(size, std::vector<double>(size));

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      double x = (i - size / 2.0) * range / size;
      double y = (j - size / 2.0) * range / size;
      z[i][j] = std::sin(std::sqrt(x * x + y * y));
    }
  }

  return z;
}

auto generateScatterData(int n)
    -> std::pair<std::vector<double>, std::vector<double>> {
  std::vector<double> x(n), y(n);

  for (int i = 0; i < n; i++) {
    x[i] = i + 1;
    y[i] = i * i % 10 + 1; // Simple pattern
  }

  return {x, y};
}
