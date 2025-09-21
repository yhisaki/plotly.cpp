#pragma once

/**
 * @brief Structure to hold parsed gallery command line arguments
 *
 * This structure provides a consistent way to handle common command line
 * arguments used across the plotly.cpp gallery examples.
 */
struct GalleryArgs {
  bool headless = false; ///< Run in headless mode (no browser opening)
};

/**
 * @brief Parse command line arguments for gallery examples
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return GalleryArgs Parsed arguments structure
 *
 * Currently supports:
 * - First argument: headless mode ("true", "1" enables headless mode)
 */
auto parseGalleryArgs(int argc, char *argv[]) -> GalleryArgs;
