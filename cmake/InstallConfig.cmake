# plotly.cpp - Installation Configuration This module handles all installation
# and packaging logic

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Configure installation for plotly.cpp
function(configure_plotly_installation)
  # Install library targets
  install(
    TARGETS plotly-cpp plotly-cpp-static
    EXPORT plotly-cppTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    INCLUDES
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  # Install headers
  install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT devel
    FILES_MATCHING
    PATTERN "*.hpp"
    PATTERN "third_party" EXCLUDE)

  # Install generated version header
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/include/plotly/version.hpp"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/plotly
    COMPONENT devel)

  # Install webapp directory
  install(
    DIRECTORY webapp/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/plotly/webapp
    COMPONENT runtime
    FILES_MATCHING
    PATTERN "*.html"
    PATTERN "*.js"
    PATTERN "*.css"
    PATTERN "*.json")

  # Create and install CMake config files
  install(
    EXPORT plotly-cppTargets
    FILE plotly-cppTargets.cmake
    NAMESPACE plotly-cpp::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/plotly-cpp
    COMPONENT devel)

  # Generate and install package config files
  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/plotly-cppConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/plotly-cppConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/plotly-cpp
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_DATAROOTDIR)

  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/plotly-cppConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/plotly-cppConfig.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/plotly-cppConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/plotly-cpp
    COMPONENT devel)

  message(STATUS "Installation configuration complete")
endfunction()
