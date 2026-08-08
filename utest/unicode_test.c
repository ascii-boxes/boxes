/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * SPDX-FileCopyrightText: Copyright (c) 1999-2026 Thomas Jensen and the boxes contributors
 * SPDX-License-Identifier: GPL-3.0-only
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



static int emoji_width(const char *utf8)
{
    uint32_t *s = u32_strconv_from_arg(utf8, "UTF-8");
    assert_non_null(s);

    int width = 0;
    size_t remaining = 0;
    for (const uint32_t *p = s; *p != char_nul; p++) {
        width += u32_width_with_emoji(p, &remaining);
    }

    BFREE(s);
    return width;
}



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



void test_u32_width_with_emoji(void **state)
{
    UNUSED(state);

    struct {
        const char *utf8;
        int width;
    } cases[] = {
        {"\xe2\x9a\xa0\xef\xb8\x8f", 2},                     /* ⚠️ */
        {"\xe2\x9a\xa0", 1},                                 /* ⚠ */
        {"\xf0\x9f\x92\xa1", 2},                             /* 💡 */
        {"\xe2\x93\x82\xef\xb8\x8f", 2},                     /* Ⓜ️ */
        {"A\xef\xb8\x8f", 1},                                /* invalid A + VS16 */
        {"\xe2\x93\x9c\xef\xb8\x8f", 1},                     /* invalid ⓜ + VS16 */
        {"A\xcc\x81\xef\xb8\x8f", 1},                        /* A + combining acute + VS16 */
        {"1\xef\xb8\x8f\xe2\x83\xa3", 1},                    /* 1️⃣ */
        {"1\xe2\x83\xa3", 1},                                /* 1⃣ */
        {"#\xef\xb8\x8f\xe2\x83\xa3", 1},                    /* #️⃣ */
        {"*\xe2\x83\xa3", 1},                                /* *⃣ */
        {"0\xe2\x83\xa3", 1},                                /* 0⃣ */
        {"9\xef\xb8\x8f\xe2\x83\xa3", 1},                    /* 9️⃣ */
        {"1\xef\xb8\x8f", 1},                                /* 1 + VS16 */
        {"#\xef\xb8\x8f", 1},                                /* # + VS16 */
        {"*\xef\xb8\x8f", 1},                                /* * + VS16 */
        {"A\xe2\x83\xa3", 1},                                /* invalid keycap */
        {"\xf0\x9f\x91\x8b\xf0\x9f\x8f\xbb", 2},             /* 👋🏻 */
        {"\xf0\x9f\x91\x81\xef\xb8\x8f\xe2\x80\x8d"
         "\xf0\x9f\x97\xa8\xef\xb8\x8f", 2},                 /* 👁️‍🗨️ */
        {"\xe2\x9d\xa4\xef\xb8\x8f\xe2\x80\x8d"
         "\xf0\x9f\x94\xa5", 2},                             /* ❤️‍🔥 */
        {"\xf0\x9f\x8f\xb3\xef\xb8\x8f\xe2\x80\x8d"
         "\xf0\x9f\x8c\x88", 2},                             /* 🏳️‍🌈 */
        {"\xf0\x9f\x91\xa8\xe2\x80\x8d\xf0\x9f\x91\xa9"
         "\xe2\x80\x8d\xf0\x9f\x91\xa7", 2},                 /* 👨‍👩‍👧 */
        {"\xf0\x9f\x91\xa9\xf0\x9f\x8f\xbd\xe2\x80\x8d"
         "\xf0\x9f\x9a\x80", 2},                             /* 👩🏽‍🚀 */
        {"\xe2\x98\x85\xe2\x80\x8d\xf0\x9f\x92\xa1", 3},     /* invalid ★ + ZWJ + 💡 */
        {"\xf0\x9f\x92\xa1\xe2\x80\x8d\x41", 3},             /* invalid 💡 + ZWJ + A */
        {"\xef\xb8\x8f\xef\xb8\x8f", 0},                     /* standalone VS16 */
        {"\xe2\x9a\xa0\xef\xb8\x8e", 1},                     /* ⚠︎ */
        {"\xe2\x9a\xa0\xef\xb8\x8f"
         "1\xe2\x83\xa3\xf0\x9f\x91\x8b\xf0\x9f\x8f\xbb", 5} /* ⚠️1⃣👋🏻 */
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        assert_int_equal(cases[i].width, emoji_width(cases[i].utf8));
    }

    size_t remaining = 0;
    assert_int_equal(0, u32_width_with_emoji(NULL, &remaining));
    uint32_t empty[] = {0};
    assert_int_equal(0, u32_width_with_emoji(empty, &remaining));
    uint32_t nonempty[] = {'A', 0};
    assert_int_equal(0, u32_width_with_emoji(nonempty, NULL));
}


/* vim: set cindent sw=4: */
