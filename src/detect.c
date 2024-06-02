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
#include "logging.h"
#include "shape.h"
#include "tools.h"
#include "unicode.h"
#include "detect.h"



char *comparison_name[] = {
        "literal", "ignore_invisible_input", "ignore_invisible_shape", "ignore_invisible_all"
};



int input_is_mono()
{
    int result = 1;
    for (size_t line_no = 0; line_no < input.num_lines; line_no++) {
        if (input.lines[line_no].text->num_chars_invisible > 0) {
            result = 0;
            break;
        }
    }
    log_debug(__FILE__, MAIN, "Input is %s\n", result ? "mono" : "potentially colored");
    return result;
}



int design_is_mono(design_t *design)
{
    for (shape_t scnt = 0; scnt < NUM_SHAPES; ++scnt) {
        if (isempty(design->shape + scnt)) {
            continue;
        }
        for (size_t line_no = 0; line_no < design->shape[scnt].height; line_no++) {
            bxstr_t *shape_line = design->shape[scnt].mbcs[line_no];
            if (shape_line->num_chars_invisible > 0) {
                log_debug(__FILE__, MAIN, "Design is potentially colored\n");
                return 0;
            }
        }
    }
    log_debug(__FILE__, MAIN, "Design is mono\n");
    return 1;
}



int comp_type_is_viable(comparison_t comp_type, int mono_input, int mono_design)
{
    int result = 1;
    if ((comp_type == literal && mono_input != mono_design)
            || (comp_type == ignore_invisible_input && (mono_input || !mono_design))
            || (comp_type == ignore_invisible_shape && (!mono_input || mono_design))
            || (comp_type == ignore_invisible_all && (mono_input || mono_design)))
    {
        result = 0;
    }
    return result;
}



static int *determine_empty_sides(design_t *current_design)
{
    int *result = (int *) calloc(NUM_SIDES, sizeof(int));

    for (size_t j = 0; j < NUM_SIDES; ++j) {
        result[j] = empty_side(current_design->shape, j);
    }
    log_debug(__FILE__, MAIN, "Empty sides: TOP %d, LEFT %d, BOTTOM %d, RIGHT %d\n",
            result[BTOP], result[BLEF], result[BBOT], result[BRIG]);
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



uint32_t *prepare_comp_shape(
        design_t *design, shape_t shape, size_t shape_line_idx, comparison_t comp_type, int trim_left, int trim_right)
{
    sentry_t shape_def = design->shape[shape];
    if (shape_line_idx >= shape_def.height) {
        bx_fprintf(stderr, "%s: prepare_comp_shape(\"%s\", %s, %d, %s, %d, %d): Index out of bounds\n", PROJECT,
                design->name, shape, (int) shape_line_idx, comparison_name[comp_type], trim_left, trim_right);
        return NULL;
    }

    bxstr_t *shape_line = shape_def.mbcs[shape_line_idx];
    uint32_t *result = NULL;

    if ((comp_type == ignore_invisible_shape || comp_type == ignore_invisible_all)
            && shape_line->num_chars_invisible > 0)
    {
        size_t ltrim = trim_left ? shape_line->indent : 0;
        uint32_t *visible = bxs_filter_visible(shape_line);
        result = u32_strdup(visible + ltrim);
        BFREE(visible);

        if (trim_right && shape_line->trailing > 0) {
            set_char_at(result, shape_line->num_chars_visible - ltrim - shape_line->trailing, char_nul);
        }
    }
    else {
        result = u32_strdup(trim_left ? bxs_unindent_ptr(shape_line) : shape_line->memory);

        if (trim_right && shape_line->trailing > 0) {
            size_t x = shape_line->num_chars_visible - (trim_left ? shape_line->indent : 0) - shape_line->trailing;
            set_char_at(result, shape_line->first_char[x], char_nul);
        }
    }
    return result;
}



uint32_t *prepare_comp_input(size_t input_line_idx, int trim_left, comparison_t comp_type, size_t offset_right,
    size_t *out_indent, size_t *out_trailing)
{
    log_debug(__FILE__, MAIN, "prepare_comp_input(%d, %s, %s, %d, %p, %p)", (int) input_line_idx,
                trim_left ? "true" : "false", comparison_name[comp_type], (int) offset_right, out_indent, out_trailing);

    if (input_line_idx >= input.num_lines) {
        bx_fprintf(stderr, "%s: prepare_comp_input(%d, %d, %s, %d): Index out of bounds\n", PROJECT,
                (int) input_line_idx, trim_left, comparison_name[comp_type], (int) offset_right);
        log_debug_cont(MAIN, " -> (null)\n");
        return NULL;
    }
    bxstr_t *input_line = input.lines[input_line_idx].text;

    uint32_t *result = NULL;
    if (comp_type == ignore_invisible_input || comp_type == ignore_invisible_all) {
        if (offset_right > 0) {
            uint32_t *visible = get_visible_text(input.lines + input_line_idx);
            uint32_t *p = visible + input_line->num_chars_visible - input_line->trailing - offset_right;
            if (p >= visible) {
                result = p;
                if (out_indent != NULL) {
                    *out_indent = BMAX(input_line->indent - (size_t) (p - visible), (size_t) 0);
                }
            }
        }
        else {
            size_t ltrim = trim_left ? input_line->indent : 0;
            result = get_visible_text(input.lines + input_line_idx) + ltrim;
            if (out_indent != NULL) {
                *out_indent = trim_left ? 0 : input_line->indent;
            }
        }
        if (out_trailing != NULL) {
            *out_trailing = input_line->trailing;
        }
    }
    else {
        if (offset_right > 0) {
            int idx = (int) input_line->first_char[input_line->num_chars_visible - input_line->trailing]
                        - offset_right;
            if (idx >= 0) {
                result = input_line->memory + idx;
                if (out_indent != NULL) {
                    *out_indent = BMAX(input_line->first_char[input_line->indent] - (size_t) idx, (size_t) 0);
                }
            }
        }
        else {
            result = trim_left ? bxs_unindent_ptr(input_line) : input_line->memory;
            if (out_indent != NULL) {
                *out_indent = trim_left ? 0 : input_line->first_char[input_line->indent];
            }
        }
        if (out_trailing != NULL) {
            *out_trailing = input_line->num_chars
                    - input_line->first_char[input_line->num_chars_visible - input_line->trailing];
        }
    }

    if (is_debug_logging(MAIN)) {
        char *out_result = u32_strconv_to_output(result);
        log_debug_cont(MAIN, " -> \"%s\"\n", out_result);
        BFREE(out_result);
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

        uint32_t *shape_relevant = prepare_comp_shape(current_design, corner, j, comp_type, 1, 0);
        size_t length_relevant = u32_strlen(shape_relevant);

        for (size_t k = 0; k < current_design->shape[corner].height; ++k) {
            size_t a = k;
            if (corner == SW) {
                a += input.num_lines - current_design->shape[corner].height;
            }
            if (a >= input.num_lines) {
                break;
            }

            uint32_t *input_relevant = prepare_comp_input(a, 1, comp_type, 0, NULL, NULL);
            if (u32_strncmp(input_relevant, shape_relevant, length_relevant) == 0) {
                ++hits; /* CHECK more hit points for longer matches, or simple boxes might match too easily */
            }
        }
        BFREE(shape_relevant);
    }

    log_debug(__FILE__, MAIN, "Checking %s corner produced %d hits.\n", shape_name[corner], (int) hits);
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
        log_debug(__FILE__, MAIN, "Checking %s corner produced %d hits.\n", shape_name[corner], (int) hits);
        return hits;
    }

    for (size_t j = 0; j < current_design->shape[corner].height; ++j) {
        bxstr_t *shape_line = current_design->shape[corner].mbcs[j];
        if (bxs_is_blank(shape_line)) {
            continue;
        }

        uint32_t *shape_relevant = prepare_comp_shape(current_design, corner, j, comp_type, 0, 1);
        size_t length_relevant = u32_strlen(shape_relevant);

        for (size_t k = 0; k < current_design->shape[corner].height; ++k) {
            size_t a = k;
            if (corner == SE) {
                a += input.num_lines - current_design->shape[corner].height;
            }
            if (a >= input.num_lines) {
                break;
            }

            uint32_t *input_relevant = prepare_comp_input(a, 0, comp_type, length_relevant, NULL, NULL);
            if (input_relevant && (u32_strncmp(input_relevant, shape_relevant, length_relevant) == 0)) {
                ++hits; /* CHECK more hit points for longer matches, or simple boxes might match too easily */
            }
        }
        BFREE(shape_relevant);
    }

    log_debug(__FILE__, MAIN, "Checking %s corner produced %d hits.\n", shape_name[corner], (int) hits);
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
        /* horizontal box part is empty */
        log_debug(__FILE__, MAIN, "Checking %-3s shape produced %d hits.\n", shape_name[hshape], (int) hits);
        return hits;
    }

    for (size_t j = 0; j < current_design->shape[hshape].height; ++j) {
        bxstr_t *shape_line = current_design->shape[hshape].mbcs[j];
        if (bxs_is_blank(shape_line)) {
            continue;
        }

        uint32_t *shape_relevant = prepare_comp_shape(current_design, hshape, j, comp_type,
                is_blankward(current_design, hshape, j, 1),
                is_blankward(current_design, hshape, j, 0));
        size_t length_relevant = u32_strlen(shape_relevant);

        for (size_t k = 0; k < current_design->shape[hshape].height; ++k) {
            size_t line_idx = k;
            if (hshape >= SSE && hshape <= SSW) {
                line_idx += input.num_lines - current_design->shape[hshape].height;
            }
            if (line_idx >= input.num_lines) {
                break;
            }

            uint32_t *input_relevant = prepare_comp_input(line_idx, 1, comp_type, 0, NULL, NULL);
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
        BFREE(shape_relevant);
    }

    log_debug(__FILE__, MAIN, "Checking %-3s shape produced %d hits.\n", shape_name[hshape], (int) hits);
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
        /* no hits */
    }
    else if (isempty(current_design->shape + vshape)) {
        /* no hits */
    }
    else {
        for (size_t k = empty[BTOP] ? 0 : current_design->shape[NW].height;
                k < input.num_lines - (empty[BBOT] ? 0 : current_design->shape[SW].height); ++k)
        {
            uint32_t *input_relevant = prepare_comp_input(k, 1, comp_type, 0, NULL, NULL);

            for (size_t j = 0; j < current_design->shape[vshape].height; ++j) {
                bxstr_t *shape_line = current_design->shape[vshape].mbcs[j];
                if (bxs_is_blank(shape_line)) {
                    continue;
                }

                uint32_t *shape_relevant = prepare_comp_shape(current_design, vshape, j, comp_type, 1, 0);
                size_t length_relevant = u32_strlen(shape_relevant);

                if (u32_strncmp(input_relevant, shape_relevant, length_relevant) == 0) {
                    ++hits;
                    break;
                }
                BFREE(shape_relevant);
            }
        }
    }

    log_debug(__FILE__, MAIN, "Checking %-3s shape produced %d hits.\n", shape_name[vshape], (int) hits);
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
        /* no hits */
    }
    else if (isempty(current_design->shape + vshape)) {
        /* no hits */
    }
    else {
        for (size_t j = 0; j < current_design->shape[vshape].height; ++j) {
            bxstr_t *shape_line = current_design->shape[vshape].mbcs[j];
            if (bxs_is_blank(shape_line)) {
                continue;
            }

            uint32_t *shape_relevant = prepare_comp_shape(current_design, vshape, j, comp_type, 1, 1);
            size_t length_relevant = u32_strlen(shape_relevant);

            for (size_t k = empty[BTOP] ? 0 : current_design->shape[NW].height;
                    k < input.num_lines - (empty[BBOT] ? 0 : current_design->shape[SW].height); ++k)
            {
                uint32_t *input_relevant = prepare_comp_input(k, 0, comp_type, length_relevant, NULL, NULL);
                if (input_relevant != NULL && u32_strncmp(input_relevant, shape_relevant, length_relevant) == 0) {
                    ++hits;
                    break;
                }
            }
            BFREE(shape_relevant);
        }
    }

    log_debug(__FILE__, MAIN, "Checking %-3s shape produced %d hits.\n", shape_name[vshape], (int) hits);
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
            if (!comp_type_is_viable(comp_type, mono_input, mono_design)) {
                log_debug(__FILE__, MAIN, "Design \"%s\" skipped for comparison type '%s' because mono_input=%d and "
                        "mono_design=%d\n", current_design->name, comparison_name[comp_type], mono_input, mono_design);
                continue;
            }

            log_debug(__FILE__, MAIN, "CONSIDERING DESIGN ---- \"%s\" ---------------\n", current_design->name);
            log_debug(__FILE__, MAIN, "    comparison_type = %s\n", comparison_name[comp_type]);

            long hits = match_design(current_design, comp_type);
            log_debug(__FILE__, MAIN, "Design \"%s\" scored %ld points\n", current_design->name, hits);
            if (hits > maxhits) {
                maxhits = hits;
                result = current_design;
            }
        }
        if (maxhits > 2) {
            break;   /* do not try other comparison types if one found something */
        }
    }

    if (is_debug_logging(MAIN)) {
        if (result) {
            log_debug(__FILE__, MAIN, "CHOOSING \"%s\" design (%ld hits).\n", result->name, maxhits);
        }
        else {
            log_debug(__FILE__, MAIN, "NO DESIGN FOUND WITH EVEN ONE HIT POINT!\n");
        }
    }
    return result;
}


/* vim: set cindent sw=4: */
