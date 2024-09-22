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
 * Functions related to querying the design list by tag.
 */

#ifndef QUERY_H
#define QUERY_H


/**
 * Parse the tag query specified with the `-q` command line option.
 * @param optarg the argument to `-q` on the command line
 * @returns the parsed query which can be assigned to the global `opt` struct later, or `NULL` on error
 */
char **parse_query(char *optarg);


/**
 * Perform the tag query based on the global design list and the query from the global `opt` struct.
 * @returns 0 if successful; anything else on error (then the program should exit)
 */
int query_by_tag();


#endif

/* vim: set cindent sw=4: */
