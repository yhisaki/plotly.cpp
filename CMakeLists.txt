cmake_minimum_required(VERSION 3.14)  # Required for stable FetchContent

project(plotly-cpp VERSION 0.1.0 LANGUAGES CXX)

enable_testing()

# Include cmake modules
include(cmake/InstallConfig.cmake)
include(cmake/PackageConfig.cmake)
include(FetchContent)

# Configure version header
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/plotly/version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/plotly/version.hpp"
    @ONLY
)

# Build options
option(PLOTLY_CPP_BUILD_GALLERY "Build example gallery applications" OFF)
option(PLOTLY_CPP_BUILD_TESTS "Build test suite" OFF)

# Display configuration summary
message(STATUS "plotly.cpp configuration:")
message(STATUS "  Build gallery: ${PLOTLY_CPP_BUILD_GALLERY}")
message(STATUS "  Build tests: ${PLOTLY_CPP_BUILD_TESTS}")

find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
    message(STATUS "Using FetchContent for nlohmann_json")
endif()


# Define source files explicitly (better than GLOB for reproducibility)
set(PLOTLY_SOURCES
    src/plotly.cpp
    src/logger.cpp
    src/detail/browser.cpp
    src/detail/http_server.cpp
    src/detail/json_rpc.cpp
    src/detail/uuid.cpp
    src/detail/websockets_client.cpp
    src/detail/websockets_endpoint.cpp
    src/detail/websockets_server.cpp
)

# Function to configure common target settings (eliminates duplication)
function(configure_plotly_target target_name)
    # Set webapp path for both build and install contexts
    set(WEBAPP_PATH_BUILD "${CMAKE_CURRENT_SOURCE_DIR}/webapp/")
    set(WEBAPP_PATH_INSTALL "/usr/share/plotly/webapp/")

    target_compile_definitions(${target_name}
        PUBLIC
        PLOTLY_CPP_WEBAPP_PATH="$<BUILD_INTERFACE:${WEBAPP_PATH_BUILD}>$<INSTALL_INTERFACE:${WEBAPP_PATH_INSTALL}>"
        PRIVATE
        CPPHTTPLIB_THREAD_POOL_COUNT=3
    )

    target_link_libraries(${target_name}
        PRIVATE
            pthread
        PUBLIC
            nlohmann_json::nlohmann_json
    )

    target_include_directories(${target_name}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/src/detail/third_party
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    # Configure compiler settings
endfunction()

# Create libraries
add_library(plotly-cpp SHARED ${PLOTLY_SOURCES})
add_library(plotly-cpp-static STATIC ${PLOTLY_SOURCES})

# Apply common configuration
configure_plotly_target(plotly-cpp)
configure_plotly_target(plotly-cpp-static)

# Create alias targets for modern CMake usage
add_library(plotly-cpp::plotly-cpp ALIAS plotly-cpp)
add_library(plotly-cpp::plotly-cpp-static ALIAS plotly-cpp-static)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gallery" AND PLOTLY_CPP_BUILD_GALLERY)
    add_subdirectory(gallery)
endif()

# Add tests subdirectory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test" AND PLOTLY_CPP_BUILD_TESTS)
    add_subdirectory(test)
endif()

# Configure installation
configure_plotly_installation()

# Include CPack for package generation
include(CPack)
