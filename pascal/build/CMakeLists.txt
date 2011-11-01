cmake_minimum_required (VERSION 2.6)
set (PROJECT pascal_parser)

project (${PROJECT})

set (HEADERS
        ../../parser/symbol.h
        ../../parser/symbol_impl.h
        ../../parser/token.h
        ../../parser/token_impl.h
        ../../parser/grammar.h
        ../../parser/grammar_impl.h
        ../../parser/parser_core.h
        ../../parser/parser_core_impl.h
        ../../parser/parser.h
        ../../parser/parser_impl.h
        ../operator.h
        ../syntax_error.h
        ../ast_visitors.h
        ../expression.h
        ../expression_fwd.h
        ../visitor.h
        ../node_tags.h
        ../node_traits.h
        ../pascal_grammar.h
        ../pretty_printer.h
        )

set (SOURCES
        ../pascal_grammar.cpp
        ../node_tags.cpp
        ../templ_insts.cpp
        ../pretty_printer.cpp
        ../test.cpp
        )

set (CMAKE_CXX_COMPILER "/usr/bin/g++-4.6")

set (CMAKE_CXX_FLAGS "-std=c++0x -g -Wall" )

set (CMAKE_CXX_FLAGS_DEBUG
        -ggdb
        )

add_executable (${PROJECT} ${HEADERS} ${SOURCES})
