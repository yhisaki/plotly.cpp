# Plotly.cpp API Documentation

## Overview

**Plotly.cpp** brings the power of [Plotly.js](https://plotly.com/javascript/) to C++. This library provides a modern C++17 interface for creating interactive data visualizations with real-time updates, event handling, and export capabilities.

![Plotly.cpp Demo](/docs/images/gallery.png)

## Key Features

- 🔗 **Plotly.js API Mapping** - Translation of most Plotly.js methods

- 🎨 **Advanced Visualizations** - Rich variety of plot types. See [gallery](gallery/README.md) for more examples.

- ⚡ **Real-Time Updates** - Stream data with smooth animations and live updates

- 🔄 **Bidirectional Events** - Handle user interactions from C++

## Architecture

The library uses a **client-server architecture**:

1. **C++ Backend** - Your application using `plotly::Figure`
   - WebSocket server for real-time communication
   - HTTP server serving the web frontend
   - JSON-RPC protocol for structured messaging

2. **JavaScript Frontend** - Browser-based rendering engine
   - Plotly.js runtime for visualization
   - Event bridge for user interactions

## Quick Start

```cpp
#include "plotly/plotly.hpp"

int main() {
    plotly::Figure fig;
    fig.openBrowser();

    std::vector<double> x = {1, 2, 3, 4, 5};
    std::vector<double> y = {1, 4, 2, 8, 5};

    plotly::Object trace = {
        {"x", x}, {"y", y},
        {"type", "scatter"},
        {"mode", "lines+markers"}
    };

    fig.newPlot(plotly::Array{trace});
    fig.waitClose();
    return 0;
}
```

## Core Classes

| Class | Description |
|-------|-------------|
| @ref plotly::Figure | Main plotting interface - your primary entry point |
| @ref plotly::Object | JSON-compatible data container (`nlohmann::json` alias) |

## Essential Methods

| Method | JavaScript Equivalent | Purpose |
|--------|----------------------|---------|
| @ref plotly::Figure::newPlot() | [`Plotly.newPlot()`](https://plotly.com/javascript/plotlyjs-function-reference/#plotlynewplot) | Create new plot |
| @ref plotly::Figure::update() | [`Plotly.update()`](https://plotly.com/javascript/plotlyjs-function-reference/#plotlyupdate) | Update data & layout |
| @ref plotly::Figure::extendTraces() | [`Plotly.extendTraces()`](https://plotly.com/javascript/plotlyjs-function-reference/#plotlyextendtraces) | Stream real-time data |
| @ref plotly::Figure::on() | [Event listeners](https://plotly.com/javascript/plotlyjs-events/) | Handle user interactions |

## Getting Started

1. **Installation** - See [Installation & Quick Start](https://github.com/yhisaki/plotly.cpp#-installation--quick-start) for installation instructions
2. **API Reference** - Browse the class documentation starting with @ref plotly::Figure or see [Complete API Reference](https://github.com/yhisaki/plotly.cpp#-complete-api-reference)
3. **Examples** - Check the `gallery/` directory for comprehensive examples

## Dependencies

- **C++17 or higher**
- **nlohmann/json** - JSON serialization (auto-fetched if not found)
- **Chrome/Chromium browser** - For visualization frontend

---

For complete examples and advanced usage, visit the [project repository](https://github.com/yhisaki/plotly.cpp).
