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
 * Unit tests of the 'cmdline' module
 */

#ifndef BXSTRING_TEST_H
#define BXSTRING_TEST_H


void test_ascii_simple(void **state);
void test_ascii_illegalchar(void **state);
void test_ascii_null(void **state);

void test_ansi_unicode_book(void **state);
void test_ansi_unicode_space_kinds(void **state);
void test_ansi_unicode_chinese(void **state);
void test_ansi_unicode_empty(void **state);
void test_ansi_unicode_blanks(void **state);
void test_ansi_unicode_invisible_only(void **state);
void test_ansi_unicode_illegalchar(void **state);
void test_ansi_unicode_tabs(void **state);
void test_ansi_unicode_broken_escapes(void **state);
void test_ansi_unicode_null(void **state);
void test_ansi_unicode_tc183(void **state);

void test_bxs_new_empty_string(void **state);

void test_bxs_is_blank(void **state);

void test_bxs_strdup(void **state);

void test_bxs_cut_front(void **state);
void test_bxs_cut_front_zero(void **state);

void test_bxs_first_char_ptr_errors(void **state);

void test_bxs_last_char_ptr(void **state);

void test_bxs_unindent_ptr_null(void **state);

void test_bxs_trimdup_null(void **state);
void test_bxs_trimdup_invalid_startidx(void **state);
void test_bxs_trimdup_invalid_endidx(void **state);
void test_bxs_trimdup_invalid_endidx2(void **state);
void test_bxs_trimdup_normal(void **state);
void test_bxs_trimdup_vanish(void **state);
void test_bxs_trimdup_ansi(void **state);
void test_bxs_trimdup_ansi_same(void **state);

void test_bxs_substr_errors(void **state);

void test_bxs_strcat(void **state);
void test_bxs_strcat_empty(void **state);
void test_bxs_strcat_empty2(void **state);
void test_bxs_strcat_empty3(void **state);

void test_bxs_strchr(void **state);
void test_bxs_strchr_empty(void **state);
void test_bxs_strchr_cursor(void **state);

void test_bxs_trim(void **state);
void test_bxs_trim_blanks(void **state);
void test_bxs_trim_none(void **state);

void test_bxs_ltrim2(void **state);
void test_bxs_ltrim5(void **state);
void test_bxs_ltrim_empty(void **state);
void test_bxs_ltrim_max(void **state);

void test_bxs_rtrim(void **state);
void test_bxs_rtrim_empty(void **state);

void test_bxs_prepend_spaces_null(void **state);

void test_bxs_append_spaces(void **state);

void test_bxs_to_output(void **state);

void test_bxs_is_empty_null(void **state);

void test_bxs_is_visible_char(void **state);

void test_bxs_filter_visible(void **state);
void test_bxs_filter_visible_none(void **state);

void test_bxs_strcmp(void **state);

void test_bxs_valid_anywhere_error(void **state);
void test_bxs_valid_in_filename_error(void **state);

void test_bxs_free_null(void **state);

void test_bxs_concat(void **state);
void test_bxs_concat_nullarg(void **state);


#endif

/* vim: set cindent sw=4: */
