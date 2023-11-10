cmake_minimum_required(VERSION 3.23)

# Build options options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(DISABLE_TESTING "Disable test compilation" OFF)
# TODO: Currently testing is only available on these platforms
set(TESTING_PLATFORMS "Linux" "MSYS")
option(ENABLE_COVERAGE "Enable coverage measurement" OFF)

# Project and generated target name.
project(boxes LANGUAGES C)

# =============================================================================

# Boxes version, fallback in case not in a boxes Git repository.
set(BVERSION "2.2.2-dev")
set(GLOBALCONF "/usr/share/boxes")

# Get version from Git repository (tags).
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

# Generate boxes.h include file.
# TODO: Use CMake "configure_file" <https://cmake.org/cmake/help/latest/command/configure_file.html>.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/boxes.h
    COMMAND sed -e "s|--BVERSION--|${BVERSION}|; s|--GLOBALCONF--|${GLOBALCONF}|"
                ${CMAKE_CURRENT_SOURCE_DIR}/src/boxes.in.h > ${CMAKE_CURRENT_BINARY_DIR}/boxes.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/boxes.in.h
    COMMENT "Generating boxes.h"
    VERBATIM
)

# Find packages (probably needs some Conan packages installed, i.e., run "conan install" before).
find_package(libunistring REQUIRED)
find_package(PCRE2 REQUIRED)
find_package(Curses REQUIRED)

# Generate parser.
find_package(BISON REQUIRED)
bison_target(boxes_parser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser.c
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h
    COMPILE_FLAGS "--warnings=all --verbose"
)

# Generate lexer.
find_package(FLEX REQUIRED)
flex_target(boxes_scanner
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.h
)
add_flex_bison_dependency(boxes_scanner boxes_parser)

# =============================================================================

# The executable target name is the project name.
# Note that a static library with the project name and suffix "_lib" is
# built first, and used for testing as well as linking the executable.
set(TARGET ${PROJECT_NAME})

# Includes for both, the static library and the executable.
set(INCLUDES
    ./src
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Libraries that are used to compile the static library, and link the executable.
set(LIBRARIES
    libunistring::libunistring
    pcre2::pcre2
    Curses::Curses
)

# List all generated files.
set(GENERATED_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/boxes.h
    ${BISON_boxes_parser_OUTPUTS}
    ${FLEX_boxes_scanner_OUTPUTS}
)

# Source files for the static library.
set(SOURCES_LIB
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

# Source files for only the exectuable (i.e., containing "main").
set(SOURCES_EXE
    src/boxes.c
)

# -----------------------------------------------------------------------------

# Build static library.
add_library(${TARGET}_lib STATIC
    ${GENERATED_FILES}
    ${SOURCES_LIB}
)
target_compile_features(${TARGET}_lib
    PUBLIC c_std_99
)
target_include_directories(${TARGET}_lib
    PUBLIC ${INCLUDES}
)
target_link_libraries(${TARGET}_lib
    PUBLIC ${LIBRARIES}
)

# Build boxes exectuable.
add_executable(${TARGET}
    ${SOURCES_EXE}
)
target_compile_features(${TARGET}_lib
    PRIVATE c_std_99
)
target_include_directories(${TARGET}
   PRIVATE ${INCLUDES}
)
target_link_libraries(${TARGET}
    PRIVATE
        ${TARGET}_lib
        ${LIBRARIES}
)

# -----------------------------------------------------------------------------

# Check and include unit tests.
if ((NOT DISABLE_TESTING) AND (CMAKE_SYSTEM_NAME IN_LIST TESTING_PLATFORMS))
    enable_testing()
    add_subdirectory(utest)
endif ()

# =============================================================================

# Create man page
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/boxes.1
    COMMAND sed -e "s|--BVERSION--|${BVERSION}|; s|--GLOBALCONF--|${GLOBALCONF}|"
                ${CMAKE_SOURCE_DIR}/doc/boxes.1.in > ${CMAKE_BINARY_DIR}/boxes.1
    DEPENDS ${CMAKE_SOURCE_DIR}/doc/boxes.1.in
    COMMENT "Generating man page"
    VERBATIM
)
add_custom_target(
    boxes_man_page ALL
    DEPENDS ${CMAKE_BINARY_DIR}/boxes.1
)

# -----------------------------------------------------------------------------

# Installing
install(
    TARGETS ${TARGET}
)
install(
    FILES ${CMAKE_BINARY_DIR}/boxes.1
    DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1
)

# Packaging
set(CPACK_PACKAGE_NAME "boxes")
set(CPACK_PACKAGE_VERSION ${BVERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Command line ASCII boxes")
set(CPACK_PACKAGE_DESCRIPTION "Boxes is a command line program which draws, removes, and repairs ASCII art boxes.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://boxes.thomasjensen.com")
set(CPACK_PACKAGE_VENDOR "Thomas Jensen and the boxes contributors")
set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README.md)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_INSTALL ${CMAKE_BINARY_DIR}/boxes.1)
set(CPACK_SET_DESTDIR true)
set(CPACK_GENERATOR "ZIP" "TGZ") # "DEB"
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Thomas Jensen and the boxes contributors")

include(CPack)
