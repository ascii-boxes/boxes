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
#include "shape.h"
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



void test_shorten_preferleft_allowall(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shape_line_ctx = (shape_line_ctx_t *) calloc(1, sizeof(shape_line_ctx_t));
    shape_line_ctx->text = bxs_from_ascii("  abc  ");
    size_t quality = shape_line_ctx->text->num_chars;

    uint32_t *actual = shorten(shape_line_ctx, &quality, 1, 1, 1);
    uint32_t *expected = u32_strconv_from_arg(" abc  ", "ASCII");
    assert_int_equal(6, (int) quality);
    assert_int_equal(0, u32_strcmp(expected, actual));
    BFREE(actual);
    BFREE(expected);

    actual = shorten(shape_line_ctx, &quality, 1, 1, 1);
    expected = u32_strconv_from_arg("abc  ", "ASCII");
    assert_int_equal(5, (int) quality);
    assert_int_equal(0, u32_strcmp(expected, actual));
    BFREE(actual);
    BFREE(expected);

    actual = shorten(shape_line_ctx, &quality, 1, 1, 1);
    expected = u32_strconv_from_arg("abc ", "ASCII");
    assert_int_equal(4, (int) quality);
    assert_int_equal(0, u32_strcmp(expected, actual));
    BFREE(actual);
    BFREE(expected);

    actual = shorten(shape_line_ctx, &quality, 1, 1, 1);
    expected = u32_strconv_from_arg("abc", "ASCII");
    assert_int_equal(3, (int) quality);
    assert_int_equal(0, u32_strcmp(expected, actual));
    BFREE(actual);
    BFREE(expected);

    actual = shorten(shape_line_ctx, &quality, 1, 1, 1);
    assert_int_equal(3, (int) quality);
    assert_null(actual);
    BFREE(actual);

    bxs_free(shape_line_ctx->text);
    BFREE(shape_line_ctx);
}



void test_shorten_preferleft_allowright(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shape_line_ctx = (shape_line_ctx_t *) calloc(1, sizeof(shape_line_ctx_t));
    shape_line_ctx->text = bxs_from_ascii("  abc  ");
    size_t quality = shape_line_ctx->text->num_chars;

    uint32_t *actual = shorten(shape_line_ctx, &quality, 1, 0, 1);
    uint32_t *expected = u32_strconv_from_arg("  abc ", "ASCII");
    assert_int_equal(6, (int) quality);
    assert_int_equal(0, u32_strcmp(expected, actual));
    BFREE(actual);
    BFREE(expected);

    actual = shorten(shape_line_ctx, &quality, 1, 0, 1);
    expected = u32_strconv_from_arg("  abc", "ASCII");
    assert_int_equal(5, (int) quality);
    assert_int_equal(0, u32_strcmp(expected, actual));
    BFREE(actual);
    BFREE(expected);

    actual = shorten(shape_line_ctx, &quality, 1, 0, 1);
    assert_int_equal(5, (int) quality);
    assert_null(actual);
    BFREE(actual);

    bxs_free(shape_line_ctx->text);
    BFREE(shape_line_ctx);
}



void test_shorten_preferleft_allowleft(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shape_line_ctx = (shape_line_ctx_t *) calloc(1, sizeof(shape_line_ctx_t));
    shape_line_ctx->text = bxs_from_ascii("  abc  ");
    size_t quality = shape_line_ctx->text->num_chars;

    uint32_t *actual = shorten(shape_line_ctx, &quality, 1, 1, 0);
    uint32_t *expected = u32_strconv_from_arg(" abc  ", "ASCII");
    assert_int_equal(6, (int) quality);
    assert_int_equal(0, u32_strcmp(expected, actual));
    BFREE(actual);
    BFREE(expected);

    actual = shorten(shape_line_ctx, &quality, 1, 1, 0);
    expected = u32_strconv_from_arg("abc  ", "ASCII");
    assert_int_equal(5, (int) quality);
    assert_int_equal(0, u32_strcmp(expected, actual));
    BFREE(actual);
    BFREE(expected);

    actual = shorten(shape_line_ctx, &quality, 1, 1, 0);
    assert_int_equal(5, (int) quality);
    assert_null(actual);
    BFREE(actual);

    bxs_free(shape_line_ctx->text);
    BFREE(shape_line_ctx);
}



void test_shorten_preferright_allowall(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shape_line_ctx = (shape_line_ctx_t *) calloc(1, sizeof(shape_line_ctx_t));
    shape_line_ctx->text = bxs_from_ascii("  abc  ");
    size_t quality = shape_line_ctx->text->num_chars;

    uint32_t *actual = shorten(shape_line_ctx, &quality, 0, 1, 1);
    uint32_t *expected = u32_strconv_from_arg("  abc ", "ASCII");
    assert_int_equal(6, (int) quality);
    assert_int_equal(0, u32_strcmp(expected, actual));
    BFREE(actual);
    BFREE(expected);

    actual = shorten(shape_line_ctx, &quality, 0, 1, 1);
    expected = u32_strconv_from_arg("  abc", "ASCII");
    assert_int_equal(5, (int) quality);
    assert_int_equal(0, u32_strcmp(expected, actual));
    BFREE(actual);
    BFREE(expected);

    actual = shorten(shape_line_ctx, &quality, 0, 1, 1);
    expected = u32_strconv_from_arg(" abc", "ASCII");
    assert_int_equal(4, (int) quality);
    assert_int_equal(0, u32_strcmp(expected, actual));
    BFREE(actual);
    BFREE(expected);

    actual = shorten(shape_line_ctx, &quality, 0, 1, 1);
    expected = u32_strconv_from_arg("abc", "ASCII");
    assert_int_equal(3, (int) quality);
    assert_int_equal(0, u32_strcmp(expected, actual));
    BFREE(actual);
    BFREE(expected);

    actual = shorten(shape_line_ctx, &quality, 0, 1, 1);
    assert_int_equal(3, (int) quality);
    assert_null(actual);
    BFREE(actual);

    bxs_free(shape_line_ctx->text);
    BFREE(shape_line_ctx);
}



void test_shorten_corner_cases(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shape_line_ctx = (shape_line_ctx_t *) calloc(1, sizeof(shape_line_ctx_t));
    shape_line_ctx->text = bxs_from_ascii("  abc  ");
    size_t quality = shape_line_ctx->text->num_chars;

    uint32_t *actual = shorten(shape_line_ctx, &quality, 0, 0, 0);
    assert_int_equal(7, (int) quality);
    assert_null(actual);

    actual = shorten(NULL, &quality, 1, 1, 0);
    assert_int_equal(7, (int) quality);
    assert_null(actual);

    quality = 100;  /* > 7 */
    actual = shorten(shape_line_ctx, &quality, 0, 0, 0);
    assert_int_equal(100, (int) quality);
    assert_null(actual);

    actual = shorten(shape_line_ctx, NULL, 0, 0, 1);
    assert_null(actual);  /* quality == NULL */

    quality = 7;
    bxs_free(shape_line_ctx->text);
    shape_line_ctx->text = NULL;
    actual = shorten(shape_line_ctx, &quality, 0, 0, 1);
    assert_null(actual);  /* text == NULL */

    BFREE(shape_line_ctx);
}



static shape_line_ctx_t *build_shapes_relevant_sunny()
{
    shape_line_ctx_t *shapes_relevant = (shape_line_ctx_t *) calloc(SHAPES_PER_SIDE, sizeof(shape_line_ctx_t));
    shapes_relevant[0].empty = 0;
    shapes_relevant[0].text = bxs_from_ascii("WCORNER");
    shapes_relevant[0].elastic = 0;
    shapes_relevant[1].empty = 0;
    shapes_relevant[1].text = bxs_from_ascii("-");
    shapes_relevant[1].elastic = 1;
    shapes_relevant[2].empty = 0;
    shapes_relevant[2].text = bxs_from_ascii("middle");
    shapes_relevant[2].elastic = 0;
    shapes_relevant[3].empty = 0;
    shapes_relevant[3].text = bxs_from_ascii("-");
    shapes_relevant[3].elastic = 1;
    shapes_relevant[4].empty = 0;
    shapes_relevant[4].text = bxs_from_ascii("ECORNER");
    shapes_relevant[4].elastic = 0;
    return shapes_relevant;
}



static void free_shapes_relevant(shape_line_ctx_t *shapes_relevant)
{
    for (size_t i = 0; i < SHAPES_PER_SIDE; i++) {
        bxs_free(shapes_relevant[i].text);
    }
    BFREE(shapes_relevant);
}



void test_hmm_sunny_day(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shapes_relevant = build_shapes_relevant_sunny();

    uint32_t *input_line = u32_strconv_from_arg("  WCORNER-----middle-----ECORNER  ", "ASCII");
    uint32_t *cur_pos = input_line + 9;  /* '-' after WCORNER */
    uint32_t *end_pos = input_line + 25; /* 'E' of ECORNER */

    int actual = hmm(shapes_relevant, cur_pos, 1, end_pos, 1, 1);

    assert_int_equal(1, actual);

    free_shapes_relevant(shapes_relevant);
    BFREE(input_line);
}



void test_hmm_sunny_day_short(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shapes_relevant = build_shapes_relevant_sunny();

    uint32_t *input_line = u32_strconv_from_arg("WCORNER-middle-", "ASCII");
    uint32_t *cur_pos = input_line + 7;  /* '-' after WCORNER */
    uint32_t *end_pos = input_line + 15; /* NUL */

    int actual = hmm(shapes_relevant, cur_pos, 1, end_pos, 1, 0);

    assert_int_equal(1, actual);

    free_shapes_relevant(shapes_relevant);
    BFREE(input_line);
}



void test_hmm_missing_elastic_nne(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shapes_relevant = build_shapes_relevant_sunny();

    uint32_t *input_line = u32_strconv_from_arg("WCORNER---middleECORNER", "ASCII");
    uint32_t *cur_pos = input_line + 7;  /* '-' after WCORNER */
    uint32_t *end_pos = input_line + 16; /* 'E' of ECORNER */

    int actual = hmm(shapes_relevant, cur_pos, 1, end_pos, 1, 1);

    assert_int_equal(0, actual); /* should fail because NNE shape is not present */

    free_shapes_relevant(shapes_relevant);
    BFREE(input_line);
}



void test_hmm_invalid_input(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shapes_relevant = build_shapes_relevant_sunny();

    uint32_t *input_line = u32_strconv_from_arg("not_used", "ASCII");
    uint32_t *cur_pos = input_line + 5;
    uint32_t *end_pos = input_line + 2; /* before cur_pos, which is an error */

    int actual = hmm(shapes_relevant, cur_pos, 1, end_pos, 1, 1);

    assert_int_equal(0, actual); /* should fail because cur_pos > end_pos */

    free_shapes_relevant(shapes_relevant);
    BFREE(input_line);
}



static shape_line_ctx_t *build_shapes_relevant_empty_r()
{
    shape_line_ctx_t *shapes_relevant = (shape_line_ctx_t *) calloc(SHAPES_PER_SIDE, sizeof(shape_line_ctx_t));
    shapes_relevant[0].empty = 0;
    shapes_relevant[0].text = bxs_from_ascii("WCORNER");
    shapes_relevant[0].elastic = 0;
    shapes_relevant[1].empty = 1;
    shapes_relevant[1].text = NULL;
    shapes_relevant[1].elastic = 0;
    shapes_relevant[2].empty = 0;
    shapes_relevant[2].text = bxs_from_ascii("xx");
    shapes_relevant[2].elastic = 1;
    shapes_relevant[3].empty = 1;
    shapes_relevant[3].text =  NULL;
    shapes_relevant[3].elastic = 0;
    shapes_relevant[4].empty = 1;
    shapes_relevant[4].text = NULL;
    shapes_relevant[4].elastic = 0;
    return shapes_relevant;
}



void test_hmm_empty_shapes_success(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shapes_relevant = build_shapes_relevant_empty_r();

    uint32_t *input_line = u32_strconv_from_arg(" WCORNERxxxxxxxx", "ASCII");
    uint32_t *cur_pos = input_line + 8;  /* first 'x' */
    uint32_t *end_pos = input_line + 16; /* NUL */

    int actual = hmm(shapes_relevant, cur_pos, 1, end_pos, 1, 1);

    assert_int_equal(1, actual); /* matches */

    free_shapes_relevant(shapes_relevant);
    BFREE(input_line);
}



static shape_line_ctx_t *build_shapes_relevant_backtrack()
{
    shape_line_ctx_t *shapes_relevant = (shape_line_ctx_t *) calloc(SHAPES_PER_SIDE, sizeof(shape_line_ctx_t));
    shapes_relevant[0].empty = 0;
    shapes_relevant[0].text = bxs_from_ascii("WCORNER");
    shapes_relevant[0].elastic = 0;
    shapes_relevant[1].empty = 0;
    shapes_relevant[1].text = bxs_from_ascii("-");
    shapes_relevant[1].elastic = 1;
    shapes_relevant[2].empty = 0;
    shapes_relevant[2].text = bxs_from_ascii("--middle--");
    shapes_relevant[2].elastic = 0;
    shapes_relevant[3].empty = 0;
    shapes_relevant[3].text = bxs_from_ascii("-");
    shapes_relevant[3].elastic = 1;
    shapes_relevant[4].empty = 1;
    shapes_relevant[4].text = NULL;
    shapes_relevant[4].elastic = 0;
    return shapes_relevant;
}



void test_hmm_backtracking(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shapes_relevant = build_shapes_relevant_backtrack();

    uint32_t *input_line = u32_strconv_from_arg(" WCORNER-----middle-----", "ASCII");
    uint32_t *cur_pos = input_line + 8;  /* first '-' */
    uint32_t *end_pos = input_line + 24; /* NUL */

    int actual = hmm(shapes_relevant, cur_pos, 1, end_pos, 1, 1);

    assert_int_equal(1, actual); /* matches, elastic NNW doesn't eat part of N shape */

    free_shapes_relevant(shapes_relevant);
    BFREE(input_line);
}



static shape_line_ctx_t *build_shapes_relevant_shiftable()
{
    shape_line_ctx_t *shapes_relevant = (shape_line_ctx_t *) calloc(SHAPES_PER_SIDE, sizeof(shape_line_ctx_t));
    shapes_relevant[0].empty = 0; /* maybe there is text in a different shape line */
    shapes_relevant[0].text = bxs_from_ascii("  ");
    shapes_relevant[0].elastic = 0;
    shapes_relevant[1].empty = 1;
    shapes_relevant[1].text = bxs_from_ascii("   ");
    shapes_relevant[1].elastic = 1;
    shapes_relevant[2].empty = 0;
    shapes_relevant[2].text = bxs_from_ascii("  NORTH  ");
    shapes_relevant[2].elastic = 0;
    shapes_relevant[3].empty = 1;
    shapes_relevant[3].text = NULL;
    shapes_relevant[3].elastic = 1;
    shapes_relevant[4].empty = 1;
    shapes_relevant[4].text = NULL;
    shapes_relevant[4].elastic = 0;
    return shapes_relevant;
}



void test_hmm_shiftable(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shapes_relevant = build_shapes_relevant_shiftable();

    uint32_t *input_line = u32_strconv_from_arg("      NORTH      ", "ASCII");
    uint32_t *cur_pos = input_line;      /* first character */
    uint32_t *end_pos = input_line + 17; /* NUL */

    int actual = hmm(shapes_relevant, cur_pos, 1, end_pos, 0, 0);

    assert_int_equal(1, actual); /* matches */

    free_shapes_relevant(shapes_relevant);
    BFREE(input_line);
}



void test_hmm_shortened(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shapes_relevant = build_shapes_relevant_shiftable();

    uint32_t *input_line = u32_strconv_from_arg("NORTH", "ASCII");
    uint32_t *cur_pos = input_line;      /* first character */
    uint32_t *end_pos = input_line + 5;  /* NUL */

    int actual = hmm(shapes_relevant, cur_pos, 1, end_pos, 0, 0);

    assert_int_equal(1, actual); /* matches, because "  NORTH  " can be shortened because shiftable */

    free_shapes_relevant(shapes_relevant);
    BFREE(input_line);
}



static shape_line_ctx_t *build_shapes_relevant_shiftable_left()
{
    shape_line_ctx_t *shapes_relevant = (shape_line_ctx_t *) calloc(SHAPES_PER_SIDE, sizeof(shape_line_ctx_t));
    shapes_relevant[0].empty = 0; /* maybe there is text in a different shape line */
    shapes_relevant[0].text = bxs_from_ascii("  ");
    shapes_relevant[0].elastic = 0;
    shapes_relevant[1].empty = 1;
    shapes_relevant[1].text = bxs_from_ascii("   ");
    shapes_relevant[1].elastic = 1;
    shapes_relevant[2].empty = 0;
    shapes_relevant[2].text = bxs_from_ascii("  NORTH  ");
    shapes_relevant[2].elastic = 0;
    shapes_relevant[3].empty = 1;
    shapes_relevant[3].text = NULL;
    shapes_relevant[3].elastic = 1;
    shapes_relevant[4].empty = 0;
    shapes_relevant[4].text = bxs_from_ascii("ECORNER  ");
    shapes_relevant[4].elastic = 0;
    return shapes_relevant;
}



void test_hmm_shortened_right_fail(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shapes_relevant = build_shapes_relevant_shiftable_left();

    uint32_t *input_line = u32_strconv_from_arg("NORTH ECORNER  ", "ASCII");
    uint32_t *cur_pos = input_line;      /* first character */
    uint32_t *end_pos = input_line + 6;  /* 'E' of "ECORNER" */

    int actual = hmm(shapes_relevant, cur_pos, 1, end_pos, 0, 1);

    assert_int_equal(0, actual); /* does not match */
                                 /* because "  NORTH  " cannot be shortened right b/c right not shiftable */

    free_shapes_relevant(shapes_relevant);
    BFREE(input_line);
}



void test_hmm_shortened_right(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shapes_relevant = build_shapes_relevant_shiftable_left();

    uint32_t *input_line = u32_strconv_from_arg("NORTH  ECORNER", "ASCII");
    uint32_t *cur_pos = input_line;      /* first character */
    uint32_t *end_pos = input_line + 7;  /* 'E' of "ECORNER" */

    int actual = hmm(shapes_relevant, cur_pos, 1, end_pos, 0, 1);

    assert_int_equal(1, actual); /* matches */
                                 /* because "  NORTH  " can be shortened left, and ECORNER can be shortened right */

    free_shapes_relevant(shapes_relevant);
    BFREE(input_line);
}



static shape_line_ctx_t *build_shapes_relevant_blank()
{
    shape_line_ctx_t *shapes_relevant = (shape_line_ctx_t *) calloc(SHAPES_PER_SIDE, sizeof(shape_line_ctx_t));
    shapes_relevant[0].empty = 0; /* maybe there is text in a different shape line */
    shapes_relevant[0].text = bxs_from_ascii("  ");
    shapes_relevant[0].elastic = 0;
    shapes_relevant[1].empty = 0;
    shapes_relevant[1].text = bxs_from_ascii("   ");
    shapes_relevant[1].elastic = 1;
    shapes_relevant[2].empty = 0;
    shapes_relevant[2].text = bxs_from_ascii(" ");
    shapes_relevant[2].elastic = 0;
    shapes_relevant[3].empty = 1;
    shapes_relevant[3].text = bxs_from_ascii(" ");
    shapes_relevant[3].elastic = 1;
    shapes_relevant[4].empty = 1;
    shapes_relevant[4].text = NULL;
    shapes_relevant[4].elastic = 0;
    return shapes_relevant;
}



void test_hmm_blank_shiftable(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shapes_relevant = build_shapes_relevant_blank();

    uint32_t *input_line = u32_strconv_from_arg("      ", "ASCII");
    uint32_t *cur_pos = input_line;      /* first character */
    uint32_t *end_pos = input_line + 6;  /* NUL */

    int actual = hmm(shapes_relevant, cur_pos, 1, end_pos, 0, 0);

    assert_int_equal(1, actual); /* matches */

    free_shapes_relevant(shapes_relevant);
    BFREE(input_line);
}



void test_hmm_blank(void **state)
{
    UNUSED(state);

    shape_line_ctx_t *shapes_relevant = build_shapes_relevant_blank();

    uint32_t *input_line = u32_strconv_from_arg("WCORNER       ECORNER", "ASCII");
    uint32_t *cur_pos = input_line + 7;   /* first blank */
    uint32_t *end_pos = input_line + 14;  /* 'E' of "ECORNER" */

    int actual = hmm(shapes_relevant, cur_pos, 1, end_pos, 1, 1);

    assert_int_equal(1, actual); /* matches */

    free_shapes_relevant(shapes_relevant);
    BFREE(input_line);
}


/* vim: set cindent sw=4: */
