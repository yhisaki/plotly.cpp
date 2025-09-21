#pragma once

#include <vector>

/**
 * @brief Create a linearly spaced vector of values between two endpoints
 * @param a Start value
 * @param b End value
 * @param n Number of points to generate
 * @return std::vector<double> Vector containing n linearly spaced values from a
 * to b
 */
auto linspace(double a, double b, int n) -> std::vector<double>;
