# mandel/src
cmake_minimum_required(VERSION 3.13)

##
## lib-include
##
add_library(lib-include INTERFACE)

target_include_directories(lib-include
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

##
## lib_objlib
##
add_library(lib_objlib OBJECT)

target_sources(lib_objlib
    PRIVATE
        "lib/bmp_file.cpp"
        "lib/compute.cpp"
        lib/script_engine.cpp
        lib/pixel.cpp
    PUBLIC
        include/bmp_file.hpp
        include/compute.hpp
        include/script_engine.hpp
        include/pixel.hpp
    )

target_link_libraries(lib_objlib 
    PUBLIC
        lib-include
        angelscript
        angelscript-addons
    )

target_include_directories(lib_objlib
    PUBLIC
        ../extern/angelscript/angelscript/include
    )

add_subdirectory(proto)

