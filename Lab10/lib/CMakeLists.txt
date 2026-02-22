add_library(itmoscript
    ${CMAKE_SOURCE_DIR}/bin/lexer.cpp
    ${CMAKE_SOURCE_DIR}/bin/parser.cpp
    ${CMAKE_SOURCE_DIR}/bin/interpreter.cpp
)

target_include_directories(itmoscript
    PUBLIC
        ${CMAKE_SOURCE_DIR}/lib
)