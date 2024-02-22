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
 * Unit tests of the 'logging' module
 */

#ifndef LOGGING_TEST_H
#define LOGGING_TEST_H

#include "logging.h"

int logging_setup(void **state);
int logging_teardown(void **state);

void test_debug_shorten_nopath(void **state);
void test_debug_shorten_minimal(void **state);
void test_debug_shorten_null(void **state);
void test_debug_shorten_backslash(void **state);
void test_debug_shorten_slash(void **state);
void test_debug_shorten_nosuffix(void **state);
void test_debug_shorten_wrongdot(void **state);
void test_debug_shorten_empty(void **state);
void test_debug_inactive_area(void **state);
void test_debug_continue(void **state);
void test_debug_continue_inactive(void **state);
void test_debug_area_too_big(void **state);
void test_debug_area_reserved(void **state);
void test_debug_area_all(void **state);
void test_debug_deactivated(void **state);
void test_debug_all_active(void **state);


#endif


/* vim: set cindent sw=4: */
