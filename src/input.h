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
 * Read and analyze input text.
 */

#ifndef INPUT_H
#define INPUT_H

#include "boxes.h"


/**
 * Read the entire input from `opt.infile` until EOF is encountered. Tabs are expanded.
 * @return a pointer to the read input data, for which new memory was allocated, or `NULL` on error
 */
input_t *read_all_input();


/**
 * Analyze and prepare the input text for further processing. Compute statistics, remove indentation, and apply
 * regular expressions if specified in the design.
 * @param input_data the input data to analyze and modify
 * @returns == 0 on success; anything else on error
 */
int analyze_input(input_t *input_data);


/**
 * Apply regular expression substitutions to input text. Attn: This modifies the global input array!
 * @param input_data pointer to the input data where substitutions should be applied
 * @param mode when 0: use replacement rules (box is being *drawn*);
 *             when 1: use reversion rules (box is being *removed*)
 * @returns == 0 on success; anything else on error
 */
int apply_substitutions(input_t *input_data, const int mode);


#endif

/*EOF*/                                          /* vim: set cindent sw=4: */
