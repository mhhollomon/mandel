# mandel/src/proto
cmake_minimum_required(VERSION 3.13)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS fractal.proto)

add_library(fractal-file)

target_sources(fractal-file
    PRIVATE
        ../lib/fractal_file.cpp
        ${PROTO_HDRS}
        ${PROTO_SRCS}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/fractal_file.hpp"
    )
target_link_libraries(fractal-file 
    PUBLIC
        ${Protobuf_LIBRARIES}
        lib-include
    )

target_include_directories(fractal-file
    PUBLIC
        lib-include
        ${CMAKE_CURRENT_BINARY_DIR}
    )
