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

#ifndef CMDLINE_TEST_H
#define CMDLINE_TEST_H


void test_indentmode_none(void **state);
void test_indentmode_invalid_long(void **state);
void test_indentmode_invalid_short(void **state);
void test_indentmode_box(void **state);
void test_indentmode_text(void **state);

void test_killblank_true(void **state);
void test_killblank_false(void **state);
void test_killblank_invalid(void **state);
void test_killblank_multiple(void **state);
void test_killblank_long(void **state);

void test_padding_top_bottom(void **state);
void test_padding_invalid(void **state);
void test_padding_negative(void **state);
void test_padding_notset(void **state);
void test_padding_invalid_value(void **state);
void test_padding_novalue(void **state);

void test_tabstops_zero(void **state);
void test_tabstops_500(void **state);
void test_tabstops_4X(void **state);
void test_tabstops_4e(void **state);
void test_tabstops_4ex(void **state);
void test_tabstops_7(void **state);

void test_alignment_invalid_hX(void **state);
void test_alignment_invalid_vX(void **state);
void test_alignment_invalid_jX(void **state);
void test_alignment_notset(void **state);
void test_alignment_incomplete(void **state);

void test_inputfiles_illegal_third_file(void **state);
void test_inputfiles_stdin_stdout(void **state);
void test_inputfiles_stdin(void **state);
void test_inputfiles_input_nonexistent(void **state);
void test_inputfiles_actual_success(void **state);

void test_command_line_design_empty(void **state);

void test_help(void **state);
void test_version_requested(void **state);


#endif

/* vim: set cindent sw=4: */
