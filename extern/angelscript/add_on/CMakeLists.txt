cmake_minimum_required(VERSION 3.13)

add_library(angelscript-addons OBJECT)


target_include_directories(angelscript-addons
    PRIVATE
        ../angelscript/include
    INTERFACE 
        .
    )

target_compile_options(angelscript-addons PRIVATE -fno-strict-aliasing)

target_sources(angelscript-addons
    PRIVATE
        scriptbuilder/scriptbuilder.cpp
        debugger/debugger.cpp
        scriptstdstring/scriptstdstring.cpp
        scriptstdstring/scriptstdstring_utils.cpp
        scriptarray/scriptarray.cpp
        scriptmath/scriptmath.cpp
        scriptmath/scriptmathcomplex.cpp
    )
