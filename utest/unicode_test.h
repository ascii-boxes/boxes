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
 * Unit tests of the 'unicode' module
 */

#ifndef UNICODE_TEST_H
#define UNICODE_TEST_H


void test_to_utf32(void **state);
void test_is_blank(void **state);
void test_is_allowed_in_sample(void **state);
void test_is_allowed_in_shape(void **state);
void test_is_allowed_in_filename(void **state);
void test_is_allowed_in_kv_string(void **state);
void test_u32_strnrstr(void **state);
void test_u32_insert_space_at(void **state);


#endif

/* vim: set cindent sw=4: */