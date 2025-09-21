#include "plotly/logger.hpp"
#include <glog/logging.h>
#include <gtest/gtest.h>

auto main(int argc, char **argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  plotly::setLogLevel(plotly::LogLevel::TRACE);
  // Optional: also log to stderr for visibility in CI
  FLAGS_alsologtostderr = true;
  return RUN_ALL_TESTS();
}
