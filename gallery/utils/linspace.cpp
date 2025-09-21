#include "linspace.hpp"
#include <vector>

auto linspace(double a, double b, int n) -> std::vector<double> {
  std::vector<double> result(n);
  for (int i = 0; i < n; i++) {
    result[i] = a + i * (b - a) / (n - 1);
  }
  return result;
}
