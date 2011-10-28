cmake_minimum_required (VERSION 2.6)
set (PROJECT pascal_parser)

project (${PROJECT})

set (HEADERS
        ../parser/symbol.h
        ../parser/token.h
        ../parser/grammar.h
        ../parser/parser_core.h
        ../parser/parser.h
        ../pascal/operator.h
        ../pascal/syntax_error.h
        ../pascal/ast_visitors.h
        ../pascal/expression.h
        ../pascal/visitor.h
        ../pascal/node_tags.h
        ../pascal/node_traits.h
        )

set (SOURCES
        ../pascal/pascal_ast.cpp
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
