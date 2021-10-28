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
 * Unit tests of the 'regulex' module
 */

#include "config.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "global_mock.h"
#include "regulex.h"


void test_compile_pattern_empty(void **state)
{
    (void) state;  /* unused */

    assert_null(compile_pattern(NULL));
    assert_non_null(compile_pattern(""));
}


void test_compile_pattern_error(void **state)
{
    (void) state;  /* unused */

    assert_null(compile_pattern("incomplete[x"));
    assert_int_equal(1, collect_err_size);
    assert_string_equal("Regular expression pattern \"incomplete[x\" failed to compile at position 12: "
        "missing terminating ] for character class\n", collect_err[0]);

    collect_reset();
    assert_null(compile_pattern("incomplete\\"));
    assert_int_equal(1, collect_err_size);
    assert_string_equal("Regular expression pattern \"incomplete\\\" failed to compile at position 11: "
        "\\ at end of pattern\n", collect_err[0]);
}


/*EOF*/                                          /* vim: set cindent sw=4: */
