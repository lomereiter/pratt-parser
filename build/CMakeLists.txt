cmake_minimum_required (VERSION 2.6)
set (PROJECT calculator)

project (${PROJECT})

set (HEADERS
        ../symbol.h
        ../token.h
        ../grammar.h
        ../parser_core.h
        ../parser.h
        )

set (SOURCES
        ../calculator.cpp
        )

set (CMAKE_CXX_COMPILER
        g++-4.6
        )

set (CMAKE_CXX_FLAGS
        -std=c++0x
        )

set (CMAKE_CXX_FLAGS_DEBUG
        -ggdb
        )

add_executable (${PROJECT} ${HEADERS} ${SOURCES})
