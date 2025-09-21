# Create utils library
add_library(gallery_utils STATIC
    utils/linspace.cpp
    utils/arg_parser.cpp
    utils/data_generators.cpp
)

target_include_directories(gallery_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Gather all gallery source files
file(GLOB GALLERY_SOURCES "*.cpp")

# Create an executable for each gallery
foreach(gallery_source ${GALLERY_SOURCES})
    get_filename_component(gallery_name ${gallery_source} NAME_WE)
    add_executable(${gallery_name} ${gallery_source})

    target_link_libraries(${gallery_name} PRIVATE
        plotly-cpp
        gallery_utils
    )

    target_include_directories(${gallery_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )
endforeach()
