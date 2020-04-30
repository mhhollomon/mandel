# mandel
cmake_minimum_required(VERSION 3.13)

project(Mandel VERSION 0.2.0
    DESCRIPTION "Mandelbrot thing"
             LANGUAGES CXX)

#
# Make sure we use -std=c++17 or higher
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options("-g;-O0;-Wall;-pedantic")

add_subdirectory(extern)

add_subdirectory(src)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)

link_directories(
    ${GTKMM_LIBRARY_DIRS}  )

include_directories(
    ${GTKMM_INCLUDE_DIRS}  )

##
## mandel application
##
###add_executable(mandel)
#
#target_sources(mandel
#    PRIVATE
#        "src/main.cpp"
#        "src/include/compute.hpp"
#    )
#
#target_link_libraries(mandel
#    PRIVATE
#        cxxopts
#        lib_objlib
#        lib-include
#    )
#
##
## fractalator application
##
add_executable(fractalator)

target_sources(fractalator
    PRIVATE
        "src/fractalator.cpp"
        "src/include/compute.hpp"
    )

target_link_libraries(fractalator
    PRIVATE
        cxxopts
        lib_objlib
        lib-include
        fractal-file
    )

##
## colorator application
##
add_executable(colorator)

target_sources(colorator
    PRIVATE
        src/colorator/colorator.cpp
        src/colorator/color_script_engine.cpp
        src/colorator/color_script_engine.hpp
        src/include/compute.hpp
        src/include/pixel.hpp
        src/include/bmp_file.hpp
    )

target_link_libraries(colorator
    PRIVATE
        cxxopts
        lib_objlib
        lib-include
        fractal-file
        angelscript-addons
    )


##
## gmandel application
##
#add_executable(gmandel)
#
#target_sources(gmandel
#    PRIVATE
#        "src/main-gtk.cpp"
#        "src/MandelWin.cpp"
#        "src/MandelApp.cpp"
#    )
##
#target_link_libraries(gmandel
#    PRIVATE
##        lib-include
#        cxxopts
#        ${GTKMM_LIBRARIES}
#        lib_objlib
#    )
