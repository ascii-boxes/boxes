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
 * Unit tests of the 'bxstring' module
 */

#include "config.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistr.h>

#include <cmocka.h>
#include <string.h>

#include "boxes.h"
#include "bxstring.h"
#include "bxstring_test.h"
#include "global_mock.h"
#include "tools.h"
#include "unicode.h"
#include "utest_tools.h"



void test_ascii_simple(void **state)
{
    UNUSED(state);

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

    bxs_free(actual);
}



void test_ascii_illegalchar(void **state)
{
    UNUSED(state);

    bxstr_t *actual = bxs_from_ascii("illegal \b backspace");

    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: illegal character '\b' (0x00000008) encountered in string\n", collect_err[0]);
}



void test_ascii_null(void **state)
{
    UNUSED(state);

    bxstr_t *actual = bxs_from_ascii(NULL);

    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: internal error: from_ascii() called with NULL\n", collect_err[0]);
}



void test_ansi_unicode_book(void **state)
{
    UNUSED(state);

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

    BFREE(ustr32);
    bxs_free(actual);
}



void test_ansi_unicode_space_kinds(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("\xe2\x80\x83\xe2\x80\x82 X", /* em-space, en-space, space, x */
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

    BFREE(ustr32);
    bxs_free(actual);
}



void test_ansi_unicode_chinese(void **state)
{
    UNUSED(state);

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

    BFREE(ustr32);
    bxs_free(actual);
}



void test_ansi_unicode_empty(void **state)
{
    UNUSED(state);

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

    BFREE(ustr32);
    bxs_free(actual);
}



void test_ansi_unicode_blanks(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("    ", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *actual = bxs_from_unicode(ustr32);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("    ", actual->ascii);
    assert_int_equal(4, (int) actual->indent);
    assert_int_equal(4, (int) actual->num_columns);
    assert_int_equal(4, (int) actual->num_chars);
    assert_int_equal(4, (int) actual->num_chars_visible);
    assert_int_equal(0, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 1, 2, 3};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 0);
    int expected_vischar_idx[] = {0, 1, 2, 3};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 0);

    BFREE(ustr32);
    bxs_free(actual);
}



void test_ansi_unicode_invisible_only(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("\x1b[38;5;203m\x1b[0m\x1b[38;5;198m\x1b[m", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *actual = bxs_from_unicode(ustr32);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(0, (int) actual->num_columns);
    assert_int_equal(29, (int) actual->num_chars);
    assert_int_equal(0, (int) actual->num_chars_visible);
    assert_int_equal(29, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {29};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 1);
    int expected_vischar_idx[] = {29};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 1);

    BFREE(ustr32);
    bxs_free(actual);
}



void test_ansi_unicode_illegalchar(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("illegal \b backspace", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *actual = bxs_from_unicode(ustr32);
    BFREE(ustr32);

    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: illegal character '\b' (0x00000008) encountered in string\n", collect_err[0]);
}



void test_ansi_unicode_tabs(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg(" \t", "ASCII");
    assert_non_null(ustr32);
    bxstr_t *actual = bxs_from_unicode(ustr32);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("  ", actual->ascii);   /* tab converted to space in ascii representation */
    assert_int_equal(2, (int) actual->indent);
    assert_int_equal(2, (int) actual->num_columns);
    assert_int_equal(2, (int) actual->num_chars);
    assert_int_equal(2, (int) actual->num_chars_visible);
    assert_int_equal(0, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 1, 2};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 3);
    int expected_vischar_idx[] = {0, 1, 2};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 3);

    BFREE(ustr32);
    bxs_free(actual);
}



void test_ansi_unicode_broken_escapes(void **state)
{
    UNUSED(state);

    /* "\x1b[38;5;203 X" is the first escape sequence, followed by
     * "X " (visible) and
     * "\x1b[0m" (regular escape sequence) and
     * "__" (visible)
     * "\x1b[38;5;203m" (regular, invisible)
     * "b" (visible)
     * "\x1b[" (broken, counts as invisible, and belonging to the previous 'b')
     */
    uint32_t *ustr32 = u32_strconv_from_arg("\x1b[38;5;203 XX \x1b[0m__\x1b[38;5;203mb\x1b[", "ASCII");
    assert_non_null(ustr32);
    bxstr_t *actual = bxs_from_unicode(ustr32);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("X __b", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(5, (int) actual->num_columns);
    assert_int_equal(34, (int) actual->num_chars);
    assert_int_equal(5, (int) actual->num_chars_visible);
    assert_int_equal(29, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 13, 18, 19, 20, 34};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 6);
    int expected_vischar_idx[] = {12, 13, 18, 19, 31, 34};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 6);

    BFREE(ustr32);
    bxs_free(actual);
}



void test_ansi_unicode_null(void **state)
{
    UNUSED(state);

    bxstr_t *actual = bxs_from_unicode(NULL);

    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: internal error: bxs_from_unicode() called with NULL\n", collect_err[0]);
}



void test_ansi_unicode_tc183(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("\x1b[38;5;43m|\x1b[39m\x1b[38;5;49m \x1b[39m    X", "ASCII");
    assert_non_null(ustr32);
    bxstr_t *actual = bxs_from_unicode(ustr32);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("|     X", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(7, (int) actual->num_columns);
    assert_int_equal(37, (int) actual->num_chars);
    assert_int_equal(7, (int) actual->num_chars_visible);
    assert_int_equal(30, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 16, 32, 33, 34, 35, 36, 37};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 8);
    int expected_vischar_idx[] = {10, 26, 32, 33, 34, 35, 36, 37};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 8);

    BFREE(ustr32);
    bxs_free(actual);
}



void test_bxs_new_empty_string(void **state)
{
    UNUSED(state);

    bxstr_t *actual = bxs_new_empty_string();

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_int_equal(1, is_char_at(actual->memory, 0, char_nul));
    assert_string_equal("", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(0, (int) actual->num_columns);
    assert_int_equal(0, (int) actual->num_chars);
    assert_int_equal(0, (int) actual->num_chars_visible);
    assert_int_equal(0, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 1);
    int expected_vischar_idx[] = {0};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 1);

    bxs_free(actual);
}



void test_bxs_strdup(void **state)
{
    UNUSED(state);

    bxstr_t *actual = bxs_strdup(NULL);
    assert_null(actual);

    uint32_t *ustr32 = u32_strconv_from_arg(" x\x1b[38;5;203mc\x1b[0m\x1b[38;5;198mc\x1b[0mx  ", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *bxstr = bxs_from_unicode(ustr32);
    actual = bxs_strdup(bxstr);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal(" xccx  ", actual->ascii);
    assert_int_equal(1, (int) actual->indent);
    assert_int_equal(7, (int) actual->num_columns);
    assert_int_equal(37, (int) actual->num_chars);
    assert_int_equal(7, (int) actual->num_chars_visible);
    assert_int_equal(30, (int) actual->num_chars_invisible);
    assert_int_equal(2, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 1, 2, 18, 34, 35, 36, 37};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 8);
    int expected_vischar_idx[] = {0, 1, 13, 29, 34, 35, 36, 37};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 8);

    BFREE(ustr32);
    bxs_free(actual);
    bxs_free(bxstr);
}



void test_bxs_cut_front(void **state)
{
    UNUSED(state);

    bxstr_t *actual = bxs_cut_front(NULL, 1);
    assert_null(actual);

    uint32_t *ustr32 = u32_strconv_from_arg(" x\x1b[38;5;203mx\x1b[0m\x1b[38;5;198mf\x1b[0moo", "ASCII");
    assert_non_null(ustr32);
    bxstr_t *input = bxs_from_unicode(ustr32);
    actual = bxs_cut_front(input, 3);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("foo", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(3, (int) actual->num_columns);
    assert_int_equal(18, (int) actual->num_chars);
    assert_int_equal(3, (int) actual->num_chars_visible);
    assert_int_equal(15, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 16, 17, 18};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 4);
    int expected_vischar_idx[] = {11, 16, 17, 18};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 4);
    bxs_free(actual);

    actual = bxs_cut_front(input, 1000);
    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(0, (int) actual->num_columns);
    assert_int_equal(0, (int) actual->num_chars);
    assert_int_equal(0, (int) actual->num_chars_visible);
    assert_int_equal(0, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    bxs_free(actual);

    BFREE(ustr32);
    bxs_free(input);
}



void test_bxs_cut_front_zero(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg(" x\x1b[38;5;203mx\x1b[0m\x1b[38;5;198mf\x1b[0moo", "ASCII");
    assert_non_null(ustr32);
    bxstr_t *input = bxs_from_unicode(ustr32);

    bxstr_t *actual = actual = bxs_cut_front(input, 0);
    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal(" xxfoo", actual->ascii);
    assert_int_equal(1, (int) actual->indent);
    assert_int_equal(6, (int) actual->num_columns);
    assert_int_equal(36, (int) actual->num_chars);
    assert_int_equal(6, (int) actual->num_chars_visible);
    assert_int_equal(30, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 1, 2, 18, 34, 35, 36};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 7);
    int expected_vischar_idx[] = {0, 1, 13, 29, 34, 35, 36};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 7);

    bxs_free(actual);
    BFREE(ustr32);
    bxs_free(input);
}



void test_bxs_first_char_ptr_errors(void **state)
{
    UNUSED(state);

    uint32_t *actual = bxs_first_char_ptr(NULL, 0);
    assert_null(actual);

    uint32_t *ustr32 = u32_strconv_from_arg("ab\x1b[38;5;203mc\x1b[0m\x1b[38;5;198md\x1b[0me", "ASCII");
    assert_non_null(ustr32);
    bxstr_t *input = bxs_from_unicode(ustr32);
    actual = bxs_first_char_ptr(input, 1000);
    assert_non_null(actual);
    assert_true(u32_is_blank(actual));

    bxs_free(input);
    BFREE(ustr32);
}



void test_bxs_last_char_ptr(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("abc", "ASCII");
    assert_non_null(ustr32);
    bxstr_t *input = bxs_from_unicode(ustr32);

    assert_null(bxs_last_char_ptr(NULL));

    uint32_t *actual = bxs_last_char_ptr(input);
    assert_true(*actual == char_nul);
    assert_int_equal(3, actual - input->memory);

    BFREE(ustr32);
    bxs_free(input);
}



void test_bxs_unindent_ptr_null(void **state)
{
    UNUSED(state);

    uint32_t *actual = bxs_unindent_ptr(NULL);
    assert_null(actual);
}



void test_bxs_trimdup_null(void **state)
{
    UNUSED(state);

    bxstr_t *actual = bxs_trimdup(NULL, 0, 0);
    assert_null(actual);
    assert_int_equal(0, collect_err_size);
}



void test_bxs_trimdup_invalid_startidx(void **state)
{
    UNUSED(state);

    bxstr_t *input = bxs_from_ascii("foo");
    bxstr_t *actual = bxs_trimdup(input, 1000, 1000);

    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: internal error: start_idx out of bounds in bxs_trimdup()\n", collect_err[0]);

    bxs_free(input);
}



void test_bxs_trimdup_invalid_endidx(void **state)
{
    UNUSED(state);

    bxstr_t *input = bxs_from_ascii("foo");
    bxstr_t *actual = bxs_trimdup(input, 0, 1000);

    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: internal error: end_idx out of bounds in bxs_trimdup()\n", collect_err[0]);

    bxs_free(input);
}



void test_bxs_trimdup_invalid_endidx2(void **state)
{
    UNUSED(state);

    bxstr_t *input = bxs_from_ascii("foo");
    bxstr_t *actual = bxs_trimdup(input, 2, 1);

    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: internal error: end_idx before start_idx in bxs_trimdup()\n", collect_err[0]);

    bxs_free(input);
}



void test_bxs_trimdup_normal(void **state)
{
    UNUSED(state);

    bxstr_t *input = bxs_from_ascii("A, foo, B");
    bxstr_t *actual = bxs_trimdup(input, 2, 6);

    assert_non_null(actual);
    assert_string_equal("foo", actual->ascii);
    assert_int_equal(3, actual->num_chars);

    bxs_free(input);
    bxs_free(actual);
}



void test_bxs_trimdup_vanish(void **state)
{
    UNUSED(state);

    bxstr_t *input = bxs_from_ascii("A,    , B");
    bxstr_t *actual = bxs_trimdup(input, 2, 6);

    assert_non_null(actual);
    assert_string_equal("", actual->ascii);
    assert_int_equal(0, actual->num_chars);

    bxs_free(input);
    bxs_free(actual);
}



void test_bxs_trimdup_ansi(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("\x1b[38;5;203m \x1b[0m \x1b[38;5;203m \x1b[0m"
        "\x1b[38;5;203mX\x1b[0m\x1b[38;5;203mX\x1b[0m\x1b[38;5;198m \x1b[0m ", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *input = bxs_from_unicode(ustr32);
    assert_int_equal(7, input->num_chars_visible);
    assert_int_equal(3, input->indent);
    assert_int_equal(2, input->trailing);

    bxstr_t *actual = bxs_trimdup(input, 0, input->num_chars_visible);

    assert_non_null(actual);
    assert_string_equal("XX", actual->ascii);
    assert_int_equal(32, actual->num_chars);
    assert_int_equal(30, actual->num_chars_invisible);
    assert_int_equal(2, actual->num_chars_visible);
    assert_int_equal(2, actual->num_columns);
    assert_int_equal(0, actual->indent);

    BFREE(ustr32);
    bxs_free(input);
    bxs_free(actual);
}



void test_bxs_trimdup_ansi_same(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("X \x1b[38;5;203mX\x1b[0m\x1b[38;5;198m \x1b[0mX", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *input = bxs_from_unicode(ustr32);
    assert_int_equal(5, input->num_chars_visible);
    assert_int_equal(0, input->indent);
    assert_int_equal(0, input->trailing);

    bxstr_t *actual = bxs_trimdup(input, 0, input->num_chars_visible);

    assert_non_null(actual);
    assert_string_equal("X X X", actual->ascii);
    assert_int_equal(input->indent, actual->indent);
    assert_int_equal(input->num_columns, actual->num_columns);
    assert_int_equal(input->num_chars, actual->num_chars);
    assert_int_equal(input->num_chars_visible, actual->num_chars_visible);
    assert_int_equal(input->num_chars_invisible, actual->num_chars_invisible);
    assert_int_equal(input->trailing, actual->trailing);
    assert_memory_equal(input->memory, actual->memory, input->num_chars * sizeof(ucs4_t));
    assert_memory_equal(input->ascii, actual->ascii, input->num_columns * sizeof(char));
    assert_memory_equal(input->first_char, actual->first_char, input->num_chars_visible * sizeof(size_t));
    assert_memory_equal(input->visible_char, actual->visible_char, input->num_chars_visible * sizeof(size_t));

    BFREE(ustr32);
    bxs_free(input);
    bxs_free(actual);
}



void test_bxs_substr_errors(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("ab\x1b[38;5;203mc\x1b[0m\x1b[38;5;198md\x1b[0me", "ASCII");
    assert_non_null(ustr32);
    bxstr_t *input = bxs_from_unicode(ustr32);

    bxstr_t *actual = bxs_substr(NULL, 0, 0);
    assert_null(actual);

    actual = bxs_substr(input, 1000, input->num_chars);
    assert_non_null(actual);
    assert_true(bxs_is_empty(actual));
    bxs_free(actual);

    actual = bxs_substr(input, input->num_chars - 1, 1000);
    assert_non_null(actual);
    assert_string_equal("e", actual->ascii);
    assert_int_equal(1, actual->num_chars);
    bxs_free(actual);

    actual = bxs_substr(input, 3, 0);   /* start_idx > end_idx */
    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: internal error: end_idx before start_idx in bxs_substr()\n", collect_err[0]);

    BFREE(ustr32);
    bxs_free(input);
}



void test_bxs_strcat_empty(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("x", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *bxstr = bxs_from_unicode(ustr32);

    bxstr_t *actual = bxs_strcat(NULL, ustr32);
    assert_non_null(actual);
    assert_string_equal("x", actual->ascii);
    bxs_free(actual);

    actual = bxs_strcat(bxstr, NULL);
    assert_non_null(actual);
    assert_string_equal("x", actual->ascii);
    bxs_free(actual);

    BFREE(ustr32);
    bxs_free(bxstr);
}



void test_bxs_strcat_empty2(void **state)
{
    UNUSED(state);

    uint32_t *ustr32_x = u32_strconv_from_arg("x", "UTF-8");
    uint32_t *ustr32_empty = u32_strconv_from_arg("", "UTF-8");
    assert_non_null(ustr32_x);
    assert_non_null(ustr32_empty);
    bxstr_t *bxstr = bxs_from_unicode(ustr32_x);

    bxstr_t *actual = bxs_strcat(bxstr, ustr32_empty);
    assert_non_null(actual);
    assert_string_equal("x", actual->ascii);

    BFREE(ustr32_x);
    BFREE(ustr32_empty);
    bxs_free(actual);
    bxs_free(bxstr);
}



void test_bxs_strcat_empty3(void **state)
{
    UNUSED(state);

    uint32_t *ustr32_empty = u32_strconv_from_arg("", "UTF-8");
    uint32_t *ustr32_x = u32_strconv_from_arg("x", "UTF-8");
    assert_non_null(ustr32_empty);
    assert_non_null(ustr32_x);
    bxstr_t *bxstr = bxs_from_unicode(ustr32_empty);

    bxstr_t *actual = bxs_strcat(bxstr, ustr32_x);
    assert_non_null(actual);
    assert_string_equal("x", actual->ascii);

    BFREE(ustr32_empty);
    BFREE(ustr32_x);
    bxs_free(actual);
    bxs_free(bxstr);
}



void test_bxs_strcat(void **state)
{
    UNUSED(state);

    uint32_t *ustr32_1 = u32_strconv_from_arg("\x1b[38;5;203mA\x1b[0m\x1b[38;5;198mB\x1b[0m", "UTF-8");
    uint32_t *ustr32_2 = u32_strconv_from_arg("-\x1b[38;5;203ma\x1b[0m\x1b[38;5;198mb\x1b[0m", "UTF-8");
    assert_non_null(ustr32_1);
    assert_non_null(ustr32_2);
    bxstr_t *bxstr = bxs_from_unicode(ustr32_1);
    bxstr_t *actual = bxs_strcat(bxstr, ustr32_2);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("AB-ab", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(5, (int) actual->num_columns);
    assert_int_equal(65, (int) actual->num_chars);
    assert_int_equal(5, (int) actual->num_chars_visible);
    assert_int_equal(60, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 16, 32, 33, 49, 65};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 6);
    int expected_vischar_idx[] = {11, 27, 32, 44, 60, 65};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 6);

    BFREE(ustr32_1);
    BFREE(ustr32_2);
    bxs_free(actual);
    bxs_free(bxstr);
}



void test_bxs_strchr(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("\x1b[38;5;203ma\x1b[0m\x1b[38;5;198mb\x1b[0m", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *bxstr = bxs_from_unicode(ustr32);

    ucs4_t char_a = 0x00000061;
    ucs4_t char_m = 0x0000006D;

    uint32_t *found = bxs_strchr(bxstr, char_a, NULL);
    assert_int_equal(11, found - bxstr->memory);

    found = bxs_strchr(bxstr, char_m, NULL);
    assert_null(found);

    found = bxs_strchr(NULL, char_a, NULL);
    assert_null(found);

    BFREE(ustr32);
    bxs_free(bxstr);
}



void test_bxs_strchr_empty(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("\x1b[38;5;203m\x1b[0m", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *bxstr = bxs_from_unicode(ustr32);

    ucs4_t char_m = 0x0000006D;
    uint32_t *found = bxs_strchr(bxstr, char_m, NULL);
    assert_null(found);

    BFREE(ustr32);
    bxs_free(bxstr);
}



void test_bxs_strchr_cursor(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("foo, \x1b[38;5;203mBAR\x1b[0m, \x1b[38;5;198mBAZ\x1b[0m", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *bxstr = bxs_from_unicode(ustr32);
    ucs4_t char_comma = 0x0000002C;  /* ',' */

    int cursor = -1;
    uint32_t *found = bxs_strchr(bxstr, char_comma, &cursor);
    assert_non_null(found);
    assert_int_equal(3, cursor);
    assert_memory_equal(&char_comma, found, sizeof(ucs4_t));

    found = bxs_strchr(bxstr, char_comma, &cursor);
    assert_non_null(found);
    assert_int_equal(8, cursor);
    assert_memory_equal(&char_comma, found, sizeof(ucs4_t));

    found = bxs_strchr(bxstr, char_comma, &cursor);
    assert_null(found);
    assert_int_equal(8, cursor);

    BFREE(ustr32);
    bxs_free(bxstr);
}



void test_bxs_trim(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 =           /* em-space */
            u32_strconv_from_arg("\xe2\x80\x83 \x1b[38;5;203m \x1b[0mtrimmed\x1b[38;5;198m \x1b[0m ", "UTF-8");
    bxstr_t *bxstr = bxs_from_unicode(ustr32);
    assert_int_equal(3, (int) bxstr->indent);

    bxstr_t *actual = bxs_trim(bxstr);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("trimmed", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(7, (int) actual->num_columns);
    assert_int_equal(7, (int) actual->num_chars);
    assert_int_equal(7, (int) actual->num_chars_visible);
    assert_int_equal(0, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 1, 2, 3, 4, 5, 6, 7};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 8);
    int expected_vischar_idx[] = {0, 1, 2, 3, 4, 5, 6, 7};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 8);

    BFREE(ustr32);
    bxs_free(actual);
    bxs_free(bxstr);
}



void test_bxs_trim_blanks(void **state)
{
    UNUSED(state);
                                             /* em-space */
    uint32_t *ustr32 = u32_strconv_from_arg(" \xe2\x80\x83\x1b[38;5;203m \x1b[0m  \x1b[38;5;198m \x1b[0m ", "UTF-8");
    bxstr_t *bxstr = bxs_from_unicode(ustr32);
    assert_int_equal(7, (int) bxstr->indent);
    assert_int_equal(0, (int) bxstr->trailing);

    bxstr_t *actual = bxs_trim(bxstr);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(0, (int) actual->num_columns);
    assert_int_equal(0, (int) actual->num_chars);
    assert_int_equal(0, (int) actual->num_chars_visible);
    assert_int_equal(0, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 1);
    int expected_vischar_idx[] = {0};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 1);

    BFREE(ustr32);
    bxs_free(actual);
    bxs_free(bxstr);
}



void test_bxs_trim_none(void **state)
{
    UNUSED(state);

    bxstr_t *actual = bxs_trim(NULL);
    assert_null(actual);

    uint32_t *ustr32 = u32_strconv_from_arg("\x1b[38;5;203mX\x1b[0mX\x1b[38;5;198mX\x1b[0mX", "ASCII");
    bxstr_t *bxstr = bxs_from_unicode(ustr32);

    actual = bxs_trim(bxstr);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("XXXX", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(4, (int) actual->num_columns);
    assert_int_equal(34, (int) actual->num_chars);
    assert_int_equal(4, (int) actual->num_chars_visible);
    assert_int_equal(30, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 16, 17, 33, 34};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 5);
    int expected_vischar_idx[] = {11, 16, 28, 33, 34};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 5);

    BFREE(ustr32);
    bxs_free(actual);
    bxs_free(bxstr);
}



void test_bxs_ltrim2(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("\xe2\x80\x83 \x1b[38;5;203m\xe2\x80\x82\x1b[0m X", "UTF-8");
    /* em-space, space, en-space, space, X */
    bxstr_t *bxstr = bxs_from_unicode(ustr32);
    uint32_t *expected = u32_strconv_from_arg("\x1b[38;5;203m\xe2\x80\x82\x1b[0m X", "UTF-8");

    uint32_t *actual = bxs_ltrim(bxstr, 2);
    
    assert_non_null(actual);
    assert_int_equal(0, u32_strcmp(expected, actual));

    BFREE(ustr32);
    BFREE(actual);
    BFREE(expected);
    bxs_free(bxstr);
}



void test_bxs_ltrim5(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("\xe2\x80\x83 \x1b[38;5;203m\xe2\x80\x82\x1b[0m abc", "UTF-8");
    /* em-space, space, en-space, space, abc */
    bxstr_t *bxstr = bxs_from_unicode(ustr32);
    uint32_t *expected = u32_strconv_from_arg("abc", "ASCII");

    uint32_t *actual = bxs_ltrim(bxstr, 5);
    
    assert_non_null(actual);
    assert_int_equal(0, u32_strcmp(expected, actual));

    BFREE(ustr32);
    BFREE(actual);
    BFREE(expected);
    bxs_free(bxstr);
}



void test_bxs_ltrim_empty(void **state)
{
    UNUSED(state);

    uint32_t *actual = bxs_ltrim(NULL, 2);
    assert_null(actual);

    uint32_t *ustr32 = u32_strconv_from_arg("\xe2\x80\x83 \x1b[38;5;203m\xe2\x80\x82\x1b[0m X", "UTF-8");
    /* em-space, space, en-space, space, X */
    bxstr_t *bxstr = bxs_from_unicode(ustr32);

    actual = bxs_ltrim(bxstr, 0);
    assert_ptr_not_equal(ustr32, actual);
    assert_int_equal(0, u32_strcmp(ustr32, actual));

    BFREE(ustr32);
    BFREE(actual);
    bxs_free(bxstr);
}



void test_bxs_ltrim_max(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("\xe2\x80\x83 \x1b[38;5;203m\xe2\x80\x82\x1b[0m ", "UTF-8");
    /* em-space, space, en-space, space */
    bxstr_t *bxstr = bxs_from_unicode(ustr32);
    uint32_t *expected = new_empty_string32();

    uint32_t *actual = bxs_ltrim(bxstr, 20);
    
    assert_non_null(actual);
    assert_int_equal(0, u32_strlen(actual));
    assert_int_equal(0, u32_strcmp(expected, actual));

    BFREE(ustr32);
    BFREE(actual);
    BFREE(expected);
    bxs_free(bxstr);
}



void test_bxs_rtrim(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("xx\x1b[38;5;203m \x1b[0m \x1b[38;5;198m \x1b[0m ", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *bxstr = bxs_from_unicode(ustr32);
    bxstr_t *actual = bxs_rtrim(bxstr);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("xx", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(2, (int) actual->num_columns);
    assert_int_equal(2, (int) actual->num_chars);
    assert_int_equal(2, (int) actual->num_chars_visible);
    assert_int_equal(0, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 1, 2};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 3);
    int expected_vischar_idx[] = {0, 1, 2};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 3);

    BFREE(ustr32);
    bxs_free(actual);
    bxs_free(bxstr);
}



void test_bxs_rtrim_empty(void **state)
{
    UNUSED(state);

    bxstr_t *actual = bxs_rtrim(NULL);
    assert_null(actual);

    uint32_t *ustr32 = u32_strconv_from_arg("X\x1b[38;5;203m \x1b[0mX", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *bxstr = bxs_from_unicode(ustr32);
    actual = bxs_rtrim(bxstr);

    assert_non_null(actual);
    assert_non_null(actual->memory);
    assert_string_equal("X X", actual->ascii);
    assert_int_equal(0, (int) actual->indent);
    assert_int_equal(3, (int) actual->num_columns);
    assert_int_equal(18, (int) actual->num_chars);
    assert_int_equal(3, (int) actual->num_chars_visible);
    assert_int_equal(15, (int) actual->num_chars_invisible);
    assert_int_equal(0, (int) actual->trailing);
    int expected_firstchar_idx[] = {0, 1, 17, 18};
    assert_array_equal(expected_firstchar_idx, actual->first_char, 4);
    int expected_vischar_idx[] = {0, 12, 17, 18};
    assert_array_equal(expected_vischar_idx, actual->visible_char, 4);

    BFREE(ustr32);
    bxs_free(actual);
    bxs_free(bxstr);
}



void test_bxs_prepend_spaces_null(void **state)
{
    UNUSED(state);

    bxstr_t *actual = bxs_prepend_spaces(NULL, 2);
    assert_non_null(actual);
    assert_string_equal("  ", actual->ascii);
    assert_int_equal(2, (int) actual->num_chars);
    assert_int_equal(2, (int) actual->num_chars_visible);
    assert_int_equal(0, (int) actual->num_chars_invisible);

    bxs_free(actual);
}



void test_bxs_append_spaces(void **state)
{
    UNUSED(state);

    bxs_append_spaces(NULL, 2);

    uint32_t *ustr32 = u32_strconv_from_arg("X\x1b[38;5;203mY\x1b[0mZ", "UTF-8");
    assert_non_null(ustr32);
    bxstr_t *bxstr = bxs_from_unicode(ustr32);
    bxs_append_spaces(bxstr, 0);

    bxs_append_spaces(bxstr, 3);
    assert_non_null(bxstr->memory);
    assert_string_equal("XYZ   ", bxstr->ascii);
    assert_int_equal(0, (int) bxstr->indent);
    assert_int_equal(6, (int) bxstr->num_columns);
    assert_int_equal(21, (int) bxstr->num_chars);
    assert_int_equal(6, (int) bxstr->num_chars_visible);
    assert_int_equal(15, (int) bxstr->num_chars_invisible);
    assert_int_equal(3, (int) bxstr->trailing);
    int expected_firstchar_idx[] = {0, 1, 17, 18, 19, 20, 21};
    assert_array_equal(expected_firstchar_idx, bxstr->first_char, 7);
    int expected_vischar_idx[] = {0, 12, 17, 18, 19, 20, 21};
    assert_array_equal(expected_vischar_idx, bxstr->visible_char, 7);

    BFREE(ustr32);
    bxs_free(bxstr);
}



void test_bxs_to_output(void **state)
{
    UNUSED(state);

    char *actual = bxs_to_output(NULL);
    assert_string_equal("NULL", actual);

    BFREE(actual);

    bxstr_t *bxstr = bxs_from_ascii("foobar");
    actual = bxs_to_output(bxstr);
    assert_string_equal("foobar", actual);

    BFREE(actual);
    bxs_free(bxstr);
}



void test_bxs_is_empty_null(void **state)
{
    UNUSED(state);

    int actual = bxs_is_empty(NULL);
    assert_int_equal(1, actual);
}



void test_bxs_is_blank(void **state)
{
    UNUSED(state);

    assert_int_equal(1, bxs_is_blank(NULL));

    bxstr_t *bxstr = bxs_new_empty_string();
    assert_int_equal(1, bxs_is_blank(bxstr));
    bxs_free(bxstr);

    uint32_t *ustr32 = u32_strconv_from_arg(" \x1b[38;5;203m \x1b[0m \x1b[38;5;203m\x1b[0m", "ASCII");
    assert_non_null(ustr32);
    bxstr = bxs_from_unicode(ustr32);
    assert_int_equal(1, bxs_is_blank(bxstr));
    BFREE(ustr32);
    bxs_free(bxstr);

    ustr32 = u32_strconv_from_arg("\x1b[38;5;203m\x1b[0m", "ASCII");
    assert_non_null(ustr32);
    bxstr = bxs_from_unicode(ustr32);
    assert_int_equal(1, bxs_is_blank(bxstr));
    BFREE(ustr32);
    bxs_free(bxstr);

    ustr32 = u32_strconv_from_arg("x", "ASCII");
    assert_non_null(ustr32);
    bxstr = bxs_from_unicode(ustr32);
    assert_int_equal(0, bxs_is_blank(bxstr));
    BFREE(ustr32);
    bxs_free(bxstr);
}



void test_bxs_is_visible_char(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("\x1b[38;5;203mX\x1b[0m \x1b[38;5;203mY\x1b[0m", "ASCII");
    assert_non_null(ustr32);
    bxstr_t *input = bxs_from_unicode(ustr32);

    assert_int_equal(0, bxs_is_visible_char(NULL, 4));
    assert_int_equal(0, bxs_is_visible_char(input, 1000));
    assert_int_equal(0, bxs_is_visible_char(input, 0));
    assert_int_equal(0, bxs_is_visible_char(input, 1));
    assert_int_equal(0, bxs_is_visible_char(input, 31));
    assert_int_equal(0, bxs_is_visible_char(input, 32));
    assert_int_equal(0, bxs_is_visible_char(input, 33));

    assert_int_equal(1, bxs_is_visible_char(input, 11));
    assert_int_equal(1, bxs_is_visible_char(input, 16));
    assert_int_equal(1, bxs_is_visible_char(input, 28));

    BFREE(ustr32);
    bxs_free(input);
}



void test_bxs_filter_visible(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("\x1b[38;5;203mX\x1b[0m \x1b[38;5;203mY\x1b[0m", "ASCII");
    assert_non_null(ustr32);
    bxstr_t *input = bxs_from_unicode(ustr32);
    uint32_t *expected = u32_strconv_from_arg("X Y", "ASCII");

    uint32_t *actual = bxs_filter_visible(input);
    assert_int_equal(0, u32_strcmp(expected, actual));

    actual = bxs_filter_visible(NULL);
    assert_null(actual);

    BFREE(actual);
    BFREE(expected);
    BFREE(ustr32);
    bxs_free(input);
}



void test_bxs_filter_visible_none(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("plain", "ASCII");
    assert_non_null(ustr32);
    bxstr_t *input = bxs_from_unicode(ustr32);
    uint32_t *expected = u32_strconv_from_arg("plain", "ASCII");

    uint32_t *actual = bxs_filter_visible(input);
    assert_int_equal(0, u32_strcmp(expected, actual));

    BFREE(actual);
    BFREE(expected);
    BFREE(ustr32);
    bxs_free(input);
}



void test_bxs_strcmp(void **state)
{
    UNUSED(state);

    int actual = bxs_strcmp(NULL, NULL);
    assert_int_equal(0, actual);

    bxstr_t *bxstr1 = bxs_from_ascii("alice");
    bxstr_t *bxstr2 = bxs_from_ascii("bob");

    actual = bxs_strcmp(NULL, bxstr1);
    assert_int_equal(1, actual);

    actual = bxs_strcmp(bxstr1, NULL);
    assert_int_equal(-1, actual);

    actual = bxs_strcmp(bxstr1, bxstr2);
    assert_int_equal(-1, actual);

    actual = bxs_strcmp(bxstr2, bxstr1);
    assert_int_equal(1, actual);

    bxs_free(bxstr1);
    bxs_free(bxstr2);
}



void test_bxs_valid_anywhere_error(void **state)
{
    UNUSED(state);

    size_t error_pos = 42;

    assert_int_equal(0, bxs_valid_anywhere(NULL, NULL));
    assert_int_equal(0, bxs_valid_anywhere(NULL, &error_pos));
    assert_int_equal(0, (int) error_pos);

    bxstr_t *bxstr = bxs_from_ascii("illegal X backspace");
    ucs4_t char_backspace = 0x00000008;
    bxstr->memory[8] = char_backspace;
    assert_int_equal(0, bxs_valid_anywhere(bxstr, NULL));

    bxs_free(bxstr);
}



void test_bxs_valid_in_filename_error(void **state)
{
    UNUSED(state);

    uint32_t *ustr32 = u32_strconv_from_arg("\x1b[38;5;203m\x1b[0m", "ASCII");
    bxstr_t *bxstr_no_vis = bxs_from_unicode(ustr32);
    BFREE(ustr32);
    ustr32 = u32_strconv_from_arg("\x1b[38;5;203m_VISIBLE_\x1b[0m", "ASCII");
    bxstr_t *bxstr_vis_invis = bxs_from_unicode(ustr32);

    assert_int_equal(0, bxs_valid_in_filename(bxstr_no_vis, NULL));
    assert_int_equal(0, bxs_valid_in_filename(bxstr_vis_invis, NULL));

    BFREE(ustr32);
    bxs_free(bxstr_no_vis);
    bxs_free(bxstr_vis_invis);
}



void test_bxs_free_null(void **state)
{
    UNUSED(state);

    bxstr_t *bstr = (bxstr_t *) calloc(1, sizeof(bxstr_t));
    bstr->ascii = NULL;
    bstr->memory = NULL;
    bstr->first_char = NULL;
    bstr->visible_char = NULL;

    bxs_free(bstr);
    bxs_free(NULL);
}



void test_bxs_concat(void **state)
{
    UNUSED(state);

    bxstr_t *actual = bxs_concat(0);
    assert_non_null(actual);
    assert_string_equal("", actual->ascii);
    assert_int_equal(0, actual->num_chars);

    uint32_t *s1 = u32_strconv_from_arg("\x1b[38;5;203mX\x1b[0m", "ASCII");
    uint32_t *s2 = u32_strconv_from_arg("---", "ASCII");
    uint32_t *s3 = u32_strconv_from_arg("ÄÖÜ\x1b[38;5;203m!\x1b[0m", "UTF-8");
    uint32_t *s4 = u32_strconv_from_arg("  ", "ASCII");
    actual = bxs_concat(5, s1, s2, s3, s4, s4);

    assert_non_null(actual);
    assert_string_equal("X---xxx!    ", actual->ascii);
    assert_int_equal(12, actual->num_chars_visible);
    assert_int_equal(30, actual->num_chars_invisible);
    assert_int_equal(42, actual->num_chars);
    assert_int_equal(4, actual->trailing);
    assert_int_equal(0, actual->indent);
    assert_int_equal(12, actual->num_columns);

    BFREE(s1);
    BFREE(s2);
    BFREE(s3);
    BFREE(s4);
    bxs_free(actual);
}



void test_bxs_concat_nullarg(void **state)
{
    UNUSED(state);

    uint32_t *s1 = u32_strconv_from_arg("\x1b[38;5;203mAB", "ASCII");
    uint32_t *s2 = u32_strconv_from_arg("-CD-", "ASCII");
    uint32_t *s3 = u32_strconv_from_arg("-EF\x1b[0m", "ASCII");
    bxstr_t *actual = bxs_concat(4, s1, s2, NULL, s3);

    assert_non_null(actual);
    assert_string_equal("AB-CD-(NULL)-EF", actual->ascii);
    assert_int_equal(15, actual->num_chars_visible);
    assert_int_equal(15, actual->num_chars_invisible);
    assert_int_equal(30, actual->num_chars);
    assert_int_equal(0, actual->trailing);
    assert_int_equal(0, actual->indent);
    assert_int_equal(15, actual->num_columns);

    BFREE(s1);
    BFREE(s2);
    BFREE(s3);
    bxs_free(actual);
}


/* vim: set cindent sw=4: */
