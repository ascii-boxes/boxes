#============================================================================#
#                               CMakeLists.txt                               #
#                                                                            #
#                                                                            #
# Cross platform build manager, used by Travis and Appveyor to ensure the    #
# correct windows/linux/OSX builds                                           #
#============================================================================#

cmake_minimum_required(VERSION 3.2)
project(boxes
        VERSION 1.1.3
        LANGUAGES C)

# direct the user to our helpful team of experts
message(STATUS "")
message(STATUS " For compilation info please refer to the boxes compilation FAQ")
message(STATUS " at http://boxes.thomasjensen.com/docs/faq.html#q5")
message(STATUS "")

# Environment vars used in this build
set(BOXES_EXE boxes)
set(BOXES_CFG "${CMAKE_SOURCE_DIR}/boxes-config")

# Testing framework
add_subdirectory(${CMAKE_SOURCE_DIR}/test)

# generate boxes.h from boxes.h.in and boxes.1 from boxes.1.in, replacing some placeholders
set(boxes_GLOBALCONF "/usr/share/boxes")
configure_file("${CMAKE_SOURCE_DIR}/src/boxes.h.in" "${CMAKE_BINARY_DIR}/src/boxes.h" @ONLY)
configure_file("${CMAKE_SOURCE_DIR}/doc/boxes.1.in" "${CMAKE_BINARY_DIR}/doc/boxes.1" @ONLY)

# compile the config parser
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
bison_target(boxesParser ${CMAKE_SOURCE_DIR}/src/parser.y ${CMAKE_BINARY_DIR}/src/parser.c)
flex_target(boxesScanner ${CMAKE_SOURCE_DIR}/src/lexer.l ${CMAKE_BINARY_DIR}/src/lex.yy.c)
add_flex_bison_dependency(boxesScanner boxesParser)

# add the source directories to the include path, they should be ebfore system path, since some of them spoof real system includes on linux
include_directories(BEFORE ${CMAKE_BINARY_DIR}/src)
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src)
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/misc)
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/regexp)

# sources for the boxes executable
add_executable(${BOXES_EXE}
    src/misc/getopt.c
    src/boxes.c
    src/generate.c
    src/remove.c
    src/shape.c
    src/tools.c
    src/regexp/regexp.c
    src/regexp/regsub.c
    ${BISON_boxesParser_OUTPUTS}
    ${FLEX_boxesScanner_OUTPUTS}
)

