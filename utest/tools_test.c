/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2021 Thomas Jensen and the boxes contributors
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
 * Unit tests of the 'tools' module
 */

#include "config.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "tools.h"
#include "tools_test.h"


void test_strisyes_true(void **state)
{
    (void) state;  /* unused */

    assert_int_equal(1, strisyes("On"));
    assert_int_equal(1, strisyes("on"));
    assert_int_equal(1, strisyes("yes"));
    assert_int_equal(1, strisyes("YEs"));
    assert_int_equal(1, strisyes("true"));
    assert_int_equal(1, strisyes("True"));
    assert_int_equal(1, strisyes("1"));
    assert_int_equal(1, strisyes("t"));
    assert_int_equal(1, strisyes("T"));
}


void test_strisyes_false(void **state)
{
    (void) state;  /* unused */

    assert_int_equal(0, strisyes(NULL));
    assert_int_equal(0, strisyes(""));
    assert_int_equal(0, strisyes(" "));
    assert_int_equal(0, strisyes("off"));
    assert_int_equal(0, strisyes("false"));
    assert_int_equal(0, strisyes("no"));
    assert_int_equal(0, strisyes("0"));
    assert_int_equal(0, strisyes("f"));
    assert_int_equal(0, strisyes("F"));
    assert_int_equal(0, strisyes("tru"));
    assert_int_equal(0, strisyes("yes sir"));
    assert_int_equal(0, strisyes("100"));
}


void test_strisno_true(void **state)
{
    (void) state;  /* unused */

    assert_int_equal(1, strisno("off"));
    assert_int_equal(1, strisno("Off"));
    assert_int_equal(1, strisno("no"));
    assert_int_equal(1, strisno("NO"));
    assert_int_equal(1, strisno("false"));
    assert_int_equal(1, strisno("False"));
    assert_int_equal(1, strisno("0"));
    assert_int_equal(1, strisno("f"));
    assert_int_equal(1, strisno("F"));
}


void test_strisno_false(void **state)
{
    (void) state;  /* unused */

    assert_int_equal(0, strisno(NULL));
    assert_int_equal(0, strisno(""));
    assert_int_equal(0, strisno(" "));
    assert_int_equal(0, strisno("on"));
    assert_int_equal(0, strisno("true"));
    assert_int_equal(0, strisno("yes"));
    assert_int_equal(0, strisno("1"));
    assert_int_equal(0, strisno("t"));
    assert_int_equal(0, strisno("T"));
    assert_int_equal(0, strisno("fal"));
    assert_int_equal(0, strisno("no sir"));
    assert_int_equal(0, strisno("00"));
}


/*EOF*/                                          /* vim: set cindent sw=4: */
