# mandel/extern

cmake_minimum_required(VERSION 3.13)

#add_library(doctest INTERFACE)
#target_include_directories(doctest INTERFACE doctest/)

add_library(cxxopts INTERFACE)
target_include_directories(cxxopts INTERFACE cxxopts/)

add_subdirectory(angelscript/angelscript/projects/cmake)

add_subdirectory(angelscript/add_on)

add_library(cereal INTERFACE)
target_include_directories(cereal INTERFACE cereal/)
