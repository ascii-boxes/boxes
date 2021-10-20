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
 * Unit tests of the 'tools' module
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "tools.h"


static void test_strisyes_true()
{
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


static void test_strisyes_false()
{
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


int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_strisyes_true),
        cmocka_unit_test(test_strisyes_false)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
