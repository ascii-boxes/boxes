/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2021 Thomas Jensen and the boxes contributors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/*
 * Unit tests of the 'cmdline' module
 */

#include "config.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "boxes.h"
#include "cmdline.h"
#include "tools.h"
#include "global_mock.h"
#include "cmdline_test.h"


static opt_t *act(const int num_args, ...)
{
    assert_true(num_args >= 0);

    int argc = 1;
    char **argv = (char **) calloc(argc + 1, sizeof(char *));
    argv[0] = "out/boxes";

    if (num_args > 0) {
        va_list va;
        va_start(va, num_args);
        char *arg;
        while ((arg = va_arg(va, char *))) {
            ++argc;
            argv = (char **) realloc(argv, (argc + 1) * sizeof(char *));
            argv[argc - 1] = arg;
        }
        va_end(va);
    }
    argv[argc] = NULL;

    opt_t *actual = process_commandline(argc, argv);
    BFREE(argv);
    return actual;
}


void test_indentmode_none(void **state)
{
    (void) state;  /* unused */

    opt_t *actual = act(2, "-i", "none");

    assert_non_null(actual);
    assert_int_equal((int) 'n', (int) actual->indentmode);
}


void test_indentmode_invalid_long(void **state)
{
    (void) state;  /* unused */

    opt_t *actual = act(2, "-i", "INVALID");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid indentation mode\n", collect_err[0]);
}


void test_indentmode_invalid_short(void **state)
{
    (void) state;  /* unused */

    opt_t *actual = act(2, "-i", "X");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid indentation mode\n", collect_err[0]);
}


void test_indentmode_box(void **state)
{
    (void) state;  /* unused */

    opt_t *actual = act(2, "-i", "BO");

    assert_non_null(actual);
    assert_int_equal((int) 'b', (int) actual->indentmode);
}


void test_indentmode_text(void **state)
{
    (void) state;  /* unused */

    opt_t *actual = act(2, "-i", "t");

    assert_non_null(actual);
    assert_int_equal((int) 't', (int) actual->indentmode);
}


void test_killblank_true(void **state)
{
    (void) state;  /* unused */

    opt_t *actual = act(2, "-k", "true");

    assert_non_null(actual);
    assert_int_equal(1, (int) actual->killblank);
}


void test_killblank_false(void **state)
{
    (void) state;  /* unused */

    opt_t *actual = act(2, "-k", "false");

    assert_non_null(actual);
    assert_int_equal(0, (int) actual->killblank);
}


void test_killblank_invalid(void **state)
{
    (void) state;  /* unused */

    opt_t *actual = act(2, "-k", "INVALID");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: -k: invalid parameter\n", collect_err[0]);
}


void test_killblank_multiple(void **state)
{
    (void) state;  /* unused */

    opt_t *actual = act(4, "-k", "true", "-k", "false");    // first one wins

    assert_non_null(actual);
    assert_int_equal(1, (int) actual->killblank);
}


/*EOF*/                                          /* vim: set cindent sw=4: */
