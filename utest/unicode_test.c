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
 * Unit tests of the 'unicode' module
 */

#include "config.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <stdio.h>
#include <string.h>
#include <unistr.h>

#include "boxes.h"
#include "tools.h"
#include "unicode.h"
#include "unicode_test.h"



void test_to_utf32(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("A", "ASCII");
    assert_non_null(ustr32);

    ucs4_t actual_A = to_utf32('A');
    ucs4_t actual_space = to_utf32(' ');
    ucs4_t actual_invalid = to_utf32('\x1B');

    assert_int_equal(0, memcmp(ustr32, &actual_A, sizeof(ucs4_t)));
    assert_int_equal(0, memcmp(&char_space, &actual_space, sizeof(ucs4_t)));
    assert_int_equal(0, memcmp(&char_nul, &actual_invalid, sizeof(ucs4_t)));
}



void test_is_blank(void **state)
{
    UNUSED(state);

    const ucs4_t char_emspace = 0x00002003;
    const ucs4_t char_enspace = 0x00002002;

    assert_int_equal(1, is_blank(char_space));
    assert_int_equal(1, is_blank(char_emspace));
    assert_int_equal(1, is_blank(char_enspace));
    assert_int_equal(1, is_blank(char_tab));

    assert_int_equal(0, is_blank(to_utf32('x')));
    assert_int_equal(0, is_blank(char_cr));
    assert_int_equal(0, is_blank(char_newline));
    assert_int_equal(0, is_blank(char_esc));
}



void test_is_allowed_in_sample(void **state)
{
    UNUSED(state);

    const ucs4_t char_bell = 0x00000007;
    const ucs4_t char_backsp = 0x00000008;
    const ucs4_t char_u_umlaut = 0x000000fc;

    assert_int_equal(1, is_allowed_in_sample(to_utf32('x')));
    assert_int_equal(1, is_allowed_in_sample(char_u_umlaut));
    assert_int_equal(1, is_allowed_in_sample(char_space));
    assert_int_equal(1, is_allowed_in_sample(char_esc));
    assert_int_equal(1, is_allowed_in_sample(char_cr));
    assert_int_equal(1, is_allowed_in_sample(char_newline));
    assert_int_equal(1, is_allowed_in_sample(char_tab));

    assert_int_equal(0, is_allowed_in_sample(char_bell));
    assert_int_equal(0, is_allowed_in_sample(char_backsp));
}



void test_is_allowed_in_shape(void **state)
{
    UNUSED(state);

    const ucs4_t char_bell = 0x00000007;
    const ucs4_t char_backsp = 0x00000008;
    const ucs4_t char_u_umlaut = 0x000000fc;

    assert_int_equal(1, is_allowed_in_shape(to_utf32('x')));
    assert_int_equal(1, is_allowed_in_shape(char_u_umlaut));
    assert_int_equal(1, is_allowed_in_shape(char_space));
    assert_int_equal(1, is_allowed_in_shape(char_esc));
    assert_int_equal(1, is_allowed_in_shape(char_tab));  /* But tabs are deprecated in shapes! Temporary only. */

    assert_int_equal(0, is_allowed_in_shape(char_bell));
    assert_int_equal(0, is_allowed_in_shape(char_backsp));
    assert_int_equal(0, is_allowed_in_shape(char_cr));
    assert_int_equal(0, is_allowed_in_shape(char_newline));
}



void test_is_allowed_in_filename(void **state)
{
    UNUSED(state);

    const ucs4_t char_bell = 0x00000007;
    const ucs4_t char_backsp = 0x00000008;
    const ucs4_t char_u_umlaut = 0x000000fc;

    assert_int_equal(1, is_allowed_in_filename(to_utf32('x')));
    assert_int_equal(1, is_allowed_in_filename(char_u_umlaut));
    assert_int_equal(1, is_allowed_in_filename(char_space));
    assert_int_equal(1, is_allowed_in_filename(char_tab));

    assert_int_equal(0, is_allowed_in_filename(char_esc));
    assert_int_equal(0, is_allowed_in_filename(char_bell));
    assert_int_equal(0, is_allowed_in_filename(char_backsp));
    assert_int_equal(0, is_allowed_in_filename(char_cr));
    assert_int_equal(0, is_allowed_in_filename(char_newline));
}



void test_is_allowed_in_kv_string(void **state)
{
    UNUSED(state);

    const ucs4_t char_bell = 0x00000007;
    const ucs4_t char_backsp = 0x00000008;
    const ucs4_t char_u_umlaut = 0x000000fc;

    assert_int_equal(1, is_allowed_in_kv_string(to_utf32('x')));
    assert_int_equal(1, is_allowed_in_kv_string(char_u_umlaut));
    assert_int_equal(1, is_allowed_in_kv_string(char_space));
    assert_int_equal(1, is_allowed_in_kv_string(char_tab));

    assert_int_equal(0, is_allowed_in_kv_string(char_esc));
    assert_int_equal(0, is_allowed_in_kv_string(char_bell));
    assert_int_equal(0, is_allowed_in_kv_string(char_backsp));
    assert_int_equal(0, is_allowed_in_kv_string(char_cr));
    assert_int_equal(0, is_allowed_in_kv_string(char_newline));
}



void test_u32_strnrstr(void **state)
{
    UNUSED(state);

    uint32_t *haystack = u32_strconv_from_arg("a foo found found bar fou", "ASCII");
    assert_non_null(haystack);
    uint32_t *needle = u32_strconv_from_arg("found", "ASCII");
    assert_non_null(needle);

    assert_null(u32_strnrstr(NULL, needle, u32_strlen(needle)));
    assert_ptr_equal(haystack, u32_strnrstr(haystack, NULL, 0));

    uint32_t *actual = u32_strnrstr(haystack, needle, u32_strlen(needle));
    assert_ptr_equal(haystack + 12, actual);

    BFREE(haystack);
    BFREE(needle);
}



void test_u32_insert_space_at(void **state)
{
    UNUSED(state);

    uint32_t *expected = u32_strconv_from_arg("x xxx  ", "ASCII");
    assert_non_null(expected);
    uint32_t *s = u32_strconv_from_arg("xxxx", "ASCII");
    assert_non_null(s);

    u32_insert_space_at(NULL, 2, 3);
    u32_insert_space_at(&s, 3, 0);
    u32_insert_space_at(&s, 1, 1);
    u32_insert_space_at(&s, 10000, 2);

    assert_non_null(s);
    assert_int_equal(0, u32_strcmp(expected, s));

    BFREE(s);
    BFREE(expected);
}


/* vim: set cindent sw=4: */