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
 * Unit tests of the 'remove' module
 */

#include "config.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistr.h>

#include <cmocka.h>

#include "boxes.h"
#include "unicode.h"
#include "tools.h"
#include "remove_test.h"
#include "global_mock.h"
#include "utest_tools.h"



void test_match_outer_shape_null(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("    xx x xx    ");
    bxstr_t *shape_line = bxs_from_ascii(" xx ");

    match_result_t *actual = match_outer_shape(BLEF, NULL, shape_line);
    assert_null(actual);

    actual = match_outer_shape(BLEF, input_line, NULL);
    assert_null(actual);

    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_left_anchored(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("    xx x xx    ");
    bxstr_t *shape_line = bxs_from_ascii(" xx ");

    match_result_t *actual = match_outer_shape(BLEF, input_line, shape_line);
    uint32_t *expected_p = bxs_first_char_ptr(input_line, 3);
    assert_non_null(actual);
    assert_int_equal(3, (int) actual->p_idx);
    assert_int_equal(0, u32_strcmp(expected_p, actual->p));
    assert_int_equal(4, actual->len);
    assert_int_equal(0, actual->shiftable);

    BFREE(actual);
    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_left_shiftable(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("    xx x xx    ");
    bxstr_t *shape_line = bxs_from_ascii("  ");

    match_result_t *actual = match_outer_shape(BLEF, input_line, shape_line);
    assert_non_null(actual);
    assert_int_equal(0, (int) actual->p_idx);
    assert_int_equal(0, u32_strcmp(input_line->memory, actual->p));
    assert_int_equal(0, actual->len);
    assert_int_equal(1, actual->shiftable);

    BFREE(actual);
    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_left_shortened(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("  xx x xx  ");
    bxstr_t *shape_line = bxs_from_ascii("    xx");  /* length 6 */

    match_result_t *actual = match_outer_shape(BLEF, input_line, shape_line);
    assert_non_null(actual);
    assert_int_equal(0, (int) actual->p_idx);
    assert_int_equal(0, u32_strcmp(input_line->memory, actual->p));
    assert_int_equal(4, actual->len);  /* only 4 characters matched */
    assert_int_equal(0, actual->shiftable);

    BFREE(actual);
    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_left_not_found(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("  xx x xx  ");
    bxstr_t *shape_line = bxs_from_ascii(" ---");

    match_result_t *actual = match_outer_shape(BLEF, input_line, shape_line);
    assert_null(actual);

    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_left_too_far_in(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("    xx M xx    ");
    bxstr_t *shape_line = bxs_from_ascii("x M");

    match_result_t *actual = match_outer_shape(BLEF, input_line, shape_line);
    assert_null(actual);

    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_left_edge(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("   x yy");
    bxstr_t *shape_line = bxs_from_ascii("x");

    match_result_t *actual = match_outer_shape(BLEF, input_line, shape_line);

    uint32_t *expected_p = bxs_first_char_ptr(input_line, 3);
    assert_non_null(actual);
    assert_int_equal(3, (int) actual->p_idx);
    assert_int_equal(0, u32_strcmp(expected_p, actual->p));
    assert_int_equal(1, actual->len);
    assert_int_equal(0, actual->shiftable);

    BFREE(actual);
    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_right_anchored(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("    xx x xx    ");
    bxstr_t *shape_line = bxs_from_ascii(" xx ");

    match_result_t *actual = match_outer_shape(BRIG, input_line, shape_line);
    uint32_t *expected_p = bxs_first_char_ptr(input_line, 8);
    assert_non_null(actual);
    assert_int_equal(8, (int) actual->p_idx);
    assert_int_equal(0, u32_strcmp(expected_p, actual->p));
    assert_int_equal(4, actual->len);
    assert_int_equal(0, actual->shiftable);

    BFREE(actual);
    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_right_shiftable(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("    xx x xx    ");
    bxstr_t *shape_line = bxs_from_ascii("  ");

    match_result_t *actual = match_outer_shape(BRIG, input_line, shape_line);
    assert_non_null(actual);
    assert_int_equal(15, (int) actual->p_idx);
    assert_int_equal(1, is_char_at(actual->p, 0, char_nul));
    assert_int_equal(0, actual->len);
    assert_int_equal(1, actual->shiftable);

    BFREE(actual);
    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_right_shortened(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("  xx x xx  ");
    bxstr_t *shape_line = bxs_from_ascii("xx    ");  /* length 6 */

    match_result_t *actual = match_outer_shape(BRIG, input_line, shape_line);

    uint32_t *expected_p = bxs_first_char_ptr(input_line, 7);
    assert_non_null(actual);
    assert_int_equal(7, (int) actual->p_idx);
    assert_int_equal(0, u32_strcmp(expected_p, actual->p));
    assert_int_equal(4, actual->len);  /* only 4 characters matched */
    assert_int_equal(0, actual->shiftable);

    BFREE(actual);
    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_right_not_found(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("  xx x xx  ");
    bxstr_t *shape_line = bxs_from_ascii("--- ");

    match_result_t *actual = match_outer_shape(BRIG, input_line, shape_line);
    assert_null(actual);

    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_right_too_far_in(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("    xx M xx    ");
    bxstr_t *shape_line = bxs_from_ascii("M x");

    match_result_t *actual = match_outer_shape(BRIG, input_line, shape_line);
    assert_null(actual);

    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_right_shortened2(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("  x Y");
    bxstr_t *shape_line = bxs_from_ascii("Y  ");

    match_result_t *actual = match_outer_shape(BRIG, input_line, shape_line);

    uint32_t *expected_p = bxs_first_char_ptr(input_line, 4);
    assert_non_null(actual);
    assert_int_equal(4, (int) actual->p_idx);
    assert_int_equal(0, u32_strcmp(expected_p, actual->p));
    assert_int_equal(1, actual->len);
    assert_int_equal(0, actual->shiftable);

    BFREE(actual);
    bxs_free(input_line);
    bxs_free(shape_line);
}



void test_match_outer_shape_right_edge(void **state)
{
    UNUSED(state);

    bxstr_t *input_line = bxs_from_ascii("x");
    bxstr_t *shape_line = bxs_from_ascii("x");

    match_result_t *actual = match_outer_shape(BRIG, input_line, shape_line);

    assert_non_null(actual);
    assert_int_equal(0, (int) actual->p_idx);
    assert_int_equal(0, u32_strcmp(input_line->memory, actual->p));
    assert_int_equal(1, actual->len);
    assert_int_equal(0, actual->shiftable);

    BFREE(actual);
    bxs_free(input_line);
    bxs_free(shape_line);
}


/* vim: set cindent sw=4: */
