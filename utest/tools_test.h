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
 * Unit tests of the 'tools' module.
 */

#ifndef TOOLS_TEST_H
#define TOOLS_TEST_H


void test_strisyes_true(void **state);
void test_strisyes_false(void **state);

void test_strisno_true(void **state);
void test_strisno_false(void **state);

void test_my_strrspn_edge(void **state);
void test_my_strrspn(void **state);

void test_is_csi_reset(void **state);

void test_is_ascii_id_valid(void **state);
void test_is_ascii_id_invalid(void **state);
void test_is_ascii_id_strict_valid(void **state);
void test_is_ascii_id_strict_invalid(void **state);

void test_repeat(void **state);


#endif

/* vim: set cindent sw=4: */
