# CMake 3.14 is relatively new, and it is the first version to support
# Visual Studio 2019.
cmake_minimum_required(VERSION 3.14)

project(
    modern_bloom
    DESCRIPTION "Bloom filters for modern C++"
    VERSION 0.2
)

add_library(modern_bloom INTERFACE)
add_library(modern_bloom::modern_bloom ALIAS modern_bloom)

target_sources(
    modern_bloom
    INTERFACE
    include/bloom_filter.hpp
    include/dynamic_bloom.hpp
    include/static_bloom.hpp
    include/internal/utils.hpp
)

target_include_directories(modern_bloom INTERFACE include)

target_compile_features(modern_bloom INTERFACE cxx_std_17)

add_subdirectory(test)

option(BUILD_DOCS "Build documentation" ON)

if(BUILD_DOCS)
    find_package(Doxygen)

    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${PROJECT_SOURCE_DIR}/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)
        set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/include)
        set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs)

        file(WRITE ${DOXYGEN_OUT} "")
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        add_custom_target(
            docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs
            COMMENT "Generating documentation with Doxygen"
            VERBATIM)
    else()
        message("Not generating documentation because Doxygen was not found")
    endif()
endif()
