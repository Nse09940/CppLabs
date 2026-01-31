# lib/CMakeLists.txt

# Собираем статическую (или динамическую, если хотите) библиотеку itmoscript
# из трёх исходников, лежащих в ../bin:
#    bin/lexer.cpp
#    bin/parser.cpp
#    bin/interpreter.cpp

add_library(itmoscript
    # Путь относительно lib/ к cpp-файлам:
    # один уровень вверх, потом bin/…
    ${CMAKE_SOURCE_DIR}/bin/lexer.cpp
    ${CMAKE_SOURCE_DIR}/bin/parser.cpp
    ${CMAKE_SOURCE_DIR}/bin/interpreter.cpp
)

# Сообщаем, где находятся заголовочные файлы:
target_include_directories(itmoscript
    PUBLIC
        ${CMAKE_SOURCE_DIR}/lib
)

# (Опционально) если внутри interpreter.cpp или других используются какие-то
# системные библиотеки, их тоже можно добавить здесь, например:
# target_link_libraries(itmoscript PUBLIC pthread)
