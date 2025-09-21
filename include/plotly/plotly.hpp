#ifndef PLOTLY_PLOTLY_HPP
#define PLOTLY_PLOTLY_HPP

/**
 * @file plotly.hpp
 * @brief Public Plotly C++ API header.
 * @details
 *  This header provides a C++ API that interoperates with Plotly.js to create,
 *  update, and export figures. `plotly::Figure` acts as an RAII handle to a
 *  Plotly.js runtime and exposes asynchronous operations via
 *  `std::future<plotly::Object>`.
 */

#include "plotly/abi_macros.hpp"

#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>

/**
 * @namespace plotly
 * @brief C++ bindings for Plotly.
 */
namespace plotly {

using Object = nlohmann::json;
using Array = std::vector<Object>;

/**
 * @brief Handle for creating and manipulating a Plotly figure.
 * @details
 *  - Non-copyable (copy constructor and assignment are disabled)
 *  - Some operations are asynchronous and return `std::future<plotly::Object>`
 *  - Returned `plotly::Object` values carry Plotly.js responses and event
 *    payloads in a JSON-compatible structure
 *
 *  Typical usage:
 *  1. Construct a `Figure` (optionally auto-opening the viewer/frontend)
 *  2. Call `newPlot` to render the initial chart
 *  3. Call `update` and/or `relayout` to modify the chart
 *  4. Optionally call `downloadImage` to export an image
 *  5. Register event listeners with `on`
 *
 * @note Thread-safety characteristics depend on the implementation. If multiple
 *       threads will operate on the same instance, coordinate access
 *       appropriately.
 */
class PLOTLY_EXPORT Figure {
private:
  class Impl;                   ///< Internal implementation class (PIMPL idiom)
  std::unique_ptr<Impl> _pimpl; ///< Owning pointer to implementation

public:
  /**
   * @brief Construct a new `Figure` instance.
   */
  Figure(const std::filesystem::path &webappPath =
             std::filesystem::path(PLOTLY_CPP_WEBAPP_PATH));

  /** @brief Copy construction is disabled. */
  Figure(const Figure &) = delete;
  /** @brief Copy assignment is disabled. */
  auto operator=(const Figure &) -> Figure & = delete;
  /** @brief Destructor. */
  ~Figure();

  /**
   * @brief Open the figure in the browser.
   * @param headless If true, the browser will be opened in headless mode.
   * @return true if successful, false otherwise.
   */
  auto openBrowser(bool headless = false) -> bool;

  /**
   * @brief Check whether the figure is currently open.
   * @return true if open; false otherwise.
   */
  [[nodiscard]] auto isOpen() const -> bool;

  /**
   * @brief Wait until the figure is closed (no client connected).
   * @details This method blocks until the WebSocket client disconnects,
   *          indicating that the figure viewer has been closed.
   */
  void waitClose() const;

  /**
   * @brief Download the figure as an image.
   * @param opts Options compatible with Plotly.js `Plotly.downloadImage`.
   * @return true if the download was initiated successfully, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlydownloadimage
   */
  auto downloadImage(const Object &opts = Object()) -> bool;

  /**
   * @brief Create and render a new plot.
   * @param data Data array equivalent to Plotly.js `Plotly.newPlot` data.
   * @param layout Optional layout specification.
   * @param config Optional configuration.
   * @return true if the plot was created successfully, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlynewplot
   */
  auto newPlot(const Object &data, const Object &layout = Object(),
               const Object &config = Object()) -> bool;

  /**
   * @brief Update an existing plot.
   * @param traceUpdate Trace updates (equivalent to the data portion of
   *        Plotly.js `Plotly.update`).
   * @param layoutUpdate Optional layout updates.
   * @return true if the update was successful, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlyupdate
   */
  auto update(const Object &traceUpdate, const Object &layoutUpdate = Object())
      -> bool;

  /**
   * @brief Apply layout-only changes to the plot.
   * @param layout Layout specification (equivalent to Plotly.js
   *               `Plotly.relayout`). If omitted, defaults are used.
   * @return true if the relayout was successful, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlyrelayout
   */
  auto relayout(const Object &layout = Object()) -> bool;

  /**
   * @brief Redraw the plot.
   * @return true if the redraw was successful, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlyredraw
   */
  auto redraw() -> bool;

  /**
   * @brief Purge the plot, removing all data and layout.
   * @return true if the purge was successful, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlypurge
   */
  auto purge() -> bool;

  /**
   * @brief Restyle existing traces.
   * @param aobj Style updates to apply.
   * @param traces Optional trace indices to target (defaults to all).
   * @return true if the restyle was successful, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlyrestyle
   */
  auto restyle(const Object &aobj, const Object &traces = Object()) -> bool;

  /**
   * @brief Add new traces to the plot.
   * @param traces New trace data to add.
   * @param newIndices Optional indices where to insert the traces.
   * @return true if the traces were added successfully, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlyaddtraces
   */
  auto addTraces(const Object &traces, const Object &newIndices = Object())
      -> bool;

  /**
   * @brief Delete traces from the plot.
   * @param indices Indices of traces to delete.
   * @return true if the traces were deleted successfully, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlydeletetraces
   */
  auto deleteTraces(const Object &indices) -> bool;

  /**
   * @brief Move traces to new positions.
   * @param currentIndices Current indices of traces to move.
   * @param newIndices New indices for the traces.
   * @return true if the traces were moved successfully, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlymovetraces
   */
  auto moveTraces(const Object &currentIndices, const Object &newIndices)
      -> bool;

  /**
   * @brief Extend existing traces with new data.
   * @param update Data to append to traces.
   * @param indices Trace indices to extend.
   * @param maxPoints Optional maximum number of points to keep.
   * @return true if the traces were extended successfully, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlyextendtraces
   */
  auto extendTraces(const Object &update, const Object &indices,
                    const Object &maxPoints = Object()) -> bool;

  /**
   * @brief Prepend data to existing traces.
   * @param update Data to prepend to traces.
   * @param indices Trace indices to prepend to.
   * @return true if the data was prepended successfully, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlyprependtraces
   */
  auto prependTraces(const Object &update, const Object &indices) -> bool;

  /**
   * @brief React-style update of the plot.
   * @param data New data array.
   * @param layout Optional layout specification.
   * @param config Optional configuration.
   * @return true if the react update was successful, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlyreact
   */
  auto react(const Object &data, const Object &layout = Object(),
             const Object &config = Object()) -> bool;

  /**
   * @brief Add animation frames to the plot.
   * @param frames Frame data to add.
   * @return true if the frames were added successfully, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlyaddframes
   */
  auto addFrames(const Object &frames) -> bool;

  /**
   * @brief Delete animation frames from the plot.
   * @param frames Frame names or indices to delete.
   * @return true if the frames were deleted successfully, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlydeleteframes
   */
  auto deleteFrames(const Object &frames) -> bool;

  /**
   * @brief Animate the plot.
   * @param frameOrGroupNameOrFrameList Frame specification for animation.
   * @param opts Animation options.
   * @return true if the animation was started successfully, false otherwise.
   * @see
   * https://plotly.com/javascript/plotlyjs-function-reference/#plotlyanimate
   */
  auto animate(const Object &frameOrGroupNameOrFrameList,
               const Object &opts = Object()) -> bool;

  /**
   * @brief Register a listener for a Plotly event.
   * @param event Event name (e.g., "plotly_click", "plotly_hover").
   * @param callback Callback invoked when the event fires; receives the event
   *                 payload as `plotly::Object`.
   * @return true if the event listener was registered successfully, false
   * otherwise.
   * @see https://plotly.com/javascript/plotlyjs-events/
   */
  auto on(const std::string &event,
          const std::function<void(plotly::Object)> &callback) -> bool;

  /**
   * @brief Remove all listeners for a specific Plotly event.
   * @param event Event name (e.g., "plotly_click", "plotly_hover").
   * @return true if the listeners were removed successfully, false otherwise.
   * @see https://plotly.com/javascript/plotlyjs-events/
   */
  auto removeAllListeners(const std::string &event) -> bool;

  /**
   * @brief Set the download directory for the browser.
   * @param directory Path to the directory where downloads should be saved.
   * @param remoteDebuggingPort Port for Chrome DevTools remote debugging
   * (default: 9222).
   * @return true if successful, false otherwise.
   * @details This function sets the download directory for the browser instance
   *          used by the Figure. It communicates with the browser via Chrome
   * DevTools Protocol to configure the download behavior.
   */
  auto setDownloadDirectory(const std::filesystem::path &directory,
                            int remoteDebuggingPort = 9222) -> bool;
};

} // namespace plotly

#endif // PLOTLY_PLOTLY_HPP
