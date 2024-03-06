# CMake 3.14 is relatively new, and it is the first version to support
# Visual Studio 2019.
cmake_minimum_required(VERSION 3.14)

project(
    modern_bloom
    DESCRIPTION "Bloom filters for modern C++"
    VERSION 0.2
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(modern_bloom INTERFACE)
add_library(modern_bloom::modern_bloom ALIAS modern_bloom)

target_sources(
    modern_bloom
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/bloom_filter.hpp>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/bloom_filter.hpp>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/dynamic_bloom.hpp>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/static_bloom.hpp>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/internal/utils.hpp>
)

target_include_directories(
    modern_bloom
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/modern_bloom>
)

target_compile_features(modern_bloom INTERFACE cxx_std_17)

write_basic_package_version_file(
    modern_bloom-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
    ARCH_INDEPENDENT
)

install(
    TARGETS modern_bloom
    EXPORT modern_bloom-targets
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/modern_bloom
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/modern_bloom-config.cmake.in
    modern_bloom-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/modern_bloom
)

export(
    EXPORT modern_bloom-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/modern_bloom-targets.cmake
    NAMESPACE modern_bloom::
)

install(
    EXPORT modern_bloom-targets
    FILE modern_bloom-targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/modern_bloom
    NAMESPACE modern_bloom::
)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/modern_bloom-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/modern_bloom-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/modern_bloom
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/modern_bloom
)

export(PACKAGE modern_bloom)

option(MB_BUILD_TESTS "Build tests" ON)

if(MB_BUILD_TESTS)
    add_subdirectory(test)
endif()

option(MB_BUILD_DOCS "Build documentation" ON)

if(MB_BUILD_DOCS)
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
