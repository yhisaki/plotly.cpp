# plotly.cpp - Package Configuration This module handles CPack configuration for
# creating Debian packages

# CPack configuration for deb package
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_MONOLITHIC_INSTALL OFF)

# Define components
set(CPACK_COMPONENTS_ALL runtime devel)

# Main package configuration
set(CPACK_PACKAGE_NAME "libplotly-cpp")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "C++ library for creating interactive plots with Plotly.js")
set(CPACK_PACKAGE_DESCRIPTION
    "Plotly.cpp brings the power of Plotly.js to C++. Most Plotly.js functions have direct C++ equivalents, making it a familiar plotting library for C++ developers."
)
set(CPACK_PACKAGE_CONTACT "Yukinari Hisaki <yhisaki31@gmail.com>")
set(CPACK_PACKAGE_VENDOR "Yukinari Hisaki")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/yhisaki/plotly.cpp")

# Debian-specific configuration
set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.17), libstdc++6 (>= 4.9)")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "nlohmann-json3-dev")

# Runtime component (shared library)
set(CPACK_DEBIAN_RUNTIME_PACKAGE_NAME "libplotly-cpp${PROJECT_VERSION_MAJOR}")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_RUNTIME_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_DEPENDS "libc6 (>= 2.17), libstdc++6 (>= 4.9)")

# Development component (headers, static library, cmake files)
set(CPACK_DEBIAN_DEVEL_PACKAGE_NAME "libplotly-cpp-dev")
set(CPACK_DEBIAN_DEVEL_PACKAGE_SECTION "libdevel")
set(CPACK_DEBIAN_DEVEL_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_DEVEL_PACKAGE_DEPENDS
    "libplotly-cpp${PROJECT_VERSION_MAJOR} (= \\\${binary:Version}), nlohmann-json3-dev"
)

# Architecture detection
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
else()
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "any")
endif()

# Exclude unnecessary files from package
set(CPACK_SOURCE_IGNORE_FILES
    "/\\\\.git/"
    "/\\\\.github/"
    "/docs/images/"
    "/test/"
    "/gallery/"
    "/build/"
    "/\\\\.vscode/"
    "/\\\\.idea/")

message(STATUS "CPack package configuration complete")
