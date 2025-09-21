#include "arg_parser.hpp"
#include <string>

auto parseGalleryArgs(int argc, char *argv[]) -> GalleryArgs {
  GalleryArgs args;

  // Parse command line argument for headless mode (default: false)
  if (argc > 1) {
    std::string arg = argv[1];
    if (arg == "true" || arg == "1") {
      args.headless = true;
    }
  }

  return args;
}
