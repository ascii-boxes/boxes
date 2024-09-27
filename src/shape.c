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

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "boxes.h"
#include "bxstring.h"
#include "logging.h"
#include "shape.h"
#include "tools.h"



char *shape_name[] = {
        "NW", "NNW", "N", "NNE", "NE", "ENE", "E", "ESE",
        "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW"
};

shape_t north_side[SHAPES_PER_SIDE] = {NW, NNW, N, NNE, NE};  /* clockwise */

shape_t east_side[SHAPES_PER_SIDE] = {NE, ENE, E, ESE, SE};

shape_t south_side[SHAPES_PER_SIDE] = {SE, SSE, S, SSW, SW};
shape_t south_side_rev[SHAPES_PER_SIDE] = {SW, SSW, S, SSE, SE};

shape_t west_side[SHAPES_PER_SIDE] = {SW, WSW, W, WNW, NW};

shape_t corners[NUM_CORNERS] = {NW, NE, SE, SW};

shape_t *sides[] = {north_side, east_side, south_side, west_side};



shape_t findshape(const sentry_t *sarr, const int num)
/*
 *  Find a non-empty shape and return its name
 *
 *      sarr    the shape array to check
 *      num     number of entries in sarr to be checked
 *
 *  RETURNS: a shape_name  on success
 *           num           on error (e.g. empty shape array)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i;

    for (i = 0; i < num; ++i) {
        if (isempty(sarr + i)) {
            continue;
        } else {
            break;
        }
    }

    return (shape_t) i;
}



int on_side(const shape_t s, const int idx)
/*
 *  Compute the side that shape s is on.
 *
 *      s    shape to look for
 *      idx  which occurence to return (0 == first, 1 == second (for corners)
 *
 *  RETURNS: side number (BTOP etc.)  on success
 *           NUM_SIDES                on error (e.g. idx==1 && s no corner)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int side;
    int i;
    int found = 0;

    for (side = 0; side < NUM_SIDES; ++side) {
        for (i = 0; i < SHAPES_PER_SIDE; ++i) {
            if (sides[side][i] == s) {
                if (found == idx) {
                    return side;
                } else {
                    ++found;
                }
            }
        }
    }

    return NUM_SIDES;
}



int genshape(const size_t width, const size_t height, char ***chars, bxstr_t ***mbcs)
/*
 *  Generate a shape consisting of spaces only.
 *
 *      width   desired shape width
 *      height  desired shape height
 *      chars   pointer to the shape lines (should be NULL upon call)
 *      mbcs    pointer to the shape lines, MBCS version (should be NULL upon call)
 *
 *  Memory is allocated for the shape lines which must be freed by the caller.
 *
 *  RETURNS:  == 0   on success (memory allocated)
 *            != 0   on error   (no memory allocated)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t j;

    if (width <= 0 || height <= 0 || width > LINE_MAX_BYTES) {
        fprintf(stderr, "%s: internal error\n", PROJECT);
        return 1;
    }

    *chars = (char **) calloc(height, sizeof(char *));
    if (*chars == NULL) {
        perror(PROJECT);
        return 2;
    }

    *mbcs = (bxstr_t **) calloc(height, sizeof(bxstr_t *));
    if (*mbcs == NULL) {
        BFREE(*chars);
        perror(PROJECT);
        return 4;
    }

    for (j = 0; j < height; ++j) {
        (*chars)[j] = nspaces(width);
        (*mbcs)[j] = bxs_from_ascii((*chars)[j]);
    }

    return 0;
}



void freeshape(sentry_t *shape)
/*
 *  Free all memory allocated by the shape and set the struct to
 *  SENTRY_INITIALIZER. Do not free memory of the struct.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t j;

    for (j = 0; j < shape->height; ++j) {
        BFREE (shape->chars[j]);
        bxs_free(shape->mbcs[j]);
    }
    BFREE (shape->chars);
    BFREE (shape->mbcs);
    BFREE (shape->blank_leftward);
    BFREE (shape->blank_rightward);

    *shape = SENTRY_INITIALIZER;
}



int isempty(const sentry_t *shape)
/*
 *  Return true if shape is empty.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    if (shape == NULL) {
        return 1;
    } else if (shape->chars == NULL || shape->mbcs == NULL) {
        return 1;
    } else if (shape->width == 0 || shape->height == 0) {
        return 1;
    } else {
        return 0;
    }
}



int isdeepempty(const sentry_t *shape)
/*
 *  Return true if shape is empty, also checking if lines consist of whitespace
 *  only.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t j;

    if (isempty(shape)) {
        return 1;
    }

    for (j = 0; j < shape->height; ++j) {
        if (shape->chars[j]) {
            if (strspn(shape->chars[j], " \t") != shape->width) {
                return 0;
            }
        }
    }

    return 1;
}



size_t highest(const sentry_t *sarr, const int n, ...)
/*
 *  Return height (vert.) of highest shape in given list.
 *
 *  sarr         array of shapes to examine
 *  n            number of shapes following
 *  ...          the shapes to consider
 *
 *  RETURNS:     height in lines (may be zero)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    va_list ap;
    int i;
    size_t max = 0;                      /* current maximum height */

    va_start (ap, n);

    for (i = 0; i < n; ++i) {
        shape_t r = va_arg (ap, shape_t);
        if (!isempty(sarr + r)) {
            if (sarr[r].height > max) {
                max = sarr[r].height;
            }
        }
    }

    va_end (ap);

    return max;
}



size_t widest(const sentry_t *sarr, const int n, ...)
/*
 *  Return width (horiz.) of widest shape in given list.
 *
 *  sarr         array of shapes to examine
 *  n            number of shapes following
 *  ...          the shapes to consider
 *
 *  RETURNS:     width in chars (may be zero)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    va_list ap;
    int i;
    size_t max = 0;                      /* current maximum width */

    va_start (ap, n);

    for (i = 0; i < n; ++i) {
        shape_t r = va_arg (ap, shape_t);
        if (!isempty(sarr + r)) {
            if (sarr[r].width > max) {
                max = sarr[r].width;
            }
        }
    }

    va_end (ap);

    return max;
}



/**
 * Return true if the shapes on the given side consist entirely of spaces - and spaces only, tabs are considered
 * non-empty.
 *
 * @param sarr pointer to shape list of design to check
 * @param aside the box side (one of `BTOP` etc.)
 * @return == 0: side is not empty;
 *         \!= 0: side is empty
 */
int empty_side(sentry_t *sarr, const int aside)
{
    int i;

    for (i = 0; i < SHAPES_PER_SIDE; ++i) {
        if (isdeepempty(sarr + sides[aside][i])) {
            continue;
        } else {
            return 0;
        }                    /* side is not empty */
    }

    return 1;                            /* side is empty */
}



static int is_west(sentry_t *shape, int include_corners)
{
    size_t offset = include_corners ? 0 : 1;
    for (size_t i = offset; i < SHAPES_PER_SIDE - offset; i++) {
        if (west_side[i] == shape->name) {
            return 1;
        }
    }
    return 0;
}



static int is_east(sentry_t *shape, int include_corners)
{
    size_t offset = include_corners ? 0 : 1;
    for (size_t i = offset; i < SHAPES_PER_SIDE - offset; i++) {
        if (east_side[i] == shape->name) {
            return 1;
        }
    }
    return 0;
}



static int find_in_horiz(shape_t side[], shape_t shape_name)
{
    int result = -1;
    for (size_t i = 0; i < SHAPES_PER_SIDE; i++) {
        if (side[i] == shape_name) {
            result = i;
            break;
        }
    }
    return result;
}



static int find_in_north(shape_t shape_name)
{
    return find_in_horiz(north_side, shape_name);
}



static int find_in_south(shape_t shape_name)
{
    return find_in_horiz(south_side_rev, shape_name);
}



static int *new_blankward_cache(const size_t shape_height)
{
    int *result = (int *) calloc(shape_height, sizeof(int));
    for (size_t i = 0; i < shape_height; i++) {
        result[i] = -1;
    }
    return result;
}



static int is_blankward_calc(
        design_t *current_design, const shape_t shape, const size_t shape_line_idx, const int is_leftward)
{
    int result = -1;
    sentry_t *shape_data = current_design->shape + shape;
    if (is_west(shape_data, is_leftward ? 1 : 0)) {
        result = is_leftward ? 1 : 0;
    }
    else if (is_east(shape_data, is_leftward ? 0 : 1)) {
        result = is_leftward ? 0 : 1;
    }
    else {
        shape_t *side = north_side;
        int pos = find_in_north(shape);
        if (pos < 0) {
            side = south_side_rev;
            pos = find_in_south(shape);
        }
        result = 1;
        size_t loop_init = (size_t) pos + 1;
        size_t loop_end = SHAPES_PER_SIDE;
        if (is_leftward) {
            loop_init = 0;
            loop_end = (size_t) pos;
        }
        for (size_t i = loop_init; i < loop_end; i++) {
            sentry_t *tshape = current_design->shape + side[i];
            if (tshape->mbcs != NULL && !bxs_is_blank(tshape->mbcs[shape_line_idx])) {
                result = 0;
                break;
            }
        }
    }
    return result;
}



int is_blankward(design_t *current_design, const shape_t shape, const size_t shape_line_idx, const int is_leftward)
{
    if (current_design == NULL) {
        return 0;  /* this would be a bug */
    }
    sentry_t *shape_data = current_design->shape + shape;
    if (shape_line_idx >= shape_data->height) {
        return 0;  /* this would be a bug */
    }
    int *blankward_cache = is_leftward ? shape_data->blank_leftward : shape_data->blank_rightward;
    if (blankward_cache != NULL && blankward_cache[shape_line_idx] >= 0) {
        return blankward_cache[shape_line_idx];  /* cached value available */
    }
    if (blankward_cache == NULL) {
        blankward_cache = new_blankward_cache(shape_data->height);
        if (is_leftward) {
            shape_data->blank_leftward = blankward_cache;
        }
        else {
            shape_data->blank_rightward = blankward_cache;
        }
    }

    int result = is_blankward_calc(current_design, shape, shape_line_idx, is_leftward);
    blankward_cache[shape_line_idx] = result;
    return result;
}



void debug_print_shape(sentry_t *shape)
{
    if (is_debug_logging(MAIN)) {
        if (shape == NULL) {
            log_debug(__FILE__, MAIN, "NULL\n");
            return;
        }
        log_debug(__FILE__, MAIN, "Shape %3s (%dx%d): elastic=%s,%s bl=",
            shape_name[shape->name], (int) shape->width, (int) shape->height,
            shape->elastic ? "true" : "false", shape->elastic ? " " : "");
        if (shape->blank_leftward == NULL) {
            log_debug_cont(MAIN, "NULL");
        }
        else {
            log_debug_cont(MAIN, "[");
            for (size_t i = 0; i < shape->height; i++) {
                log_debug_cont(MAIN, "%d%s", shape->blank_leftward[i],
                        shape->height > 0 && i < (shape->height - 1) ? ", " : "");
            }
            log_debug_cont(MAIN, "]");
        }
        log_debug_cont(MAIN, ", br=");
        if (shape->blank_rightward == NULL) {
            log_debug_cont(MAIN, "NULL");
        }
        else {
            log_debug_cont(MAIN, "[");
            for (size_t i = 0; i < shape->height; i++) {
                log_debug_cont(MAIN, "%d%s", shape->blank_rightward[i],
                        shape->height > 0 && i < (shape->height - 1) ? ", " : "");
            }
            log_debug_cont(MAIN, "]");
        }
        log_debug_cont(MAIN, ", ascii=");
        if (shape->chars == NULL) {
            log_debug_cont(MAIN, "NULL");
        }
        else {
            log_debug_cont(MAIN, "[");
            for (size_t i = 0; i < shape->height; i++) {
                log_debug_cont(MAIN, "%s%s%s%s", shape->chars[i] != NULL ? "\"" : "", shape->chars[i],
                        shape->chars[i] != NULL ? "\"" : "", (int) i < ((int) shape->height) - 1 ? ", " : "");
            }
            log_debug_cont(MAIN, "]");
        }
        log_debug_cont(MAIN, ", mbcs=");
        if (shape->mbcs == NULL) {
            log_debug_cont(MAIN, "NULL");
        }
        else {
            log_debug_cont(MAIN, "[");
            for (size_t i = 0; i < shape->height; i++) {
                char *out_mbcs = bxs_to_output(shape->mbcs[i]);
                log_debug_cont(MAIN, "%s%s%s%s", shape->mbcs[i] != NULL ? "\"" : "", out_mbcs,
                        shape->mbcs[i] != NULL ? "\"" : "", shape->height > 0 && i < (shape->height - 1) ? ", " : "");
                BFREE(out_mbcs);
            }
            log_debug_cont(MAIN, "]");
        }
        log_debug_cont(MAIN, "\n");
    }
}


/* vim: set sw=4: */
