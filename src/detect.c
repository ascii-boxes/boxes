/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2023 Thomas Jensen and the boxes contributors
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
 *
 * When detecting box shapes in input lines, we can find (in order of precedence):
 * 1. (box design: colored, input: colored)
 *    Colored boxes which are colored as per their design. In that case, matching all the invisible characters
 *    increases our confidence that we have found the right shape.
 *    -> use both shapes and input as-is, comparison type `literal`
 * 2. (box design: monochrome, input: monochrome)
 *    Boxes which have no invisible characters because color was never involved. This one is the classic case.
 *    -> use both shapes and input as-is, comparison type `literal` (same as 1.)
 * 3. (box design: monochrome, input: colored)
 *    Colored boxes which are colored because of lolcat processing or similar. In that case, we can ignore the
 *    invisible characters in the input.
 *    -> use shapes as-is, and ignore invisible characters in input, comparison type `ignore_invisible_input`
 * 4. (box design: colored, input: monochrome)
 *    Boxes which have no invisible characters because they have been removed (for example via --no-color), even though
 *    the original design was colored. In that case, we must ignore the invisible characters in the design.
 *    -> ignore invisible characters in shapes, use input as-is, comparison type `ignore_invisible_shape`
 * 5. Fallback: We assume to never see a colored box design PLUS lolcat-induced codes, or a case where a colored box
 *    design had its color removed and replaced with lolcat colors. That's just messy and we will treat it as case
 *    number two, where we ignore colors on both input and box design.
 *    -> ignore invisible characters in both shapes and input, comparison type `ignore_invisible_all`
 */

#include "config.h"

#include <unistr.h>
#include <unitypes.h>

#include "boxes.h"
#include "bxstring.h"
#include "shape.h"
#include "tools.h"
#include "detect.h"



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

static char *comparison_name[] = {
        "literal", "ignore_invisible_input", "ignore_invisible_shape", "ignore_invisible_all"
};



/**
 * Determine whether the input text contains ANSI escape codes (i.e. it is potentially colored) or not.
 * @return 1 if no invisible characters are in the input, 0 if there are any
 */
static int input_is_mono()
{
    for (size_t line_no = 0; line_no < input.num_lines; line_no++) {
        if (input.lines[line_no].text->num_chars_invisible > 0) {
            return 0;
        }
    }
    return 1;
}



/**
 * Determine whether the given box design contains ANSI escape codes in any of its shapes (i.e. it is potentially
 * colored) or not.
 * @param current_design the box design to check
 * @return 1 if no invisible characters are found in the box design, 0 if there are any
 */
static int design_is_mono(design_t *current_design)
{
    for (shape_t scnt = 0; scnt < NUM_SHAPES; ++scnt) {
        if (isempty(current_design->shape + scnt)) {
            continue;
        }
        for (size_t line_no = 0; line_no < input.num_lines; line_no++) {
            bxstr_t *shape_line = current_design->shape[scnt].mbcs[line_no];
            if (shape_line->num_chars_invisible > 0) {
                return 0;
            }
        }
    }
    return 1;
}



static int *determine_empty_sides(design_t *current_design)
{
    int *result = (int *) calloc(NUM_SIDES, sizeof(int));

    for (size_t j = 0; j < NUM_SIDES; ++j) {
        result[j] = empty_side(current_design->shape, j);
    }
    #ifdef DEBUG
        fprintf (stderr, "Empty sides: TOP %d, LEFT %d, BOTTOM %d, RIGHT %d\n",
            result[BTOP], result[BLEF], result[BBOT], result[BRIG]);
    #endif
    return result;
}



static uint32_t *get_visible_text(line_t *line)
{
    uint32_t *result = NULL;
    if (line != NULL) {
        if (line->cache_visible == NULL) {
            line->cache_visible = bxs_filter_visible(line->text);
        }
        result = line->cache_visible;
    }
    return result;
}



/**
 * Try and find west corner shapes. Every non-empty shape line is searched for on every input line. A hit is generated
 * whenever a match is found.
 * @param current_design the current design to check
 * @param comp_type the comparison type (how to compare colored strings)
 * @param empty information on which box sides are empty in that design
 * @param corner which west corner to search for
 * @return the number of hits for this corner
 */
static size_t find_west_corner(design_t *current_design, comparison_t comp_type, int *empty, shape_t corner)
{
    size_t hits = 0;
    if (empty[BLEF] || (empty[BTOP] && corner == NW) || (empty[BBOT] && corner == SW)) {
        return hits;
    }

    for (size_t j = 0; j < current_design->shape[corner].height; ++j) {
        bxstr_t *shape_line = current_design->shape[corner].mbcs[j];
        if (bxs_is_blank(shape_line)) {
            continue;
        }

        uint32_t *shape_relevant_for_freeing = NULL;
        uint32_t *shape_relevant = NULL;
        size_t length_relevant;
        if ((comp_type == ignore_invisible_shape || comp_type == ignore_invisible_all)
                && shape_line->num_chars_invisible > 0)
        {
            shape_relevant_for_freeing = bxs_filter_visible(shape_line);
            shape_relevant = shape_relevant_for_freeing + shape_line->indent;
            length_relevant = shape_line->num_chars_visible - shape_line->indent;
        }
        else {
            shape_relevant = bxs_unindent_ptr(shape_line);
            length_relevant = shape_line->num_chars - (shape_relevant - shape_line->memory);
        }

        for (size_t k = 0; k < current_design->shape[corner].height; ++k) {
            size_t a = k;
            if (corner == SW) {
                a += input.num_lines - current_design->shape[corner].height;
            }
            if (a >= input.num_lines) {
                break;
            }

            uint32_t *input_relevant = NULL;
            if (comp_type == ignore_invisible_input || comp_type == ignore_invisible_all) {
                input_relevant = get_visible_text(input.lines + a) + input.lines[a].text->indent;
            }
            else {
                input_relevant = bxs_unindent_ptr(input.lines[a].text);
            }

            if (u32_strncmp(input_relevant, shape_relevant, length_relevant) == 0) {
                ++hits; /* CHECK more hit points for longer matches, or simple boxes might match too easily */
            }
        }
        BFREE(shape_relevant_for_freeing);
    }

    #ifdef DEBUG
        fprintf(stderr, "Checking %s corner produced %d hits.\n", shape_name[corner], (int) hits);
    #endif
    return hits;
}



/**
 * Try and find east corner shapes. Every non-empty shape line is searched for on every input line. A hit is generated
 * whenever a match is found.
 * @param current_design the current design to check
 * @param comp_type the comparison type (how to compare colored strings)
 * @param empty information on which box sides are empty in that design
 * @param corner which west corner to search for
 * @return the number of hits for this corner
 */
static size_t find_east_corner(design_t *current_design, comparison_t comp_type, int *empty, shape_t corner)
{
    size_t hits = 0;
    if (empty[BRIG] || (empty[BTOP] && corner == NE) || (empty[BBOT] && corner == SE)) {
        return hits;
    }

    for (size_t j = 0; j < current_design->shape[corner].height; ++j) {
        bxstr_t *shape_line = current_design->shape[corner].mbcs[j];
        if (bxs_is_blank(shape_line)) {
            continue;
        }

        bxstr_t *shape_line_rtrimmed = bxs_rtrim(shape_line);
        uint32_t *shape_relevant_for_freeing = NULL;
        uint32_t *shape_relevant = NULL;
        size_t length_relevant;
        if ((comp_type == ignore_invisible_shape || comp_type == ignore_invisible_all)
                && shape_line_rtrimmed->num_chars_invisible > 0)
        {
            shape_relevant_for_freeing = bxs_filter_visible(shape_line_rtrimmed);
            shape_relevant = shape_relevant_for_freeing;
            length_relevant = shape_line_rtrimmed->num_chars_visible;
        }
        else {
            shape_relevant = shape_line_rtrimmed->memory;
            length_relevant = shape_line_rtrimmed->num_chars;
        }

        for (size_t k = 0; k < current_design->shape[corner].height; ++k) {
            size_t a = k;
            if (corner == SE) {
                a += input.num_lines - current_design->shape[corner].height;
            }
            if (a >= input.num_lines) {
                break;
            }
            bxstr_t *input_line = input.lines[a].text;

            uint32_t *input_relevant = NULL;
            if (comp_type == ignore_invisible_input || comp_type == ignore_invisible_all) {
                input_relevant = get_visible_text(input.lines + a);
                uint32_t *p = input_relevant + input_line->num_chars_visible - input_line->trailing - length_relevant;
                if (p < input_relevant) {
                    continue;
                }
                input_relevant = p;
            }
            else {
                int idx = (int) input_line->first_char[input_line->num_chars_visible - input_line->trailing]
                          - length_relevant;
                if (idx < 0) {
                    continue;
                }
                input_relevant = input_line->memory + idx;
            }

            if (u32_strncmp(input_relevant, shape_relevant, length_relevant) == 0) {
                ++hits; /* CHECK more hit points for longer matches, or simple boxes might match too easily */
            }
        }
        BFREE(shape_relevant_for_freeing);
        bxs_free(shape_line_rtrimmed);
    }

    #ifdef DEBUG
        fprintf(stderr, "Checking %s corner produced %d hits.\n", shape_name[corner], (int) hits);
    #endif
    return hits;
}



/**
 * Try and find a horizontal shape between the box corners. Every non-empty shape line is searched for on every input
 * line. Elastic shapes must occur twice in an uninterrupted row to generate a hit.
 * @param current_design the current design to check
 * @param comp_type the comparison type (how to compare colored strings)
 * @param empty information on which box sides are empty in that design
 * @param hshape which horizontal shape to search for
 * @return the number of hits for this horizontal shape
 */
static size_t find_horizontal_shape(design_t *current_design, comparison_t comp_type, int *empty, shape_t hshape)
{
    size_t hits = 0;
    if (empty[BTOP] || empty[BBOT]) {
        return ++hits;   /* horizontal box part is empty */
    }

    for (size_t j = 0; j < current_design->shape[hshape].height; ++j) {
        bxstr_t *shape_line = current_design->shape[hshape].mbcs[j];
        if (bxs_is_blank(shape_line)) {
            continue;
        }

        uint32_t *shape_relevant_for_freeing = NULL;
        uint32_t *shape_relevant = NULL;
        size_t length_relevant;
        if ((comp_type == ignore_invisible_shape || comp_type == ignore_invisible_all)
                && shape_line->num_chars_invisible > 0)
        {
            shape_relevant_for_freeing = bxs_filter_visible(shape_line);
            shape_relevant = shape_relevant_for_freeing;
            length_relevant = shape_line->num_chars_visible;
        }
        else {
            shape_relevant = shape_line->memory;
            length_relevant = shape_line->num_chars;
        }

        for (size_t k = 0; k < current_design->shape[hshape].height; ++k) {
            size_t a = k;
            if (hshape >= SSE && hshape <= SSW) {
                a += input.num_lines - current_design->shape[hshape].height;
            }
            if (a >= input.num_lines) {
                break;
            }

            uint32_t *input_relevant = NULL; /* CHECK this eats blank NW corners, too */
            if (comp_type == ignore_invisible_input || comp_type == ignore_invisible_all) {
                input_relevant = get_visible_text(input.lines + a) + input.lines[a].text->indent;
            }
            else {
                input_relevant = bxs_unindent_ptr(input.lines[a].text);
            }

            uint32_t *p = u32_strstr(input_relevant, shape_relevant);
            if (p) {
                if (current_design->shape[hshape].elastic) {
                    p += length_relevant;
                    if (p - input_relevant >= (long) u32_strlen(input_relevant)) {
                        continue;
                    }
                    if (u32_strncmp(p, shape_relevant, length_relevant) == 0) {
                        ++hits;
                    }
                }
                else {
                    ++hits;
                }
            }
        }
        BFREE(shape_relevant_for_freeing);
    }

    #ifdef DEBUG
        fprintf(stderr, "Checking %s shape produced %d hits.\n", shape_name[hshape], (int) hits);
    #endif
    return hits;
}



/**
 * Iterate over all input lines except for potential top and bottom box parts. Check if west line starts match a
 * non-empty shape line. If so, generate a hit.
 * @param current_design the current design to check
 * @param comp_type the comparison type (how to compare colored strings)
 * @param empty information on which box sides are empty in that design
 * @param vshape which vertical shape to search for
 * @return the number of hits for this vertical shape
 */
static size_t find_vertical_west(design_t *current_design, comparison_t comp_type, int *empty, shape_t vshape)
{
    size_t hits = 0;
    if (((empty[BTOP] ? 0 : current_design->shape[NW].height) + (empty[BBOT] ? 0 : current_design->shape[SW].height))
            >= input.num_lines) {
        return hits;
    }
    if (isempty(current_design->shape + vshape)) {
        return hits;
    }

    for (size_t k = empty[BTOP] ? 0 : current_design->shape[NW].height;
            k < input.num_lines - (empty[BBOT] ? 0 : current_design->shape[SW].height); ++k)
    {
        uint32_t *input_relevant = NULL;
        if (comp_type == ignore_invisible_input || comp_type == ignore_invisible_all) {
            input_relevant = get_visible_text(input.lines + k) + input.lines[k].text->indent;
        }
        else {
            input_relevant = bxs_unindent_ptr(input.lines[k].text);
        }

        for (size_t j = 0; j < current_design->shape[vshape].height; ++j) {
            bxstr_t *shape_line = current_design->shape[vshape].mbcs[j];
            if (bxs_is_blank(shape_line)) {
                continue;
            }

            uint32_t *shape_relevant_for_freeing = NULL;
            uint32_t *shape_relevant = NULL;
            size_t length_relevant;
            if ((comp_type == ignore_invisible_shape || comp_type == ignore_invisible_all)
                    && shape_line->num_chars_invisible > 0)
            {
                shape_relevant_for_freeing = bxs_filter_visible(shape_line);
                shape_relevant = shape_relevant_for_freeing + shape_line->indent;
                length_relevant = shape_line->num_chars_visible - shape_line->indent;
            }
            else {
                shape_relevant = bxs_unindent_ptr(shape_line);
                length_relevant = shape_line->num_chars - (shape_relevant - shape_line->memory);
            }

            if (u32_strncmp(input_relevant, shape_relevant, length_relevant) == 0) {
                ++hits;
                break;
            }
            BFREE(shape_relevant_for_freeing);
        }
    }

    #ifdef DEBUG
        fprintf(stderr, "Checking %s shape produced %d hits.\n", shape_name[vshape], (int) hits);
    #endif
    return hits;
}



/**
 * Iterate over all input lines except for potential top and bottom box parts. Check if east line ends match a
 * non-empty shape line. If so, generate a hit.
 * @param current_design the current design to check
 * @param comp_type the comparison type (how to compare colored strings)
 * @param empty information on which box sides are empty in that design
 * @param vshape which vertical shape to search for
 * @return the number of hits for this vertical shape
 */
static size_t find_vertical_east(design_t *current_design, comparison_t comp_type, int *empty, shape_t vshape)
{
    size_t hits = 0;
    if (((empty[BTOP] ? 0 : current_design->shape[NW].height) + (empty[BBOT] ? 0 : current_design->shape[SW].height))
            >= input.num_lines) {
        return hits;
    }
    if (isempty(current_design->shape + vshape)) {
        return hits;
    }

    for (size_t j = 0; j < current_design->shape[vshape].height; ++j) {
        bxstr_t *shape_line = current_design->shape[vshape].mbcs[j];
        if (bxs_is_blank(shape_line)) {
            continue;
        }

        bxstr_t *shape_line_trimmed = bxs_trim(shape_line);
        uint32_t *shape_relevant_for_freeing = NULL;
        uint32_t *shape_relevant = NULL;
        size_t length_relevant;
        if ((comp_type == ignore_invisible_shape || comp_type == ignore_invisible_all)
                && shape_line_trimmed->num_chars_invisible > 0)
        {
            shape_relevant_for_freeing = bxs_filter_visible(shape_line_trimmed);
            shape_relevant = shape_relevant_for_freeing;
            length_relevant = shape_line_trimmed->num_chars_visible;
        }
        else {
            shape_relevant = shape_line_trimmed->memory;
            length_relevant = shape_line_trimmed->num_chars;
        }

        for (size_t k = empty[BTOP] ? 0 : current_design->shape[NW].height;
                k < input.num_lines - (empty[BBOT] ? 0 : current_design->shape[SW].height); ++k)
        {
            bxstr_t *input_line = input.lines[k].text;
            uint32_t *input_relevant = NULL;
            if (comp_type == ignore_invisible_input || comp_type == ignore_invisible_all) {
                input_relevant = get_visible_text(input.lines + k);
                uint32_t *p = input_relevant + input_line->num_chars_visible - input_line->trailing - length_relevant;
                if (p < input_relevant) {
                    continue;
                }
                input_relevant = p;
            }
            else {
                int idx = (int) input_line->first_char[input_line->num_chars_visible - input_line->trailing]
                          - length_relevant;
                if (idx < 0) {
                    continue;
                }
                input_relevant = input_line->memory + idx;
            }

            if (u32_strncmp(input_relevant, shape_relevant, length_relevant) == 0) {
                ++hits;
                break;
            }
        }
        BFREE(shape_relevant_for_freeing);
        bxs_free(shape_line_trimmed);
    }

    #ifdef DEBUG
        fprintf(stderr, "Checking %s shape produced %d hits.\n", shape_name[vshape], (int) hits);
    #endif
    return hits;
}



static long match_design(design_t *current_design, comparison_t comp_type)
{
    int *empty = determine_empty_sides(current_design);
    long hits = 0;

    for (shape_t scnt = 0; scnt < NUM_SHAPES; ++scnt) {
        switch (scnt) {
            case NW:
            case SW:
                hits += find_west_corner(current_design, comp_type, empty, scnt);
                break;

            case NE:
            case SE:
                hits += find_east_corner(current_design, comp_type, empty, scnt);
                break;

            case NNW: case N: case NNE:
            case SSE: case S: case SSW:
                hits += find_horizontal_shape(current_design, comp_type, empty, scnt);
                break;

            case ENE: case E: case ESE:
                hits += find_vertical_east(current_design, comp_type, empty, scnt);
                break;

            case WSW: case W: case WNW:
                hits += find_vertical_west(current_design, comp_type, empty, scnt);
                break;

            default:
                fprintf(stderr, "%s: internal error (scnt=%d)\n", PROJECT, (int) scnt);
                return 0;
        }
    }

    BFREE(empty);
    return hits;
}



design_t *autodetect_design()
{
    design_t *current_design;           /* ptr to currently tested design */
    long maxhits = 0;                   /* maximum no. of hits so far */
    design_t *result = NULL;            /* ptr to design with the most hits */
    int mono_input = input_is_mono();
    (void) comparison_name;             /* used only in debug statements */

    for (comparison_t comp_type = 0; comp_type < NUM_COMPARISON_TYPES; comp_type++) {
        current_design = designs;
        for (size_t dcnt = 0; ((int) dcnt) < num_designs; ++dcnt, ++current_design) {
            int mono_design = design_is_mono(current_design);
            if ((comp_type == literal && mono_input != mono_design)
                || (comp_type == ignore_invisible_input && (mono_input || !mono_design))
                || (comp_type == ignore_invisible_shape && (!mono_input || mono_design))
                || (comp_type == ignore_invisible_all && (mono_input || mono_design)))
            {
                #ifdef DEBUG
                    fprintf(stderr, "Design \"%s\" skipped for comparison type '%s' because mono_input=%d and "
                        "mono_design=%d\n", current_design->name, comparison_name[comp_type], mono_input, mono_design);
                #endif
                continue;
            }

            #ifdef DEBUG
                fprintf(stderr, "CONSIDERING DESIGN ---- \"%s\" ---------------\n", current_design->name);
                fprintf(stderr, "    comparison_type = '%s'\n", comparison_name[comp_type]);
            #endif
            long hits = match_design(current_design, comp_type);
            #ifdef DEBUG
                fprintf(stderr, "Design \"%s\" scored %ld points\n", current_design->name, hits);
            #endif
            if (hits > maxhits) {
                maxhits = hits;
                result = current_design;
            }
        }
        if (maxhits > 2) {
            break;   /* do not try other comparison types if one found something */
        }
    }

    #ifdef DEBUG
        if (result) {
            fprintf(stderr, "CHOOSING \"%s\" design (%ld hits).\n", result->name, maxhits);
        }
        else {
            fprintf(stderr, "NO DESIGN FOUND WITH EVEN ONE HIT POINT!\n");
        }
    #endif
    return result;
}


/* vim: set cindent sw=4: */
