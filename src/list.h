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
 * List design information about a single design or about all designs.
 */

#ifndef LIST_H
#define LIST_H


/**
 * Create a sorted shallow copy of the global design array.
 * @returns a new array, for which memory was allocated. All pointers in the array point to the original data.
 *      Returns `NULL` when out of memory.
 */
design_t **sort_designs_by_name();


/**
 * Print a sorted listing of available box designs.
 * Uses design name from BOX spec and sample picture plus author.
 * @returns != 0   on error (out of memory);
 *          == 0   on success
 */
int list_designs();


#endif

/*EOF*/                                          /* vim: set cindent sw=4: */
