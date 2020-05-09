# mandel/src/fractalator
cmake_minimum_required(VERSION 3.13)


add_library(fractalator-lib STATIC)

target_sources(fractalator-lib
    PRIVATE
        fractalator.cpp

        ../include/compute.hpp
        ../include/fractal_file.hpp
        ../include/fractalator.hpp
    )

target_link_libraries(fractalator-lib
    PRIVATE
        lib-include
        lib_objlib
    )
