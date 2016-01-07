cmake_minimum_required(VERSION 3.3)
project(boxes
    VERSION 1.1.3
    LANGUAGES C)

# The following line gives the default for the system-wide configuration file:
set(boxes_GLOBALCONF "/usr/share/boxes")

message(STATUS "")
message(STATUS " For compilation info please refer to the boxes compilation FAQ")
message(STATUS " at http://boxes.thomasjensen.com/docs/faq.html#q5")
message(STATUS "")

# the root directory of our project
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})

set(PROJECT_SOURCE_DIR ${PROJECT_ROOT}/src)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_ROOT}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_ROOT}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_ROOT}/bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -W")

# generate boxes.h from boxes.h.in and boxes.1 from boxes.1.in, replacing some placeholders
configure_file("${PROJECT_SOURCE_DIR}/boxes.h.in" "${PROJECT_BINARY_DIR}/boxes.h" @ONLY)
configure_file("${PROJECT_ROOT}/doc/boxes.1.in" "${PROJECT_BINARY_DIR}/boxes.1" @ONLY)

include_directories("${PROJECT_SOURCE_DIR}/regexp")
add_subdirectory(${PROJECT_SOURCE_DIR}/regexp)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
bison_target(boxesParser ${PROJECT_SOURCE_DIR}/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
flex_target(boxesScanner ${PROJECT_SOURCE_DIR}/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)
add_flex_bison_dependency(boxesScanner boxesParser)

message("CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}/lexer.c")

#configure_file(${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c ${CMAKE_CURRENT_BINARY_DIR}/lexer.tmp.c COPYONLY)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lexer.c
    COMMENT "Prepending config.h include to generated lexer ..."
    COMMAND echo "#include \"config.h\"" > ${CMAKE_CURRENT_BINARY_DIR}/lexer.c
    COMMAND cat ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c >> ${CMAKE_CURRENT_BINARY_DIR}/lexer.c
    MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
    DEPENDS ${PROJECT_SOURCE_DIR}/lexer.l
)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

set(SOURCE_FILES
    src/misc/getopt.c
    src/misc/getopt.h
    src/boxes.c
    src/config.h
    src/generate.c
    src/generate.h
    lexer.c
    src/lexer.h
    parser.c
    parser.h
    src/remove.c
    src/remove.h
    src/shape.c
    src/shape.h
    src/tools.c
    src/tools.h)

add_executable(boxes ${SOURCE_FILES} ${BISON_boxesParser_OUTPUTS} ${FLEX_boxesScanner_OUTPUTS})
target_link_libraries(boxes ${FLEX_LIBRARIES} ${BISON_LIBRARIES} regexp)
