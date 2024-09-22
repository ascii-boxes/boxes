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

#ifndef UTEST_TOOLS_H
#define UTEST_TOOLS_H


/**
 * Assert that the two given arrays have the same contents.
 * @param <p_expected> the expected array
 * @param <p_actual> the actual array
 * @param <p_len_expected> number of elements in `p_expected`
 */
void assert_array_equal(int p_expected[], size_t *p_actual, size_t p_len_expected);


/**
 * Print the contents of the given array for debugging purposes.
 * @param p_array the array to print
 * @param p_len the number of elements in the array
 */
void print_array_i(int p_array[], size_t p_len);

void print_array_s(size_t p_array[], size_t p_len);

#endif

/* vim: set cindent sw=4: */
