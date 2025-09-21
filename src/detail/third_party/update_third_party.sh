# Use environment variables if set, otherwise use default commit hashes
WEBSOCKETPP_COMMIT=${WEBSOCKETPP_COMMIT:-b9aeec6eaf3d5610503439b4fae3581d9aff08e8}
CPP_HTTPLIB_COMMIT=${CPP_HTTPLIB_COMMIT:-89c932f313c6437c38f2982869beacc89c2f2246}

# Remove existing files
rm -f httplib.h
rm -rf websocketpp

git clone https://github.com/zaphoyd/websocketpp.git websocketpp-temp
cd websocketpp-temp
git checkout ${WEBSOCKETPP_COMMIT}
cd ..
mv websocketpp-temp/websocketpp .
rm -rf websocketpp-temp


git clone https://github.com/yhirose/cpp-httplib.git cpp-httplib-temp
cd cpp-httplib-temp
git checkout ${CPP_HTTPLIB_COMMIT}
cd ..
mv cpp-httplib-temp/httplib.h .
rm -rf cpp-httplib-temp

