#include "plotly/plotly.hpp"
#include <gtest/gtest.h>

namespace plotly {

class FigureTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

// Test basic Figure construction
TEST_F(FigureTest, Construction) {
  EXPECT_NO_THROW({ Figure fig; });
}

// Test basic plotting workflow with headless browser
TEST_F(FigureTest, BasicPlottingWorkflow) {
  Figure fig;

  // Open browser in headless mode
  bool browserOpened = fig.openBrowser(true);

  if (browserOpened) {
    // Create simple plot data
    Object trace = {{"x", Object::array({1, 2, 3, 4})},
                    {"y", Object::array({10, 11, 12, 13})},
                    {"type", "scatter"},
                    {"mode", "lines+markers"}};

    Object data = Object::array({trace});

    Object layout = Object{{"title", "Test Plot"}};

    // Test newPlot - should not throw and should succeed
    EXPECT_NO_THROW({
      bool result = fig.newPlot(data, layout);
      EXPECT_TRUE(result);
    });

    // Test downloadImage - should not throw
    Object opts = Object{{"format", "png"},
                         {"width", 800},
                         {"height", 600},
                         {"filename", "test_plot"}};

    EXPECT_NO_THROW({ fig.downloadImage(opts); });
  }
}

} // namespace plotly
