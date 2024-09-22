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
 * Mocks of boxes' global variables.
 */

#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <uniconv.h>

#include "boxes.h"
#include "unicode.h"
#include "tools.h"
#include "global_mock.h"



design_t *designs = NULL;

int num_designs = 0;

opt_t opt;

input_t input;

int color_output_enabled = 1;

char **collect_out = NULL;
int collect_out_size = 0;

char **collect_err = NULL;
int collect_err_size = 0;


void collect_reset()
{
    for (int i = 0; i < collect_out_size; i++) {
        BFREE(collect_out[i]);
    }
    BFREE(collect_out);

    for (int i = 0; i < collect_err_size; i++) {
        BFREE(collect_err[i]);
    }
    BFREE(collect_err);

    collect_out_size = 0;
    collect_err_size = 0;
}


/**
 * Mock of the `bx_fprintf()` function which records its output instead of printing it. Assumes that no output string
 * of our test cases will be longer than 512 characters.
 * @param stream `stdout` or `stderr`
 * @param format the format string, followed by the arguments
 */
void __wrap_bx_fprintf(FILE *stream, const char *format, ...)
{
    char **collect = stream == stdout ? collect_out : collect_err;
    int collect_size = stream == stdout ? collect_out_size : collect_err_size;
    collect = (char **) realloc(collect, ++collect_size * sizeof(char *));

    char *s = (char *) malloc(512);
    va_list va;
    va_start(va, format);
    vsprintf(s, format, va);
    va_end(va);
    collect[collect_size - 1] = s;

    if (stream == stdout) {
        collect_out = collect;
        collect_out_size = collect_size;
    }
    else {
        collect_err = collect;
        collect_err_size = collect_size;
    }
}


void setup_mocks()
{
    setlocale(LC_ALL, "");
    encoding = check_encoding("UTF-8", locale_charset());
    collect_reset();
    set_bx_fprintf(__wrap_bx_fprintf);
}


void teardown()
{
    collect_reset();
}


/* vim: set cindent sw=4: */
