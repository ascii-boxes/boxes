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

#ifndef REMOVE_TEST_H
#define REMOVE_TEST_H

#include "bxstring.h"
#include "remove.h"


/* defined here and not in remove.h because these functions are only visible for testing */
match_result_t *match_outer_shape(int vside, bxstr_t *input_line, bxstr_t *shape_line);
int hmm(shape_line_ctx_t *shapes_relevant, uint32_t *cur_pos, size_t shape_idx, uint32_t *end_pos, int anchored_left,
        int anchored_right);
uint32_t *shorten(shape_line_ctx_t *shape_line_ctx, size_t *quality, int prefer_left, int allow_left, int allow_right);
void toblank_mixed(bxstr_t *org_line, uint32_t **s, size_t from, size_t to);


void test_match_outer_shape_null(void **state);
void test_match_outer_shape_left_anchored(void **state);
void test_match_outer_shape_left_shiftable(void **state);
void test_match_outer_shape_left_shortened(void **state);
void test_match_outer_shape_left_not_found(void **state);
void test_match_outer_shape_left_too_far_in(void **state);
void test_match_outer_shape_left_edge(void **state);
void test_match_outer_shape_right_anchored(void **state);
void test_match_outer_shape_right_shiftable(void **state);
void test_match_outer_shape_right_shortened(void **state);
void test_match_outer_shape_right_shortened2(void **state);
void test_match_outer_shape_right_not_found(void **state);
void test_match_outer_shape_right_too_far_in(void **state);
void test_match_outer_shape_right_edge(void **state);

void test_shorten_preferleft_allowall(void **state);
void test_shorten_preferleft_allowright(void **state);
void test_shorten_preferleft_allowleft(void **state);
void test_shorten_preferright_allowall(void **state);
void test_shorten_corner_cases(void **state);

void test_hmm_sunny_day(void **state);
void test_hmm_sunny_day_short(void **state);
void test_hmm_missing_elastic_nne(void **state);
void test_hmm_invalid_input(void **state);
void test_hmm_empty_shapes_success(void **state);
void test_hmm_backtracking(void **state);
void test_hmm_shiftable(void **state);
void test_hmm_shortened(void **state);
void test_hmm_shortened_right_fail(void **state);
void test_hmm_shortened_right(void **state);
void test_hmm_blank_shiftable(void **state);
void test_hmm_blank(void **state);


#endif


/* vim: set cindent sw=4: */
