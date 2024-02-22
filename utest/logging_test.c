/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2024 Thomas Jensen and the boxes contributors
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License, version 3, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/*
 * Unit tests of the 'logging' module
 */

#include "config.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "boxes.h"
#include "tools.h"
#include "logging.h"
#include "logging_test.h"
#include "global_mock.h"
#include "utest_tools.h"


int logging_setup(void **state)
{
    UNUSED(state);

    int *areas = (int *) calloc(NUM_LOG_AREAS, sizeof(int));
    areas[MAIN - 2] = 1;
    activate_debug_logging(areas);
    BFREE(areas);
    return 0;
}



int logging_teardown(void **state)
{
    UNUSED(state);

    activate_debug_logging(NULL);  /* turn off debug logging */
    return 0;
}



void test_debug_shorten_nopath(void **state)
{
    UNUSED(state);

    log_debug("nopath.c", MAIN, "foo\n");

    assert_int_equal(1, collect_err_size);
    assert_string_equal("[nopath   ] foo\n", collect_err[0]);
}



void test_debug_shorten_minimal(void **state)
{
    UNUSED(state);

    log_debug("minimal", MAIN, "foo\n");

    assert_int_equal(1, collect_err_size);
    assert_string_equal("[minimal  ] foo\n", collect_err[0]);
}



void test_debug_shorten_null(void **state)
{
    UNUSED(state);

    log_debug(NULL, MAIN, "foo\n");

    assert_int_equal(1, collect_err_size);
    assert_string_equal("[NULL     ] foo\n", collect_err[0]);
}



void test_debug_shorten_backslash(void **state)
{
    UNUSED(state);

    log_debug("..\\path\\to\\module.c", MAIN, "foo\n");

    assert_int_equal(1, collect_err_size);
    assert_string_equal("[module   ] foo\n", collect_err[0]);
}



void test_debug_shorten_slash(void **state)
{
    UNUSED(state);

    log_debug("path/to/module.c", MAIN, "foo\n");

    assert_int_equal(1, collect_err_size);
    assert_string_equal("[module   ] foo\n", collect_err[0]);
}



void test_debug_shorten_nosuffix(void **state)
{
    UNUSED(state);

    log_debug("path/to/module", MAIN, "foo\n");

    assert_int_equal(1, collect_err_size);
    assert_string_equal("[module   ] foo\n", collect_err[0]);
}



void test_debug_shorten_wrongdot(void **state)
{
    UNUSED(state);

    log_debug("path/to.dot/module", MAIN, "foo\n");

    assert_int_equal(1, collect_err_size);
    assert_string_equal("[module   ] foo\n", collect_err[0]);
}



void test_debug_shorten_empty(void **state)
{
    UNUSED(state);

    log_debug("", MAIN, "foo\n");

    assert_int_equal(1, collect_err_size);
    assert_string_equal("[         ] foo\n", collect_err[0]);
}



void test_debug_inactive_area(void **state)
{
    UNUSED(state);

    log_debug("module.c", LEXER, "foo\n");

    assert_int_equal(0, collect_err_size);
}



void test_debug_continue(void **state)
{
    UNUSED(state);

    log_debug_cont(MAIN, "foo");

    assert_int_equal(1, collect_err_size);
    assert_string_equal("foo", collect_err[0]);
}



void test_debug_continue_inactive(void **state)
{
    UNUSED(state);

    log_debug_cont(LEXER, "foo");

    assert_int_equal(0, collect_err_size);
}



void test_debug_area_too_big(void **state)
{
    UNUSED(state);

    log_debug("module.c", 100, "foo\n");  /* 100 is way too large and does not exist */

    assert_int_equal(0, collect_err_size);
}



void test_debug_area_reserved(void **state)
{
    UNUSED(state);

    log_debug("module.c", RESERVED, "foo\n");  /* RESERVED may never be used and is always "inactive" */

    assert_int_equal(0, collect_err_size);
}



void test_debug_area_all(void **state)
{
    UNUSED(state);

    log_debug("module.c", ALL, "foo\n");  /* only MAIN is active, but not all of them */

    assert_int_equal(0, collect_err_size);
}



void test_debug_deactivated(void **state)
{
    UNUSED(state);

    activate_debug_logging(NULL);
    log_debug("module.c", MAIN, "foo\n");

    assert_int_equal(0, collect_err_size);
}



void test_debug_all_active(void **state)
{
    UNUSED(state);

    int *areas = (int *) calloc(NUM_LOG_AREAS, sizeof(int));
    for(size_t i = 0; i < NUM_LOG_AREAS; i++) {
        areas[i] = 1;
    }
    activate_debug_logging(areas);
    BFREE(areas);

    log_debug("module.c", ALL, "foo\n");

    assert_int_equal(1, collect_err_size);
    assert_string_equal("[module   ] foo\n", collect_err[0]);
}


/* vim: set cindent sw=4: */
