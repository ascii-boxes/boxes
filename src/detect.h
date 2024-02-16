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
 * Autodetect design used by box in input.
 */

#ifndef BOXES_DETECT_H
#define BOXES_DETECT_H

#include "boxes.h"



#define NUM_COMPARISON_TYPES 4  /* number of elements in `comparison_t` */

typedef enum {
    /** leave invisible characters in both shapes and input */
    literal,
    
    /** leave invisible characters in shapes, but ignore them in input */
    ignore_invisible_input,
    
    /** ignore invisible characters in shapes, but leave them in input */
    ignore_invisible_shape,
    
    /** ignore all invisible characters in both shapes and input */
    ignore_invisible_all
} comparison_t;

/** The elements of `comparision_t` as strings for printing */
extern char *comparison_name[];


/**
 * Determine if the given comparison type makes sense given the presence of invisible characters in the input or the
 * design.
 * @param comp_type comparison type
 * @param mono_input flag indicating that there are no invisible characters in the input
 * @param mono_design flag indicating that there are no invisible characters in the definition of the design
 * @return flag indicating that the comparison type should be used
 */
int comp_type_is_viable(comparison_t comp_type, int mono_input, int mono_design);


/**
 * Determine whether the input text contains ANSI escape codes (i.e. it is potentially colored) or not.
 * @return 1 if no invisible characters are in the input, 0 if there are any
 */
int input_is_mono();


/**
 * Determine whether the given box design contains ANSI escape codes in any of its shapes (i.e. it is potentially
 * colored) or not.
 * @param current_design the box design to check
 * @return 1 if no invisible characters are found in the box design, 0 if there are any
 */
int design_is_mono(design_t *design);


/**
 * Prepare one line of a shape for comparison with a part of an input line.
 * @param design the box design we are removing
 * @param shape the shape from which to take the resulting line
 * @param shape_line_idx in a multi-line shape, which line to use
 * @param comp_type the comparison type, which can lead to invisible characters being filtered out
 * @param trim_left flag indicating whether leading whitespace should be trimmed in the result
 * @param trim_right flag indicating whether trailing whitespace should be trimmed in the result
 * @return the relevant part of the selected shape line, in new memory
 */
uint32_t *prepare_comp_shape(
        design_t *design, shape_t shape, size_t shape_line_idx, comparison_t comp_type, int trim_left, int trim_right);


/**
 * Prepare one line of input for comparison with a part of a shape.
 * @param input_line_idx the line number, zero-based, as an index into `input.lines`
 * @param trim_left flag indicating whether leading whitespace should be trimmed in the result, ignored when
 *          `offset_right` > 0
 * @param comp_type the comparison type, which can lead to invisible characters being filtered out
 * @param offset_right `> 0`: the result pointer will point to the last `offset_right` non-blank characters (visible
 *          or invisible) of the string; `== 0`: result pointer will point to start of string (possibly trimmed as
 *          per `trim_left`)
 * @param out_indent pointer to a memory location where it is stored how many characters at the start of the result
 *          string are considered part of the indent, for example leading spaces, including any invisible characters.
 *          If NULL, it is ignored.
 * @param out_trailing pointer to a memory location where it is stored how many characters at the end of the result
 *          string are considered trailing, for example trailing spaces, including any invisible characters. If NULL,
 *          it is ignored.
 * @return the relevant part of the selected input line, in original memory (do not free this), or
 *          NULL if `input_line_idx` out of bounds or `offset_right` too large
 */
uint32_t *prepare_comp_input(size_t input_line_idx, int trim_left, comparison_t comp_type, size_t offset_right,
        size_t *out_indent, size_t *out_trailing);


/**
 * Autodetect design used by box in input.
 * This requires knowledge about ALL designs, so the entire config file had to be parsed at some earlier time.
 * @return != NULL: success, pointer to detected design;
 *         == NULL: on error
 */
design_t *autodetect_design();


#endif /* BOXES_DETECT_H */


/* vim: set cindent sw=4: */
