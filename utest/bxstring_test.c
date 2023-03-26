/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2023 Thomas Jensen and the boxes contributors
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
 * Unit tests of the 'bxstring' module
 */

#include "config.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "bxstring.h"
#include "bxstring_test.h"
#include "global_mock.h"
#include "tools.h"
#include "unicode.h"
#include "utest_tools.h"



void test_bxsfree_null(void **state)
{
    (void) state;  /* unused */

    bxstr_t *bstr = (bxstr_t *) calloc(1, sizeof(bxstr_t));
    bstr->ascii = NULL;
    bstr->memory = NULL;
    bstr->first_char = NULL;
    bstr->visible_char = NULL;

    bxs_free(bstr);
    bxs_free(NULL);
}



void test_ascii_simple(void **state)
{
    (void) state;  /* unused */

    bxstr_t *actual = bxs_from_ascii("foo");

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("foo", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(3, (int) actual->num_columns);
    assert_int_equal(3, (int) actual->num_chars);
    assert_int_equal(3, (int) actual->num_chars_visible);
    assert_int_equal(0, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_indexes[] = {0, 1, 2};
    assert_array_equal(expected_indexes, actual->first_char, 3);
    assert_array_equal(expected_indexes, actual->visible_char, 3);
    assert_int_equal(0, actual->offset_start);
    assert_int_equal(0, actual->offset_end);

    bxs_free(actual);
}



void test_ascii_tabs(void **state)
{
    (void) state;  /* unused */

    bxstr_t *actual = bxs_from_ascii("illegal \t tab");

    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: internal error: from_ascii() called with tabs: \"illegal \t tab\"\n", collect_err[0]);
}



void test_ascii_null(void **state)
{
    (void) state;  /* unused */

    bxstr_t *actual = bxs_from_ascii(NULL);

    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: internal error: from_ascii() called with NULL\n", collect_err[0]);
}



void test_ansi_unicode_book(void **state)
{
    (void) state;  /* unused */

    uint32_t *ustr32 = u32_strconv_from_arg(
            "\x1b[38;5;203m \x1b[0m  \x1b[38;5;198m x\x1b[0mxx\x1b[38;5;208m\xc3\xa4\x1b[0m\x1b[38;5;203mb\x1b[0m ",
            "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *actual = bxs_from_unicode(ustr32);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("    xxxxb ", actual->ascii);  // 'ä' replaced by 'x'
    assert_int_equal(4, (int) actual->indent);
    assert_int_equal(10, (int) actual->num_columns);
    assert_int_equal(70, (int) actual->num_chars);
    assert_int_equal(10, (int) actual->num_chars_visible);
    assert_int_equal(60, (int) actual->num_chars_invisible);
    assert_int_equal(1, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 16, 17, 18, 30, 35, 36, 37, 53, 69, 70};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 11);
    int expected_vischar_idx[] = {11, 16, 17, 29, 30, 35, 36, 48, 64, 69, 70};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 11);
    assert_int_equal(0, actual->offset_start);
    assert_int_equal(0, actual->offset_end);

    BFREE(ustr32);
    bxs_free(actual);
}



void test_ansi_unicode_space_kinds(void **state)
{
    (void) state;  /* unused */

    uint32_t *ustr32 = u32_strconv_from_arg(
            "\xe2\x80\x83\xe2\x80\x82 X",  /* em-space, en-space, space, x */
            "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *actual = bxs_from_unicode(ustr32);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("   X", actual->ascii);
    assert_int_equal(3, (int) actual->indent);
    assert_int_equal(4, (int) actual->num_columns);
    assert_int_equal(4, (int) actual->num_chars);
    assert_int_equal(4, (int) actual->num_chars_visible);
    assert_int_equal(0, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 1, 2, 3};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 4);
    int expected_vischar_idx[] = {0, 1, 2, 3};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 4);
    assert_int_equal(0, actual->offset_start);
    assert_int_equal(0, actual->offset_end);

    BFREE(ustr32);
    bxs_free(actual);
}



void test_ansi_unicode_chinese(void **state)
{
    (void) state;  /* unused */

    uint32_t *ustr32 = u32_strconv_from_arg(
            "\xe5\x85\xac\xe7\x88\xb8\xe8\xa6\x81\xe9\x81\x93\xef\xbc\x81",  /* 公爸要道！ */
            "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *actual = bxs_from_unicode(ustr32);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("xxxxxxxxxx", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(10, (int) actual->num_columns);
    assert_int_equal(5, (int) actual->num_chars);
    assert_int_equal(5, (int) actual->num_chars_visible);
    assert_int_equal(0, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 1, 2, 3, 4};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 5);
    int expected_vischar_idx[] = {0, 1, 2, 3, 4};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 5);
    assert_int_equal(0, actual->offset_start);
    assert_int_equal(0, actual->offset_end);

    BFREE(ustr32);
    bxs_free(actual);
}



void test_ansi_unicode_empty(void **state)
{
    (void) state;  /* unused */

    uint32_t *ustr32 = u32_strconv_from_arg("", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *actual = bxs_from_unicode(ustr32);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(0, (int) actual->num_columns);
    assert_int_equal(0, (int) actual->num_chars);
    assert_int_equal(0, (int) actual->num_chars_visible);
    assert_int_equal(0, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 0);
    int expected_vischar_idx[] = {};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 0);
    assert_int_equal(0, actual->offset_start);
    assert_int_equal(0, actual->offset_end);

    BFREE(ustr32);
    bxs_free(actual);
}



void test_ansi_unicode_tabs(void **state)
{
    (void) state;  /* unused */

    uint32_t *ustr32 = u32_strconv_from_arg("illegal \t tab", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *actual = bxs_from_unicode(ustr32);
    BFREE(ustr32);

    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: internal error: tab encountered in from_unicode()\n", collect_err[0]);
}



void test_ansi_unicode_null(void **state)
{
    (void) state;  /* unused */

    bxstr_t *actual = bxs_from_unicode(NULL);

    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: internal error: from_unicode() called with NULL\n", collect_err[0]);
}


// TODO test case for incomplete/broken escape sequences


/* vim: set cindent sw=4: */
