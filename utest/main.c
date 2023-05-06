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
 * Main module for running the unit tests.
 */

#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <cmocka.h>

#include "global_mock.h"
#include "bxstring_test.h"
#include "cmdline_test.h"
#include "tools_test.h"
#include "regulex_test.h"
#include "unicode_test.h"



static int beforeTest(void** state)
{
    (void) state;  /* unused */

    collect_reset();
    return 0;
}



int main(void)
{
    setup_mocks();

    const struct CMUnitTest cmdline_tests[] = {
        cmocka_unit_test_setup(test_indentmode_none, beforeTest),
        cmocka_unit_test_setup(test_indentmode_invalid_long, beforeTest),
        cmocka_unit_test_setup(test_indentmode_invalid_short, beforeTest),
        cmocka_unit_test_setup(test_indentmode_box, beforeTest),
        cmocka_unit_test_setup(test_indentmode_text, beforeTest),
        cmocka_unit_test_setup(test_killblank_true, beforeTest),
        cmocka_unit_test_setup(test_killblank_false, beforeTest),
        cmocka_unit_test_setup(test_killblank_invalid, beforeTest),
        cmocka_unit_test_setup(test_killblank_multiple, beforeTest),
        cmocka_unit_test_setup(test_killblank_long, beforeTest),
        cmocka_unit_test_setup(test_padding_top_bottom, beforeTest),
        cmocka_unit_test_setup(test_padding_invalid, beforeTest),
        cmocka_unit_test_setup(test_padding_negative, beforeTest),
        cmocka_unit_test_setup(test_padding_notset, beforeTest),
        cmocka_unit_test_setup(test_padding_invalid_value, beforeTest),
        cmocka_unit_test_setup(test_padding_novalue, beforeTest),
        cmocka_unit_test_setup(test_tabstops_zero, beforeTest),
        cmocka_unit_test_setup(test_tabstops_500, beforeTest),
        cmocka_unit_test_setup(test_tabstops_4X, beforeTest),
        cmocka_unit_test_setup(test_tabstops_4e, beforeTest),
        cmocka_unit_test_setup(test_tabstops_4ex, beforeTest),
        cmocka_unit_test_setup(test_tabstops_7, beforeTest),
        cmocka_unit_test_setup(test_alignment_invalid_hX, beforeTest),
        cmocka_unit_test_setup(test_alignment_invalid_vX, beforeTest),
        cmocka_unit_test_setup(test_alignment_invalid_jX, beforeTest),
        cmocka_unit_test_setup(test_alignment_notset, beforeTest),
        cmocka_unit_test_setup(test_alignment_incomplete, beforeTest),
        cmocka_unit_test_setup(test_inputfiles_illegal_third_file, beforeTest),
        cmocka_unit_test_setup(test_inputfiles_stdin_stdout, beforeTest),
        cmocka_unit_test_setup(test_inputfiles_stdin, beforeTest),
        cmocka_unit_test_setup(test_inputfiles_input_nonexistent, beforeTest),
        cmocka_unit_test_setup(test_inputfiles_actual_success, beforeTest),
        cmocka_unit_test_setup(test_command_line_design_empty, beforeTest),
        cmocka_unit_test_setup(test_help, beforeTest),
        cmocka_unit_test_setup(test_version_requested, beforeTest)
    };

    const struct CMUnitTest regulex_tests[] = {
        cmocka_unit_test_setup(test_compile_pattern_error, beforeTest),
        cmocka_unit_test_setup(test_compile_pattern_empty, beforeTest),
        cmocka_unit_test_setup(test_regex_replace_invalid_utf, beforeTest),
        cmocka_unit_test_setup(test_regex_replace_buffer_resize, beforeTest),
        cmocka_unit_test_setup(test_regex_replace_error, beforeTest)
    };

    const struct CMUnitTest tools_tests[] = {
        cmocka_unit_test(test_strisyes_true),
        cmocka_unit_test(test_strisyes_false),
        cmocka_unit_test(test_strisno_true),
        cmocka_unit_test(test_strisno_false),
        cmocka_unit_test(test_my_strrspn_edge),
        cmocka_unit_test(test_my_strrspn),
        cmocka_unit_test(test_is_csi_reset),
        cmocka_unit_test(test_is_ascii_id_valid),
        cmocka_unit_test(test_is_ascii_id_invalid),
        cmocka_unit_test(test_is_ascii_id_strict_valid),
        cmocka_unit_test(test_is_ascii_id_strict_invalid)
    };

    const struct CMUnitTest unicode_tests[] = {
        cmocka_unit_test(test_to_utf32),
        cmocka_unit_test(test_is_blank),
        cmocka_unit_test(test_is_allowed_in_sample),
        cmocka_unit_test(test_is_allowed_in_shape),
        cmocka_unit_test(test_is_allowed_in_filename),
        cmocka_unit_test(test_is_allowed_in_kv_string)
    };

    const struct CMUnitTest bxstring_tests[] = {
        cmocka_unit_test_setup(test_ascii_simple, beforeTest),
        cmocka_unit_test_setup(test_ascii_illegalchar, beforeTest),
        cmocka_unit_test_setup(test_ascii_null, beforeTest),
        cmocka_unit_test_setup(test_ansi_unicode_book, beforeTest),
        cmocka_unit_test_setup(test_ansi_unicode_space_kinds, beforeTest),
        cmocka_unit_test_setup(test_ansi_unicode_chinese, beforeTest),
        cmocka_unit_test_setup(test_ansi_unicode_empty, beforeTest),
        cmocka_unit_test_setup(test_ansi_unicode_blanks, beforeTest),
        cmocka_unit_test_setup(test_ansi_unicode_invisible_only, beforeTest),
        cmocka_unit_test_setup(test_ansi_unicode_illegalchar, beforeTest),
        cmocka_unit_test_setup(test_ansi_unicode_tabs, beforeTest),
        cmocka_unit_test_setup(test_ansi_unicode_broken_escapes, beforeTest),
        cmocka_unit_test_setup(test_ansi_unicode_null, beforeTest),
        cmocka_unit_test_setup(test_bxs_strdup, beforeTest),
        cmocka_unit_test_setup(test_bxs_trimdup_null, beforeTest),
        cmocka_unit_test_setup(test_bxs_trimdup_invalid_startidx, beforeTest),
        cmocka_unit_test_setup(test_bxs_trimdup_invalid_endidx, beforeTest),
        cmocka_unit_test_setup(test_bxs_trimdup_invalid_endidx2, beforeTest),
        cmocka_unit_test_setup(test_bxs_trimdup_normal, beforeTest),
        cmocka_unit_test_setup(test_bxs_trimdup_vanish, beforeTest),
        cmocka_unit_test_setup(test_bxs_trimdup_ansi, beforeTest),
        cmocka_unit_test_setup(test_bxs_trimdup_ansi_same, beforeTest),
        cmocka_unit_test_setup(test_bxs_strcat, beforeTest),
        cmocka_unit_test_setup(test_bxs_strcat_empty, beforeTest),
        cmocka_unit_test_setup(test_bxs_strcat_empty2, beforeTest),
        cmocka_unit_test_setup(test_bxs_strcat_empty3, beforeTest),
        cmocka_unit_test_setup(test_bxs_strchr, beforeTest),
        cmocka_unit_test_setup(test_bxs_strchr_empty, beforeTest),
        cmocka_unit_test_setup(test_bxs_strchr_cursor, beforeTest),
        cmocka_unit_test_setup(test_bxs_trim, beforeTest),
        cmocka_unit_test_setup(test_bxs_trim_blanks, beforeTest),
        cmocka_unit_test_setup(test_bxs_trim_none, beforeTest),
        cmocka_unit_test_setup(test_bxs_rtrim, beforeTest),
        cmocka_unit_test_setup(test_bxs_rtrim_empty, beforeTest),
        cmocka_unit_test_setup(test_bxs_to_output, beforeTest),
        cmocka_unit_test_setup(test_bxs_is_empty_null, beforeTest),
        cmocka_unit_test_setup(test_bxs_is_visible_char, beforeTest),
        cmocka_unit_test_setup(test_bxs_strcmp, beforeTest),
        cmocka_unit_test_setup(test_bxs_valid_anywhere_error, beforeTest),
        cmocka_unit_test_setup(test_bxs_valid_in_filename_error, beforeTest),
        cmocka_unit_test_setup(test_bxs_free_null, beforeTest)
    };

    int num_failed = 0;
    num_failed += cmocka_run_group_tests(cmdline_tests, NULL, NULL);
    num_failed += cmocka_run_group_tests(regulex_tests, NULL, NULL);
    num_failed += cmocka_run_group_tests(tools_tests, NULL, NULL);
    num_failed += cmocka_run_group_tests(unicode_tests, NULL, NULL);
    num_failed += cmocka_run_group_tests(bxstring_tests, NULL, NULL);

    teardown();
    return num_failed;
}


/*EOF*/                                          /* vim: set cindent sw=4: */
