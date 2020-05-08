# mandel/src
cmake_minimum_required(VERSION 3.13)

##
## lib-include
##
add_library(lib-include INTERFACE)

target_include_directories(lib-include
    INTERFACE include)

    #    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

##
## lib_objlib
##
add_library(lib_objlib OBJECT)

target_sources(lib_objlib
    PRIVATE
        lib/bmp_file.cpp
        lib/compute.cpp
        lib/pixel.cpp
        lib/fractal_file.cpp
    PUBLIC
        include/bmp_file.hpp
        include/compute.hpp
        include/pixel.hpp
        include/fixed_array.hpp
        include/work_queue.hpp
        include/colorator.hpp
        include/fractal_file.hpp
    )

target_link_libraries(lib_objlib 
    PUBLIC
        lib-include
        cereal
    )
target_include_directories(lib_objlib
    PUBLIC
        cereal
    )


##
## script_engine
##
add_library(script_engine STATIC)

target_sources(script_engine
    PRIVATE
        lib/script_engine.cpp
    PUBLIC
        include/script_engine.hpp
)

target_link_libraries(script_engine
    PUBLIC
        lib-include
        angelscript
        angelscript-addons
    )

target_include_directories(script_engine
    PUBLIC
        ../extern/angelscript/angelscript/include
    )

