# Third Party

This directory contains third party libraries used in the project.

## Libraries

- [**WebSocket++**](https://github.com/zaphoyd/websocketpp): WebSocket client/server implementation
- [**cpp-httplib**](https://github.com/yhirose/cpp-httplib): HTTP server implementation

## Update

Run `./update_third_party.sh` to update libraries to default commit hashes.

Use environment variables to specify commit hashes:

```bash
WEBSOCKETPP_COMMIT=b9aeec6eaf3d5610503439b4fae3581d9aff08e8 CPP_HTTPLIB_COMMIT=89c932f313c6437c38f2982869beacc89c2f2246 ./update_third_party.sh
```
