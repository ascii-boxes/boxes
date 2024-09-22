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
 * Shape handling and information functions
 */

#ifndef SHAPE_H
#define SHAPE_H

#include "boxes.h"
#include "bxstring.h"


extern char *shape_name[];

#define SHAPES_PER_SIDE 5
#define CORNERS_PER_SIDE 2

extern shape_t north_side[SHAPES_PER_SIDE];  /* groups of shapes, clockwise */
extern shape_t  east_side[SHAPES_PER_SIDE];
extern shape_t south_side[SHAPES_PER_SIDE];
extern shape_t south_side_rev[SHAPES_PER_SIDE];
extern shape_t  west_side[SHAPES_PER_SIDE];
extern shape_t corners[NUM_CORNERS];
extern shape_t *sides[NUM_SIDES];


int genshape (const size_t width, const size_t height, char ***chars, bxstr_t ***mbcs);
void freeshape (sentry_t *shape);

shape_t findshape (const sentry_t *sarr, const int num);
int on_side (const shape_t s, const int idx);

int isempty (const sentry_t *shape);
int isdeepempty (const sentry_t *shape);

size_t highest (const sentry_t *sarr, const int n, ...);
size_t widest (const sentry_t *sarr, const int n, ...);

int empty_side (sentry_t *sarr, const int aside);


/**
 * Determine if there are only blanks to the left (or right) of this shape on the given line. The result is cached in
 * the shape.
 * @param current_design the design whose shapes to use
 * @param shape the shape for which to calculate the result
 * @param shape_line_idx the index of the shape line to assess
 * @param is_leftward 1 to check on the left, 0 to check on the right
 * @return 1 if blank, 0 otherwise. Box contents is always counted as "not blank" (because it could be), and the outside
 *      of a box counts as blank.
 */
int is_blankward(design_t *current_design, const shape_t shape, const size_t shape_line_idx, const int is_leftward);


/**
 * Print complete data about a shape to stderr for debugging.
 * @param shape the shape whose data to print
 */
void debug_print_shape(sentry_t *shape);



#endif /*SHAPE_H*/

/* vim: set cindent sw=4: */
