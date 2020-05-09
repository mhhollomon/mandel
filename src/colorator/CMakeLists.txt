# mandel/src/colorator
cmake_minimum_required(VERSION 3.13)


add_library(colorator-lib STATIC)

target_sources(colorator-lib
    PRIVATE
        colorator.cpp
        color_script_engine.cpp

        ../include/compute.hpp
        ../include/pixel.hpp
        ../include/bmp_file.hpp
    INTERFACE
        color_script_engine.hpp
)

target_link_libraries(colorator-lib
    PRIVATE
        lib-include
        lib_objlib
        script_engine
    )
