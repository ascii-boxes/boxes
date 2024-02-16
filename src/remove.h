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
 * Box removal, i.e. the deletion of boxes
 */

#ifndef REMOVE_H
#define REMOVE_H


/** the result of a match operation at the beginning or end of a line */
typedef struct _match_result_t {
    /** pointer to the matched position */
    uint32_t *p;

    /** index of the matched position in the haystack string */
    size_t p_idx;

    /** number of characters matched (between 1 and shape_line->num_chars) */
    size_t len;

    /** flag indicating the match was secured by including non-blank characters (0) or only blanks were matched (1) */
    int shiftable;
} match_result_t;



typedef struct _shape_line_ctx_t {
    /** flag indicating whether the (entire!) shape is empty */
    int empty;

    /** one line of a shape, with invisible characters filtered according to the comparison type,
     *  NULL when shape is empty */
    bxstr_t *text;

    /** flag indicating whether the shape to which this line belongs is elastic */
    int elastic;
} shape_line_ctx_t;



/**
 * Remove box from input.
 * @return == 0: success;
 *         \!= 0:  error
 */
int remove_box();


/**
 * Output contents of input line list "as is" to standard output, except for removal of trailing spaces (trimming).
 * The trimming is performed on the actual input lines, modifying them.
 * @param trim_only only perform the trimming of trailing spaces, but do not output anything
 */
void output_input(const int trim_only);


#endif /*REMOVE_H*/

/* vim: set cindent sw=4: */
