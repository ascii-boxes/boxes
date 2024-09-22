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
 * Processing of command line options.
 */

#ifndef CMDLINE_H
#define CMDLINE_H

#include <stdio.h>

#include "boxes.h"



/*
 * Some ints that define return values for getopt_long() to indicate certain long options.
 */
#define OPT_COLOR 1001
#define OPT_NO_COLOR 1002
#define OPT_KILLBLANK 1003
#define OPT_NO_KILLBLANK 1004


/**
 * Print usage information on stream `st`, including a header text.
 * @param st the stream to print to
 */
void usage_long(FILE *st);


/**
 * Process command line options. Does not set the global `opt`. This function simply returns a value.
 * @param argc number of command line options as passed to `main()`
 * @param argv contents of command line options as passed to `main()`
 * @returns a pointer to a filled-in option struct, for which memory was allocated; `NULL` on error
 */
opt_t *process_commandline(int argc, char *argv[]);


#endif

/* vim: set cindent sw=4: */
