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
 * Processing of command line options.
 */

#ifndef CMDLINE_H
#define CMDLINE_H


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

/*EOF*/                                          /* vim: set cindent sw=4: */
