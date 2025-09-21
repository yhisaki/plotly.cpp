# Test configuration for plotly.cpp WebSocket components

cmake_minimum_required(VERSION 3.10)

# Include FetchContent module
include(FetchContent)

# Enable testing
enable_testing()

# Try to find Google Test, fall back to FetchContent if not found
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(STATUS "Google Test not found, using FetchContent")
    # Fetch Google Test
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
        GIT_SHALLOW TRUE
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    message(STATUS "Using FetchContent for Google Test")
else()
    message(STATUS "Found system Google Test")
endif()

# Try to find glog, fall back to FetchContent if not found
find_package(glog QUIET)
if(NOT glog_FOUND)
    message(STATUS "glog not found, using FetchContent")
    FetchContent_Declare(
        glog
        GIT_REPOSITORY https://github.com/google/glog.git
        GIT_TAG v0.6.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glog)
    message(STATUS "Using FetchContent for glog")
else()
    message(STATUS "Found system glog")
endif()

# Find all test source files
file(GLOB TEST_SOURCES "test_*.cpp")

# Create test executable with all test files
add_executable(test_main ${TEST_SOURCES})

# Link libraries
if(GTest_FOUND)
    target_link_libraries(test_main
        PRIVATE
        plotly-cpp
        GTest::gtest
        GTest::gtest_main
    )
else()
    target_link_libraries(test_main
        PRIVATE
        plotly-cpp
        gtest
        gtest_main
    )
endif()

# Link glog
if(glog_FOUND)
    target_link_libraries(test_main PRIVATE glog::glog)
else()
    target_link_libraries(test_main PRIVATE glog)
endif()

# Improve stack traces on Linux
target_link_options(test_main PRIVATE -rdynamic)

# Include directories
target_include_directories(test_main PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/detail/third_party
)


# Add compiler flags for tests
target_compile_options(test_main PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -g
)

# Discover and register tests with CTest
include(GoogleTest)
gtest_discover_tests(test_main)

# Add custom test target for convenience
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_main
    COMMENT "Running all tests"
)

# Add individual test cases as separate CTest tests
gtest_add_tests(TARGET test_main
    TEST_SUFFIX .noArgs
    TEST_LIST noArgsTests
)
