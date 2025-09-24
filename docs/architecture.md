# Architecture Overview {#architecture}

## System Architecture

**Plotly.cpp** implements a sophisticated client-server architecture that bridges C++ applications with web-based visualization capabilities through Plotly.js. The system enables real-time, bidirectional communication between C++ code and a JavaScript frontend running in a web browser.

### Core Architecture Components

The architecture consists of three main layers:

1. **C++ Backend Layer** - The core library providing the `plotly::Figure` API
2. **Communication Layer** - WebSocket and HTTP servers enabling real-time data exchange
3. **JavaScript Frontend Layer** - Browser-based visualization using Plotly.js

\dot
digraph Architecture {
    rankdir=TB;
    node [shape=box, style=filled];

    subgraph cluster_cpp {
        label="C++ Application Layer";
        style=filled;
        color=lightgrey;
        FigureAPI [label="plotly::Figure API\nnewPlot(), update(), relayout()\nextendTraces(), on(), etc.", fillcolor=lightblue];
    }

    subgraph cluster_comm {
        label="Communication Layer";
        style=filled;
        color=lightgreen;
        JSONRPC [label="JSON-RPC Server\n(WebSocket)\nBidirectional method calls\n& notifications", fillcolor=lightyellow];
        HTTP [label="HTTP Server\nServes static JavaScript\nfrontend assets", fillcolor=lightyellow];
        Browser [label="Browser Management\nAutomatic Chrome/Chromium\nlaunch & control", fillcolor=lightyellow];
    }

    subgraph cluster_js {
        label="JavaScript Frontend Layer";
        style=filled;
        color=lightcyan;
        PlotlyJS [label="Plotly.js Runtime\nActual plot rendering\nand user interaction", fillcolor=lightpink];
        RPCClient [label="JSON-RPC Client\nWebSocket communication\nwith C++ backend", fillcolor=lightpink];
        Events [label="Event Bridge\nForwards browser events\nback to C++", fillcolor=lightpink];
    }

    FigureAPI -> JSONRPC [label="Method calls"];
    FigureAPI -> HTTP [label="Static assets"];
    FigureAPI -> Browser [label="Launch browser"];

    JSONRPC -> RPCClient [label="WebSocket\n(JSON-RPC 2.0)", dir=both];
    HTTP -> RPCClient [label="Asset serving"];
    Browser -> PlotlyJS [label="Browser process"];

    RPCClient -> PlotlyJS [label="API calls"];
    PlotlyJS -> Events [label="User interactions"];
    Events -> RPCClient [label="Event data"];
}
\enddot

## Communication Protocol

### JSON-RPC 2.0 Over WebSocket

The system uses **JSON-RPC 2.0** as the primary communication protocol, transmitted over WebSocket connections. This provides:

- **Structured messaging** with request/response semantics
- **Bidirectional communication** (C++ ↔ JavaScript)
- **Error handling** with standardized error codes
- **Real-time updates** with low latency

#### C++ to JavaScript Communication Flow

\msc
App [label="C++ Application"],
Figure [label="plotly::Figure"],
RPC [label="JSON-RPC Server"],
WS [label="WebSocket"],
Client [label="JSON-RPC Client"],
Plotly [label="Plotly.js"];

App => Figure [label="fig.newPlot(data, layout)"];
Figure => RPC [label="Serialize to JSON-RPC request"];
RPC note RPC [label="{\n\"jsonrpc\": \"2.0\",\n\"method\": \"Plotly.newPlot\",\n\"params\": {\n  \"data\": [...],\n  \"layout\": {...},\n  \"config\": {...}\n},\n\"id\": 1\n}"];
RPC => WS [label="Send JSON message"];
WS => Client [label="WebSocket transmission"];
Client => Plotly [label="Plotly.newPlot(element, data, layout, config)"];
Plotly => Client [label="Success/failure result"];
Client => WS [label="JSON-RPC response"];
WS => RPC [label="Response received"];
RPC => Figure [label="Return result"];
Figure => App [label="Return success/failure"];
\endmsc

#### JavaScript to C++ Communication Flow

\msc
User [label="Browser/User"],
Plotly [label="Plotly.js"],
Events [label="Event Bridge"],
Client [label="JSON-RPC Client"],
WS [label="WebSocket"],
RPC [label="JSON-RPC Server"],
Callback [label="C++ Callback"];

User => Plotly [label="User interaction\n(click, hover, zoom)"];
Plotly => Events [label="Event triggered"];
Events => Events [label="Sanitize event data"];
Events => Client [label="Prepare notification"];
Events note Client [label="{\n\"jsonrpc\": \"2.0\",\n\"method\": \"event_callback_uuid\",\n\"params\": {\n  \"points\": [...],\n  \"event\": {...}\n}\n}"];
Client => WS [label="Send notification"];
WS => RPC [label="JSON-RPC notification"];
RPC => Callback [label="Invoke registered handler"];
Callback => RPC [label="Handler execution"];
\endmsc

### HTTP Server for Static Assets

A lightweight HTTP server serves the JavaScript frontend:

- **Single-page application** with bundled Plotly.js
- **WebSocket endpoint discovery** via `/ws_port` endpoint
- **Static file serving** for HTML, CSS, and JavaScript assets
- **Automatic port allocation** to avoid conflicts

## Implementation Details

### C++ Backend Components

#### `plotly::Figure` Class
The main user-facing API that encapsulates:
- **PIMPL idiom** for ABI stability
- **WebSocket server management**
- **HTTP server management**
- **Browser process control**
- **JSON-RPC client functionality**

Key implementation files:
- `include/plotly/plotly.hpp` - Public API interface
- `src/plotly.cpp` - Main implementation
- `src/detail/websockets_server.cpp` - WebSocket server using websocketpp
- `src/detail/http_server.cpp` - HTTP server using httplib
- `src/detail/json_rpc.cpp` - JSON-RPC protocol implementation

#### WebSocket Server
Built on **websocketpp** library:
- **Asynchronous I/O** using Boost.Asio
- **Connection management** with automatic cleanup
- **Message routing** to JSON-RPC handler
- **Thread-safe** operation

#### HTTP Server

`HttpServer` is a class that provides a simple HTTP server for serving static files.
Built on **httplib** library:
- **Static file serving** from webapp directory
- **Dynamic port allocation** to prevent conflicts
- **WebSocket port endpoint** for client discovery
- **Lifecycle management** tied to Figure lifetime

#### JSON-RPC Server
Protocol implementation providing:
- **Method registration** and dispatch
- **Response correlation** using request IDs
- **Error handling** with standard error codes
- **Notification support** for events

### JavaScript Frontend Components

#### Entry Point (`index.js`)
- **WebSocket URL discovery** via HTTP endpoint `/ws_port`
- **DOM element binding** for plot container
- **Client initialization** and connection establishment

#### JSON-RPC Client (`json-rpc-client.js`)
Sophisticated WebSocket client with:
- **Connection management** with automatic reconnection
- **Request/response correlation** using promise-based API
- **Error handling** with custom error types
- **Method registration** for handling C++ calls
- **Timeout handling** for long-running requests

Key features:
```javascript
class JsonRpcClient {
  async call(method, params, timeoutMs)     // Call C++ methods
  notify(method, params)                    // Send notifications to C++
  registerHandler(method, handler)          // Handle C++ method calls
  async connect()                           // Establish WebSocket connection
}
```

#### Plotly.js Binding (`render-plotly.js`)
Direct API mapping between JSON-RPC calls and Plotly.js:

- **Method Handlers**: Each Plotly.js function has a corresponding JSON-RPC handler:
  - `Plotly.newPlot` → `"Plotly.newPlot"` handler
  - `Plotly.update` → `"Plotly.update"` handler
  - `Plotly.extendTraces` → `"Plotly.extendTraces"` handler
  - etc.

- **Event Integration**: Bidirectional event handling:
  ```javascript
  client.registerHandler("Plotly.on", (params) => {
    plotElement.on(params.event, (eventData) => {
      const sanitizedEventData = removeUnderscoreProperties(eventData);
      client.notify(params.eventId, sanitizedEventData);
    });
  });
  ```

- **Data Sanitization**: Removes circular references and internal properties before sending events to C++

## Key Design Patterns

### API Mirroring
Every Plotly.js method has a corresponding C++ method with identical semantics:

\dot
digraph APIMirroring {
    rankdir=LR;
    node [shape=record, style=filled];

    CppAPI [label="{C++ Figure API|+ newPlot(data, layout, config)\l+ update(traceUpdate, layoutUpdate)\l+ extendTraces(update, indices, maxPoints)\l+ on(event, callback)\l+ relayout(layout)\l+ restyle(aobj, traces)\l}", fillcolor=lightblue];

    JSApi [label="{JavaScript Plotly.js API|+ Plotly.newPlot(element, data, layout, config)\l+ Plotly.update(element, traceUpdate, layoutUpdate)\l+ Plotly.extendTraces(element, update, indices, maxPoints)\l+ element.on(event, callback)\l+ Plotly.relayout(element, layout)\l+ Plotly.restyle(element, aobj, traces)\l}", fillcolor=lightpink];

    CppAPI -> JSApi [label="1:1 API Mirroring", style=bold, color=red];

    note [shape=note, label="Provides familiar API for users\ntransitioning from JavaScript to C++", fillcolor=lightyellow];
    JSApi -> note [style=dotted];
}
\enddot

### Real-time Data Streaming
The `extendTraces()` method enables efficient real-time data updates:
- **Incremental updates** without full re-rendering
- **Automatic animation** of new data points
- **Memory management** with configurable point limits
- **High-frequency updates** suitable for live monitoring

### Event-Driven Architecture
Bidirectional events enable rich interactivity:

\msc
User [label="User"],
Plotly [label="Plotly.js"],
Events [label="Event System"],
App [label="C++ Application"];

User => Plotly [label="Click/Hover/Zoom"];
Plotly => Events [label="plotly_click, plotly_hover, etc."];
Events => App [label="C++ callback with event data"];

App => Events [label="Trigger custom event"];
Events => Plotly [label="Update visualization"];
Plotly => User [label="Visual feedback"];

Events note Events [label="Bidirectional event flow enables\nrich interactive applications"];
\endmsc

### Browser Integration
Automatic browser management simplifies deployment:
- **Process spawning** with configurable Chrome/Chromium paths
- **Headless mode** support for server environments
- **Download directory** configuration via Chrome DevTools Protocol
- **Clean shutdown** with proper process cleanup

## Thread Safety and Concurrency

### WebSocket Server
- **Thread-safe** message handling using websocketpp's async model
- **Connection state** managed with proper synchronization
- **Message queuing** handled by underlying websocketpp implementation

### HTTP Server
- **Multi-threaded** request handling via httplib
- **Stateless design** for static file serving
- **Thread-safe** port allocation and lifecycle management

### Figure API
- **Single-threaded** client usage recommended
- **Internal synchronization** for WebSocket and HTTP servers
- **Async operations** return quickly without blocking

## Error Handling

### JSON-RPC Error Codes
Standard JSON-RPC 2.0 error codes:
- `-32700`: Parse error (invalid JSON)
- `-32600`: Invalid request
- `-32601`: Method not found
- `-32602`: Invalid parameters
- `-32603`: Internal error
- `-32000`: Server error

### Connection Management
- **Automatic reconnection** with exponential backoff
- **Timeout handling** for long-running requests
- **Graceful degradation** when browser disconnects
- **Resource cleanup** on connection loss

### Browser Process Management
- **Process monitoring** for browser crashes
- **Port conflict resolution** through dynamic allocation
- **Launch failure handling** with fallback options

This architecture provides a robust, scalable foundation for creating interactive data visualizations in C++ applications while leveraging the full power of Plotly.js in the browser.
