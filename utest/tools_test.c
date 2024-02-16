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
 * Unit tests of the 'tools' module
 */

#include "config.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "tools.h"
#include "tools_test.h"
#include "unicode.h"


void test_strisyes_true(void **state)
{
    (void) state; /* unused */

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
    (void) state; /* unused */

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
    (void) state; /* unused */

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
    (void) state; /* unused */

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


void test_my_strrspn_edge(void **state)
{
    (void) state; /* unused */

    assert_int_equal(0, (int) my_strrspn(NULL, "abc"));
    assert_int_equal(0, (int) my_strrspn("", "abc"));
    assert_int_equal(0, (int) my_strrspn("abc", NULL));
    assert_int_equal(0, (int) my_strrspn("abc", ""));
    assert_int_equal(0, (int) my_strrspn(NULL, NULL));
}


void test_my_strrspn(void **state)
{
    (void) state; /* unused */

    assert_int_equal(2, (int) my_strrspn("foo", "o"));
    assert_int_equal(0, (int) my_strrspn("foo", "ABC"));
    assert_int_equal(3, (int) my_strrspn("foo", "foobar"));
    assert_int_equal(1, (int) my_strrspn("foo ", " "));
    assert_int_equal(1, (int) my_strrspn("a", "a"));
    assert_int_equal(0, (int) my_strrspn("a", "A"));
    assert_int_equal(2, (int) my_strrspn("axxaa", "a"));
}


void test_is_csi_reset(void **state)
{
    (void) state; /* unused */

    assert_int_equal(1, is_csi_reset(u32_strconv_from_arg("\x1b[0m", "ASCII")));
    assert_int_equal(1, is_csi_reset(u32_strconv_from_arg("\x1b[m", "ASCII")));
    assert_int_equal(1, is_csi_reset(u32_strconv_from_arg("\x1b(0m", "ASCII")));
    assert_int_equal(1, is_csi_reset(u32_strconv_from_arg("\x1b(m", "ASCII")));
    assert_int_equal(1, is_csi_reset(u32_strconv_from_arg("\x1b[0m foo", "ASCII")));

    assert_int_equal(0, is_csi_reset(u32_strconv_from_arg("", "ASCII")));
    assert_int_equal(0, is_csi_reset(u32_strconv_from_arg("normal", "ASCII")));
    assert_int_equal(0, is_csi_reset(u32_strconv_from_arg("\x1b[", "ASCII")));
    assert_int_equal(0, is_csi_reset(u32_strconv_from_arg("not yet \x1b[0m", "ASCII")));
    assert_int_equal(0, is_csi_reset(u32_strconv_from_arg("\x1b[38;5;203m", "ASCII")));
    assert_int_equal(0, is_csi_reset(u32_strconv_from_arg("\x1b_BROKEN", "ASCII")));
}


void test_is_ascii_id_valid(void **state)
{
    (void) state; /* unused */

    bxstr_t *s = bxs_from_ascii("valid");
    assert_int_equal(1, is_ascii_id(s, 0));
    bxs_free(s);

    s = bxs_from_ascii("also-valid");
    assert_int_equal(1, is_ascii_id(s, 0));
    bxs_free(s);

    s = bxs_from_ascii("fine2");
    assert_int_equal(1, is_ascii_id(s, 0));
    bxs_free(s);

    s = bxs_from_ascii("A");
    assert_int_equal(1, is_ascii_id(s, 0));
    bxs_free(s);

    s = bxs_from_ascii("A_2");
    assert_int_equal(1, is_ascii_id(s, 0));
    bxs_free(s);
}


void test_is_ascii_id_invalid(void **state)
{
    (void) state; /* unused */

    assert_int_equal(0, is_ascii_id(NULL, 0));

    bxstr_t *s = bxs_from_ascii("");
    assert_int_equal(0, is_ascii_id(s, 0));
    bxs_free(s);

    s = bxs_from_ascii("a--b");
    assert_int_equal(0, is_ascii_id(s, 0));
    bxs_free(s);

    s = bxs_from_ascii("also-_invalid");
    assert_int_equal(0, is_ascii_id(s, 0));
    bxs_free(s);

    s = bxs_from_ascii("invalid-");
    assert_int_equal(0, is_ascii_id(s, 0));
    bxs_free(s);

    s = bxs_from_ascii("42");
    assert_int_equal(0, is_ascii_id(s, 0));
    bxs_free(s);

    uint32_t *ustr32 = u32_strconv_from_arg(
            "\xe5\x85\xac\xe7\x88\xb8\xe8\xa6\x81\xe9\x81\x93\xef\xbc\x81",  /* 公爸要道！ */
            "UTF-8");
    s = bxs_from_unicode(ustr32);
    assert_int_equal(0, is_ascii_id(s, 0));
    bxs_free(s);
    BFREE(ustr32);
}


void test_is_ascii_id_strict_valid(void **state)
{
    (void) state; /* unused */

    bxstr_t *s = bxs_from_ascii("valid");
    assert_int_equal(1, is_ascii_id(s, 1));
    bxs_free(s);

    s = bxs_from_ascii("also-valid");
    assert_int_equal(1, is_ascii_id(s, 1));
    bxs_free(s);

    s = bxs_from_ascii("fine2");
    assert_int_equal(1, is_ascii_id(s, 1));
    bxs_free(s);

    s = bxs_from_ascii("a");
    assert_int_equal(1, is_ascii_id(s, 1));
    bxs_free(s);
}


void test_is_ascii_id_strict_invalid(void **state)
{
    (void) state; /* unused */

    assert_int_equal(0, is_ascii_id(NULL, 1));

    bxstr_t *s = bxs_from_ascii("");
    assert_int_equal(0, is_ascii_id(s, 1));
    bxs_free(s);

    s = bxs_from_ascii("a--b");
    assert_int_equal(0, is_ascii_id(s, 1));
    bxs_free(s);

    s = bxs_from_ascii("also-_invalid");
    assert_int_equal(0, is_ascii_id(s, 1));
    bxs_free(s);

    s = bxs_from_ascii("invalid-");
    assert_int_equal(0, is_ascii_id(s, 1));
    bxs_free(s);

    s = bxs_from_ascii("42");
    assert_int_equal(0, is_ascii_id(s, 1));
    bxs_free(s);

    s = bxs_from_ascii("A_2");
    assert_int_equal(0, is_ascii_id(s, 1));
    bxs_free(s);
}


void test_repeat(void **state)
{
    (void) state; /* unused */

    char *actual = repeat(NULL, 1);
    assert_null(actual);

    actual = repeat("x", 0);
    assert_string_equal("", actual);
    BFREE(actual);

    actual = repeat("x", 3);
    assert_string_equal("xxx", actual);
    BFREE(actual);

    actual = repeat("abc", 3);
    assert_string_equal("abcabcabc", actual);
    BFREE(actual);
}


/* vim: set cindent sw=4: */
