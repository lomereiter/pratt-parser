cmake_minimum_required (VERSION 2.6)
set (PROJECT pascal_parser)

project (${PROJECT})

include_directories (${PROJECT_SOURCE_DIR}/include)
include_directories (${PROJECT_SOURCE_DIR}/../parser)

set (HEADERS
        ../parser/symbol.h
        ../parser/symbol_impl.h
        ../parser/token.h
        ../parser/token_impl.h
        ../parser/grammar.h
        ../parser/grammar_impl.h
        ../parser/parser_core.h
        ../parser/parser_core_impl.h
        ../parser/parser.h
        ../parser/parser_impl.h
        include/operator.h
        include/syntax_error.h
        include/ast_visitors.h
        include/node.h
        include/node_fwd.h
        include/visitor.h
        include/node_tags.h
        include/node_traits.h
        include/pascal_grammar.h
        include/pretty_printer.h
        include/pascal_literals.h
        include/pascal_handlers.h
        )

set (SOURCES
        src/templ_insts.cpp
        src/pascal_grammar.cpp
        src/pascal_literals.cpp
        src/handlers/literals.cpp
        src/handlers/operators.cpp
        src/handlers/sections.cpp
        src/handlers/types.cpp
        src/handlers/expressions.cpp
        src/node.cpp
        src/node_tags.cpp
        src/node_traits.cpp
        src/pretty_printer.cpp
        src/test.cpp
        src/operator.cpp
        )

set (CMAKE_CXX_COMPILER "g++-4.6")

set (CMAKE_CXX_FLAGS "-std=c++0x -O2 -Wall -pedantic" )

add_executable (pp_gen pretty_printer_gen/pp_gen.cpp src/pascal_literals.cpp)

add_custom_command (
    OUTPUT ${PROJECT_SOURCE_DIR}/include/pretty_printer.h ${PROJECT_SOURCE_DIR}/src/pretty_printer.cpp
    COMMAND pp_gen ${PROJECT_SOURCE_DIR}/src/pretty_printer.npp ${PROJECT_SOURCE_DIR}
    DEPENDS ${PROJECT_SOURCE_DIR}/src/pretty_printer.npp pp_gen
    )

add_custom_target (
    pretty_printer DEPENDS ${PROJECT_SOURCE_DIR}/include/pretty_printer.h 
                           ${PROJECT_SOURCE_DIR}/src/pretty_printer.cpp
                  )

add_executable (${PROJECT} ${HEADERS} ${SOURCES})
