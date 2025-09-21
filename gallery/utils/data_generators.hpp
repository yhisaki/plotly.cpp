#pragma once

#include <vector>

/**
 * @brief Common data generation utilities for gallery examples
 */

/**
 * @brief Generate sine wave data
 * @param start Start value
 * @param end End value
 * @param step Step size
 * @param phase Phase shift (default: 0)
 * @return std::pair<std::vector<double>, std::vector<double>> x and y values
 */
auto generateSineWave(double start, double end, double step, double phase = 0.0)
    -> std::pair<std::vector<double>, std::vector<double>>;

/**
 * @brief Generate 2D surface data for 3D plots
 * @param size Grid size (size x size)
 * @param range Range for x and y coordinates
 * @return std::vector<std::vector<double>> z values for surface
 */
auto generateSurfaceData(int size, double range = 5.0)
    -> std::vector<std::vector<double>>;

/**
 * @brief Generate sample scatter plot data
 * @param n Number of points
 * @return std::pair<std::vector<double>, std::vector<double>> x and y values
 */
auto generateScatterData(int n)
    -> std::pair<std::vector<double>, std::vector<double>>;
