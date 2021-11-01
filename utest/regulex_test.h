/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2021 Thomas Jensen and the boxes contributors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/*
 * Unit tests of the 'regulex' module
 */

#ifndef REGULEX_TEST_H
#define REGULEX_TEST_H


void test_compile_pattern_empty(void **state);
void test_compile_pattern_error(void **state);

void test_regex_replace_invalid_utf(void **state);
void test_regex_replace_buffer_resize(void **state);
void test_regex_replace_error(void **state);


#endif


/*EOF*/                                          /* vim: set cindent sw=4: */
