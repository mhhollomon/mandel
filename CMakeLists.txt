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

add_compile_options("-g;-O2;-Wall;-pedantic")

add_subdirectory(extern)

add_subdirectory(src)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)

link_directories(
    ${GTKMM_LIBRARY_DIRS}  )

include_directories(
    ${GTKMM_INCLUDE_DIRS}  )

#
##
## fractalator application
##
add_executable(fractalator)

target_sources(fractalator
    PRIVATE
        src/fractalator/fractalator_main.cpp
    )

target_link_libraries(fractalator
    PRIVATE
        cxxopts
        lib_objlib
        lib-include
        fractalator-lib
    )

##
## colorator application
##
add_executable(colorator)

target_sources(colorator
    PRIVATE
        src/colorator/colorator_main.cpp
    )

target_link_libraries(colorator
    PRIVATE
        cxxopts
        lib_objlib
        lib-include
        colorator-lib
    )


##
## mandel application
##
add_executable(mandel)

target_sources(mandel
    PRIVATE
        "src/mandel/mandel_main.cpp"
    )

target_link_libraries(mandel
    PRIVATE
        cxxopts
        lib_objlib
        lib-include
        fractalator-lib
        colorator-lib
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
