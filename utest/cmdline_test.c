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

#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "boxes.h"
#include "cmdline.h"
#include "tools.h"
#include "global_mock.h"
#include "cmdline_test.h"


static opt_t *act(const int num_args, ...)
{
    assert_true(num_args >= 0);

    int argc = 1;
    char **argv = (char **) calloc(argc + 1, sizeof(char *));
    argv[0] = "out/boxes";

    if (num_args > 0) {
        va_list va;
        va_start(va, num_args);
        for (int i=0; i<num_args; i++) {
            char *arg = va_arg(va, char *);
            ++argc;
            argv = (char **) realloc(argv, (argc + 1) * sizeof(char *));
            argv[argc - 1] = arg;
        }
        va_end(va);
    }
    argv[argc] = NULL;

    opt_t *actual = process_commandline(argc, argv);
    BFREE(argv);
    return actual;
}


void test_indentmode_none(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-i", "none");

    assert_non_null(actual);
    assert_int_equal((int) 'n', (int) actual->indentmode);
}


void test_indentmode_invalid_long(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-i", "INVALID");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid indentation mode\n", collect_err[0]);
}


void test_indentmode_invalid_short(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-i", "X");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid indentation mode\n", collect_err[0]);
}


void test_indentmode_box(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-i", "BO");

    assert_non_null(actual);
    assert_int_equal((int) 'b', (int) actual->indentmode);
}


void test_indentmode_text(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-i", "t");

    assert_non_null(actual);
    assert_int_equal((int) 't', (int) actual->indentmode);
}


void test_killblank_true(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-k", "true");

    assert_non_null(actual);
    assert_int_equal(1, actual->killblank);
}


void test_killblank_false(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-k", "false");

    assert_non_null(actual);
    assert_int_equal(0, actual->killblank);
}


void test_killblank_invalid(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-k", "INVALID");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: -k: invalid parameter\n", collect_err[0]);
}


void test_killblank_multiple(void **state)
{
    UNUSED(state);

    opt_t *actual = act(4, "-k", "true", "-k", "false");    // first one wins

    assert_non_null(actual);
    assert_int_equal(1, actual->killblank);
}


void test_killblank_long(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "--kill-blank", "--remove");

    assert_non_null(actual);
    assert_int_equal(1, actual->killblank);

    actual = act(2, "--no-kill-blank", "--remove");

    assert_non_null(actual);
    assert_int_equal(0, actual->killblank);
}


void test_padding_top_bottom(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-p", "t2b10");

    assert_non_null(actual);
    assert_int_equal(2, actual->padding[BTOP]);
    assert_int_equal(10, actual->padding[BBOT]);
}


void test_padding_invalid(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-p", "INVALID");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid padding specification - INVALID\n", collect_err[0]);
}


void test_padding_negative(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-p", "a-1");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid padding specification - a-1\n", collect_err[0]);
}


void test_padding_notset(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-p", "");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid padding specification - \n", collect_err[0]);
}


void test_padding_invalid_value(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-p", "l2rX");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid padding specification - l2rX\n", collect_err[0]);
}


void test_padding_novalue(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-p", "a");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid padding specification - a\n", collect_err[0]);
}


void test_tabstops_zero(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-t", "0");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid tab stop distance -- 0\n", collect_err[0]);
}


void test_tabstops_500(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-t", "500");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid tab stop distance -- 500\n", collect_err[0]);
}


void test_tabstops_4X(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-t", "4X");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid tab handling specification - 4X\n", collect_err[0]);
}


void test_tabstops_4e(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-t", "4e");

    assert_non_null(actual);
    assert_int_equal(4, actual->tabstop);
    assert_int_equal((int) 'e', (int) actual->tabexp);
}


void test_tabstops_4ex(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-t", "4ex");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: invalid tab handling specification - 4ex\n", collect_err[0]);
}


void test_tabstops_7(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-t", "7");

    assert_non_null(actual);
    assert_int_equal(7, actual->tabstop);
    assert_int_equal((int) 'e', (int) actual->tabexp);
}


void test_alignment_invalid_hX(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-a", "hX");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: Illegal text format -- hX\n", collect_err[0]);
}


void test_alignment_invalid_vX(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-a", "vX");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: Illegal text format -- vX\n", collect_err[0]);
}


void test_alignment_invalid_jX(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-a", "jX");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: Illegal text format -- jX\n", collect_err[0]);
}


void test_alignment_notset(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-a", "");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: Illegal text format -- \n", collect_err[0]);
}


void test_alignment_incomplete(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-a", "v");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: Illegal text format -- v\n", collect_err[0]);
}


void test_inputfiles_illegal_third_file(void **state)
{
    UNUSED(state);

    opt_t *actual = act(3, "file1", "file2", "file3_ILLEGAL");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(3, collect_err_size);
    assert_string_equal("boxes: illegal parameter -- file3_ILLEGAL\n", collect_err[0]);
    assert_string_equal("Usage: boxes [options] [infile [outfile]]\n", collect_err[1]);
    assert_string_equal("Try `boxes -h' for more information.\n", collect_err[2]);
}


void test_inputfiles_stdin_stdout(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-", "-");

    assert_non_null(actual);
    assert_ptr_equal(stdin, actual->infile);
    assert_ptr_equal(stdout, actual->outfile);
}


void test_inputfiles_stdin(void **state)
{
    UNUSED(state);

    opt_t *actual = act(1, "-");

    assert_non_null(actual);
    assert_ptr_equal(stdin, actual->infile);
    assert_ptr_equal(stdout, actual->outfile);
}


void test_inputfiles_input_nonexistent(void **state)
{
    UNUSED(state);

    opt_t *actual = act(1, "NON-EXISTENT");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: Can\'t open input file -- NON-EXISTENT\n", collect_err[0]);
}


void test_inputfiles_actual_success(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "../utest/dummy_in.txt", "../out/dummy_out.txt");

    assert_non_null(actual);
    assert_non_null(actual->infile);
    assert_non_null(actual->outfile);
}


void test_command_line_design_empty(void **state)
{
    UNUSED(state);

    opt_t *actual = act(2, "-c", "");

    assert_null(actual);   // invalid option, so we would need to exit with error
    assert_int_equal(1, collect_err_size);
    assert_string_equal("boxes: empty command line design definition\n", collect_err[0]);
}


void test_help(void **state)
{
    UNUSED(state);

    opt_t *actual = act(1, "-h");

    assert_non_null(actual);
    assert_int_equal(1, actual->help);
    assert_int_equal(0, actual->version_requested);
}


void test_version_requested(void **state)
{
    UNUSED(state);

    opt_t *actual = act(1, "-v");

    assert_non_null(actual);
    assert_int_equal(0, actual->help);
    assert_int_equal(1, actual->version_requested);
}


/* vim: set cindent sw=4: */
