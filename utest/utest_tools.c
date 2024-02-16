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
 * Helper functions for all the unit tests.
 */

#include "config.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <stdio.h>

#include "utest_tools.h"



void assert_array_equal(int p_expected[], size_t *p_actual, size_t p_len_expected)
{
    if (p_expected == NULL && p_actual == NULL) {
        assert_int_equal(0, p_len_expected);
        return;
    }
    if (p_expected == NULL) {
        assert_null(p_actual);
        assert_int_equal(0, p_len_expected);
    }
    if (p_actual == NULL) {
        assert_null(p_expected);
        assert_int_equal(0, p_len_expected);
    }

    for (size_t i = 0; i < p_len_expected; i++) {
        assert_int_equal(p_expected[i], (int) p_actual[i]);
    }
}



void print_array_i(int p_array[], size_t p_len)
{
    if (p_array != NULL) {
        printf("[");
        for (size_t i = 0; i < p_len; i++) {
            printf("%d%s", p_array[i], i < p_len - 1 ? ", " : "");
        }
        printf("]\n");
    }
    else {
        printf("NULL\n");
    }
}



void print_array_s(size_t p_array[], size_t p_len)
{
    if (p_array != NULL) {
        printf("[");
        for (size_t i = 0; i < p_len; i++) {
            printf("%d%s", (int) p_array[i], i < p_len - 1 ? ", " : "");
        }
        printf("]\n");
    }
    else {
        printf("NULL\n");
    }
}


/* vim: set cindent sw=4: */
