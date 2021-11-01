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
 * Main module for running the unit tests.
 */

#include "config.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "global_mock.h"
#include "tools_test.h"
#include "regulex_test.h"


static int beforeTest(void** state) {
    (void) state;  /* unused */

    collect_reset();
    return 0;
}



int main(void)
{
    setup_mocks();

    const struct CMUnitTest tools_tests[] = {
        cmocka_unit_test(test_strisyes_true),
        cmocka_unit_test(test_strisyes_false),
        cmocka_unit_test(test_strisno_true),
        cmocka_unit_test(test_strisno_false)
    };

    const struct CMUnitTest regulex_tests[] = {
        cmocka_unit_test_setup(test_compile_pattern_error, beforeTest),
        cmocka_unit_test_setup(test_compile_pattern_empty, beforeTest),
        cmocka_unit_test_setup(test_regex_replace_invalid_utf, beforeTest),
        cmocka_unit_test_setup(test_regex_replace_buffer_resize, beforeTest),
        cmocka_unit_test_setup(test_regex_replace_error, beforeTest)
    };

    int num_failed = 0;
    num_failed += cmocka_run_group_tests(tools_tests, NULL, NULL);
    num_failed += cmocka_run_group_tests(regulex_tests, NULL, NULL);

    teardown();
    return num_failed;
}


/*EOF*/                                          /* vim: set cindent sw=4: */
