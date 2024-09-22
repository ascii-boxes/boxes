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
 * Unit tests of the 'regulex' module
 */

#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <uniconv.h>
#include <string.h>

#include "boxes.h"
#include "global_mock.h"
#include "regulex.h"
#include "regulex_test.h"



void test_compile_pattern_empty(void **state)
{
    UNUSED(state);

    assert_null(compile_pattern(NULL));
    assert_non_null(compile_pattern(""));
}



void test_compile_pattern_error(void **state)
{
    UNUSED(state);

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



void test_regex_replace_invalid_utf(void **state)
{
    UNUSED(state);

    const char *input = "input";
    assert_null(regex_replace(compile_pattern("search"), NULL, /* NULL is an invalid replacement string*/
        u32_strconv_from_encoding(input, "ASCII", iconveh_question_mark), strlen(input), 0));
    assert_int_equal(1, collect_err_size);
    assert_string_equal("Failed to convert replacement string to UTF-32 - \"(null)\"\n", collect_err[0]);
}



void test_regex_replace_buffer_resize(void **state)
{
    UNUSED(state);

    const char *input = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    uint32_t *actual = regex_replace(compile_pattern("x"), "long_replacement_string_",
        u32_strconv_from_encoding(input, "ASCII", iconveh_question_mark), strlen(input), 1);
    char *actual_str = u32_strconv_to_encoding(actual, "ASCII", iconveh_question_mark);

    const char *expected = "long_replacement_string_long_replacement_string_long_replacement_string_"
        "long_replacement_string_long_replacement_string_long_replacement_string_long_replacement_string_"
        "long_replacement_string_long_replacement_string_long_replacement_string_long_replacement_string_"
        "long_replacement_string_long_replacement_string_long_replacement_string_long_replacement_string_"
        "long_replacement_string_long_replacement_string_long_replacement_string_long_replacement_string_"
        "long_replacement_string_long_replacement_string_long_replacement_string_long_replacement_string_"
        "long_replacement_string_long_replacement_string_long_replacement_string_long_replacement_string_"
        "long_replacement_string_long_replacement_string_long_replacement_string_long_replacement_string_"
        "long_replacement_string_long_replacement_string_long_replacement_string_long_replacement_string_"
        "long_replacement_string_long_replacement_string_long_replacement_string_long_replacement_string_";
    assert_string_equal(expected, actual_str);
}


void test_regex_replace_error(void **state)
{
    UNUSED(state);

    const char *input = "xxx";
    uint32_t *actual = regex_replace(compile_pattern("x"), "INVALID $2",
        u32_strconv_from_encoding(input, "ASCII", iconveh_question_mark), strlen(input), 0);
    assert_null(actual);
    assert_int_equal(1, collect_err_size);
    assert_string_equal("Error substituting \"INVALID $2\": unknown substring\n", collect_err[0]);
}


/* vim: set cindent sw=4: */
