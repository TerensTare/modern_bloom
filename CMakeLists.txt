# CMake 3.14 is relatively new, and it is the first version to support
# Visual Studio 2019.
cmake_minimum_required(VERSION 3.14)

project(modern_bloom)

add_library(modern_bloom INTERFACE)
add_library(modern_bloom::modern_bloom ALIAS modern_bloom)

target_sources(
    modern_bloom
    INTERFACE
    include/dynamic_bloom.hpp
    include/static_bloom.hpp
    include/internal/utils.hpp
)

target_include_directories(modern_bloom INTERFACE include)

target_compile_features(modern_bloom INTERFACE cxx_std_17)

add_subdirectory(test)
