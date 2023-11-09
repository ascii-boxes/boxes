cmake_minimum_required(VERSION 3.27)

project(boxes LANGUAGES C)


set(BVERSION "2.2.2-dev")
set(GLOBALCONF "/usr/share/boxes")

find_package(Git)
if (GIT_EXECUTABLE)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --always
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DESCRIBE_OUTPUT
        RESULT_VARIABLE GIT_DESCRIBE_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT GIT_DESCRIBE_ERROR)
        set(BVERSION ${GIT_DESCRIBE_OUTPUT})
    endif()
endif ()

add_custom_command(  # TODO: use configure_file?
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/boxes.h
    COMMAND sed -e "s|--BVERSION--|${BVERSION}|; s|--GLOBALCONF--|${GLOBALCONF}|"
                ${CMAKE_CURRENT_SOURCE_DIR}/src/boxes.in.h > ${CMAKE_CURRENT_BINARY_DIR}/boxes.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/boxes.in.h
    VERBATIM
)


find_package(libunistring REQUIRED)
find_package(PCRE2 REQUIRED)
find_package(Curses REQUIRED)

find_package(BISON REQUIRED)
bison_target(boxes_parser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser.c
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h
    COMPILE_FLAGS "--warnings=all --verbose"
)

find_package(FLEX REQUIRED)
flex_target(boxes_scanner
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.h
)
add_flex_bison_dependency(boxes_scanner boxes_parser)


set(TARGET ${PROJECT_NAME})

set(GENERATED_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/boxes.h
    ${BISON_boxes_parser_OUTPUTS}
    ${FLEX_boxes_scanner_OUTPUTS}
)

set(SOURCES
    src/boxes.c
    src/bxstring.c
    src/cmdline.c
    src/discovery.c
    src/generate.c
    src/input.c
    src/list.c
    src/parsecode.c
    src/parsing.c
    src/query.c
    src/regulex.c
    src/remove.c
    src/shape.c
    src/tools.c
    src/unicode.c
)

add_executable(${TARGET}
    ${SOURCES}
    ${GENERATED_FILES}
)
target_include_directories(${TARGET}
    PRIVATE
        ./src
        ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(${TARGET}
    libunistring::libunistring
    pcre2::pcre2
    Curses::Curses
    ${FLEX_LIBRARIES}
)
