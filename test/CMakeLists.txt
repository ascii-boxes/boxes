#============================================================================#
#                            test/CMakeLists.txt                             #
#                                                                            #
#                                                                            #
# Controls the testing framework used by CMake                               #
#============================================================================#

# creates a check command, which depends on the build executable
enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS ${BOXES_EXE})

# testing framework needs python
include(FindPythonInterp)

# Calls the testrunner script on the given test-case
macro(BOXES_TEST tpath)
    add_test(
        NAME ${tpath}
        COMMAND ${PYTHON_EXECUTABLE}
            "${CMAKE_CURRENT_SOURCE_DIR}/testrunner.py"
            "$<TARGET_FILE:${BOXES_EXE}>"
            "${BOXES_CFG}"
            "${tpath}"
        )
endmacro()

# Adds every .txt to the test suite
# NOTE: cmake only reconfigures itself if it sees a change to THIS FILE, so just
# adding a new test case will require you to manually reconfigure cmake
# The preferred way is to add a line for each test case, but I was lazy

file(GLOB cases "${CMAKE_CURRENT_SOURCE_DIR}/*_*.txt") # excludes thew CMakeLists.txt file
foreach(case ${cases})
    BOXES_TEST("${case}")
endforeach()

