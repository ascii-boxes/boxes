
# Test project name.
project(boxes_utest LANGUAGES C)

include(CheckCCompilerFlag)
check_c_compiler_flag(-Wno-stringop-overflow HAVE_STRINGOP_OVERFLOW)

# =============================================================================

# Find required packages. If used from Conan, make sure to run "conan install" before.
find_package(cmocka REQUIRED)

# =============================================================================

set(SUT boxes_lib)
set(TARGET ${PROJECT_NAME})

set(SOURCES
    bxstring_test.c
    cmdline_test.c
    regulex_test.c
    tools_test.c
    unicode_test.c
    global_mock.c
    utest_tools.c
    main.c
)

if (HAVE_STRINGOP_OVERFLOW)
    target_compile_options(${TARGET}
        PRIVATE -Wno-stringop-overflow
    )
endif (HAVE_STRINGOP_OVERFLOW)

# -----------------------------------------------------------------------------

add_executable(${TARGET}
    ${SOURCES}
)
target_link_libraries(${TARGET}
    PRIVATE
        ${SUT}
        cmocka::cmocka
)
target_link_options(${TARGET}
    PRIVATE "-Wl,--wrap=bx_fprintf"
)

# =============================================================================

add_test(
    NAME ${TARGET}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
