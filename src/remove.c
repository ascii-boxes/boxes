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
 * Box removal, i.e. the deletion of boxes
 */

#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistr.h>
#include <uniwidth.h>

#include "boxes.h"
#include "detect.h"
#include "remove.h"
#include "shape.h"
#include "tools.h"
#include "unicode.h"



typedef struct _line_ctx_t {
    /** index of the first character of the west shape */
    size_t west_start;

    /** index of the character following the last character of the west shape. If equal to `west_start`, then no west
     *  shape was detected. */
    size_t west_end;

    /** the length in characters of the matched west shape part */
    size_t west_quality;

    /** index of the first character of the east shape */
    size_t east_start;

    /** index of the character following the last character of the east shape. If equal to `east_start`, then no east
     *  shape was detected.  */
    size_t east_end;

    /** the length in characters of the matched east shape part */
    size_t east_quality;

    /** the input line to which the above values refer. Will look very different depending on comparison type. */
    uint32_t *input_line_used;
} line_ctx_t;



typedef struct _remove_ctx_t {
    /** Array of flags indicating which sides of the box design are defined as empty. Access via `BTOP` etc. constants. */
    int empty_side[NUM_SIDES];

    /** Flag indicating that there are no invisible characters in the definition of the design we are removing. */
    int design_is_mono;

    /** Flag indicating that there are no invisible characters in the input. */
    int input_is_mono;

    /** Index into `input.lines` of the first line of the box (topmost box line). Lines above are blank. */
    size_t top_start_idx;

    /** Index into `input.lines` of the line following the last line of the top part of the box. If the top part of the
     *  box is empty or missing, this value will be equal to `top_start_idx`. */
    size_t top_end_idx;

    /** Index into `input.lines` of the first line of the bottom side of the box. */
    size_t bottom_start_idx;

    /** Index into `input.lines` of the line following the last line of the bottom part of the box. If the bottom part
     *  of the box is empty or missing, this value will be equal to `bottom_start_idx`. Lines below are blank. */
    size_t bottom_end_idx;

    /** Information on the vertical east and west shapes in body lines, one entry for each line between `top_end_idx`
     *  (inclusive) and `bottom_start_idx` (exclusive) */
    line_ctx_t *body;
} remove_ctx_t;



typedef struct _shape_line_ctx_t {
    /** flag indicating whether the shape is empty */
    int empty;

    /** one line of a shape, possibly trimmed, with invisible characters filtered according to the comparison type */
    uint32_t *text;

    /** the length of `text` in characters */
    size_t length;

    /** flag indicating whether the shape to which this line belongs is elastic */
    int elastic;
} shape_line_ctx_t;



static size_t find_first_line()
{
    size_t result = input.num_lines;
    for (size_t line_idx = 0; line_idx < input.num_lines; line_idx++) {
        if (!bxs_is_blank(input.lines[line_idx].text)) {
            result = line_idx;
            break;
        }
    }
    return result;
}



static size_t find_last_line()
{
    size_t result = input.num_lines - 1;
    for (long line_idx = (long) input.num_lines - 1; line_idx >= 0; line_idx--) {
        if (!bxs_is_blank(input.lines[line_idx].text)) {
            result = (size_t) line_idx;
            break;
        }
    }
    return result;
}



/**
 * (horizontal middle match)
 * Recursive helper function for match_horiz_line(), uses backtracking.
 * @param shapes_relevant the prepared shape lines to be concatenated
 * @param cur_pos current position in the input line being matched
 * @param shape_idx index into `shapes_relevant` indicating which shape to try now
 * @param end_pos first character of the east corner
 * @return `== 1`: success;
 *         `== 0`: failed to match
 */
static int hmm(shape_line_ctx_t *shapes_relevant, uint32_t *cur_pos, size_t shape_idx, uint32_t *end_pos)
{
    #ifdef DEBUG
        char *out_cur_pos = u32_strconv_to_output(cur_pos);
        char *out_end_pos = u32_strconv_to_output(end_pos);
        fprintf(stderr, "hmm(shapes_relevant, \"%s\", %d, \"%s\")", out_cur_pos, (int) shape_idx, out_end_pos);
        BFREE(out_cur_pos);
        BFREE(out_end_pos);
    #endif

    int result = 0;

    if (cur_pos > end_pos) {
        result = 0; /* last shape tried was too long */
    }
    else if (shape_idx == SHAPES_PER_SIDE) {
        if (cur_pos == end_pos) {
            result = 1; /* success */
        }
    }
    else if (shapes_relevant[shape_idx].empty) {
        result = hmm(shapes_relevant, cur_pos, shape_idx + 1, end_pos);
    }
    else if (u32_strncmp(cur_pos, shapes_relevant[shape_idx].text, shapes_relevant[shape_idx].length) == 0) {
        int rc = 0;
        if (shapes_relevant[shape_idx].elastic) {
            rc = hmm(shapes_relevant, cur_pos + shapes_relevant[shape_idx].length, shape_idx, end_pos);
        }
        if (rc == 0) {
            result = hmm(shapes_relevant, cur_pos + shapes_relevant[shape_idx].length, shape_idx + 1, end_pos);
        }
    }

    #ifdef DEBUG
        fprintf(stderr, " -> %d\n", result);
    #endif
    return result;
}



static shape_line_ctx_t *prepare_comp_shapes_horiz(int hside, comparison_t comp_type, size_t shape_line_idx)
{
    shape_t start_shape = hside == BTOP ? NW : SW;
    shape_t top_side_shapes[SHAPES_PER_SIDE - CORNERS_PER_SIDE] = {NNW, N, NNE};
    shape_t bottom_side_shapes[SHAPES_PER_SIDE - CORNERS_PER_SIDE] = {SSW, S, SSE};
    shape_t end_shape = hside == BTOP ? NE : SE;
    shape_t side_shapes[SHAPES_PER_SIDE - CORNERS_PER_SIDE];
    if (hside == BTOP) {
        memcpy(side_shapes, top_side_shapes, (SHAPES_PER_SIDE - CORNERS_PER_SIDE) * sizeof(shape_t));
    }
    else {
        memcpy(side_shapes, bottom_side_shapes, (SHAPES_PER_SIDE - CORNERS_PER_SIDE) * sizeof(shape_t));
    }

    shape_line_ctx_t *shapes_relevant = (shape_line_ctx_t *) calloc(SHAPES_PER_SIDE, sizeof(shape_line_ctx_t));

    shapes_relevant[0].text = prepare_comp_shape(opt.design, start_shape, shape_line_idx, comp_type, 1, 0);
    shapes_relevant[0].length = u32_strlen(shapes_relevant[0].text);
    shapes_relevant[0].elastic = 0;
    shapes_relevant[0].empty = 0;

    for (size_t i = 0; i < SHAPES_PER_SIDE - CORNERS_PER_SIDE; i++) {
        shapes_relevant[i + 1].empty = isempty(opt.design->shape + side_shapes[i]);
        if (!shapes_relevant[i + 1].empty) {
            shapes_relevant[i + 1].text
                    = prepare_comp_shape(opt.design, side_shapes[i], shape_line_idx, comp_type, 0, 0);
            shapes_relevant[i + 1].length = u32_strlen(shapes_relevant[i + 1].text);
            shapes_relevant[i + 1].elastic = opt.design->shape[side_shapes[i]].elastic;
        }
    }

    shapes_relevant[SHAPES_PER_SIDE - 1].text
            = prepare_comp_shape(opt.design, end_shape, shape_line_idx, comp_type, 0, 1);
    shapes_relevant[SHAPES_PER_SIDE - 1].length = u32_strlen(shapes_relevant[SHAPES_PER_SIDE - 1].text);
    shapes_relevant[SHAPES_PER_SIDE - 1].elastic = 0;
    shapes_relevant[SHAPES_PER_SIDE - 1].empty = 0;

    return shapes_relevant;
}



static int match_horiz_line(remove_ctx_t *ctx, int hside, size_t input_line_idx, size_t shape_line_idx)
{
    int result = 0;
    for (comparison_t comp_type = 0; comp_type < NUM_COMPARISON_TYPES; comp_type++) {
        if (!comp_type_is_viable(comp_type, ctx->input_is_mono, ctx->design_is_mono)) {
            continue;
        }

        shape_line_ctx_t *shapes_relevant = prepare_comp_shapes_horiz(hside, comp_type, shape_line_idx);

        uint32_t *cur_pos = NULL;
        uint32_t *input_prepped = prepare_comp_input(input_line_idx, 1, comp_type, 0, NULL, NULL);
        if (input_prepped != NULL
                && u32_strncmp(input_prepped, shapes_relevant[0].text, shapes_relevant[0].length) == 0)
        {
            cur_pos = input_prepped + shapes_relevant[0].length;
        }

        uint32_t *end_pos = NULL;
        input_prepped = prepare_comp_input(
                input_line_idx, 0, comp_type, shapes_relevant[SHAPES_PER_SIDE - 1].length, NULL, NULL);
        if (input_prepped != NULL && u32_strncmp(input_prepped, shapes_relevant[SHAPES_PER_SIDE - 1].text,
                shapes_relevant[SHAPES_PER_SIDE - 1].length) == 0)
        {
            end_pos = input_prepped;
        }

        if (cur_pos && end_pos) {
            result = hmm(shapes_relevant, cur_pos, 1, end_pos);
        }

        for (size_t i = 0; i < SHAPES_PER_SIDE; i++) {
            BFREE(shapes_relevant[i].text);
        }
        BFREE(shapes_relevant);

        if (result) {
            #ifdef DEBUG
                fprintf(stderr, "Matched %s side line using comp_type=%s and shape_line_idx=%d\n",
                    hside == BTOP ? "top" : "bottom", comparison_name[comp_type], (int) shape_line_idx);
            #endif
            break;
        }
    }

    return result;
}



static size_t find_top_side(remove_ctx_t *ctx)
{
    size_t result = ctx->top_start_idx;
    sentry_t *shapes = opt.design->shape;
    for (size_t input_line_idx = ctx->top_start_idx, shape_line_count = 0;
            input_line_idx < input.num_lines && shape_line_count < shapes[NE].height;
            input_line_idx++, shape_line_count++)
    {
        int matched = 0;
        for (size_t shape_line_idx = input_line_idx - ctx->top_start_idx;;
                shape_line_idx = (shape_line_idx + 1) % shapes[NE].height)
        {
            if (match_horiz_line(ctx, BTOP, input_line_idx, shape_line_idx)) {
                matched = 1;
                break;
            }
        }
        if (!matched) {
            break;
        }
        result = input_line_idx + 1;
    }
    return result;
}



static size_t find_bottom_side(remove_ctx_t *ctx)
{
    size_t result = ctx->top_start_idx;
    sentry_t *shapes = opt.design->shape;
    for (long input_line_idx = (long) ctx->bottom_end_idx - 1, shape_line_count = (long) shapes[SE].height - 1;
            input_line_idx >= 0 && shape_line_count >= 0;
            input_line_idx--, shape_line_count--)
    {
        int matched = 0;
        for (size_t shape_line_idx = shapes[SE].height - (ctx->bottom_end_idx - input_line_idx);;
                shape_line_idx = (shape_line_idx + 1) % shapes[SE].height)
        {
            if (match_horiz_line(ctx, BBOT, input_line_idx, shape_line_idx)) {
                matched = 1;
                break;
            }
        }
        if (!matched) {
            break;
        }
        result = input_line_idx;
    }
    return result;
}



static size_t count_shape_lines(shape_t side_shapes[])
{
    size_t result = 0;
    for (size_t i = 0; i < SHAPES_PER_SIDE - CORNERS_PER_SIDE; i++) {
        if (!isempty(opt.design->shape + side_shapes[i])) {
            result += opt.design->shape[side_shapes[i]].height;
        }
    }
    return result;
}


static shape_line_ctx_t **prepare_comp_shapes_vert(int vside, comparison_t comp_type)
{
    shape_t west_side_shapes[SHAPES_PER_SIDE - CORNERS_PER_SIDE] = {WSW, W, WNW};
    shape_t east_side_shapes[SHAPES_PER_SIDE - CORNERS_PER_SIDE] = {ENE, E, ESE};
    shape_t side_shapes[SHAPES_PER_SIDE - CORNERS_PER_SIDE];
    if (vside == BLEF) {
        memcpy(side_shapes, west_side_shapes, (SHAPES_PER_SIDE - CORNERS_PER_SIDE) * sizeof(shape_t));
    }
    else {
        memcpy(side_shapes, east_side_shapes, (SHAPES_PER_SIDE - CORNERS_PER_SIDE) * sizeof(shape_t));
    }

    size_t num_shape_lines = count_shape_lines(side_shapes);

    /* allocate a NUL-terminated array: */
    shape_line_ctx_t **shape_lines = (shape_line_ctx_t **) calloc(num_shape_lines + 1, sizeof(shape_line_ctx_t *));

    for (size_t shape_idx = 0, i = 0; shape_idx < SHAPES_PER_SIDE - CORNERS_PER_SIDE; shape_idx++) {
        if (!isempty(opt.design->shape + side_shapes[shape_idx])) {
            shape_lines[i] = (shape_line_ctx_t *) calloc(1, sizeof(shape_line_ctx_t));
            for (size_t slno = 0; slno < opt.design->shape[side_shapes[shape_idx]].height; slno++, i++) {
                shape_lines[i]->text = prepare_comp_shape(opt.design, side_shapes[shape_idx], slno, comp_type, 0, 0);
                shape_lines[i]->empty = u32_is_blank(shape_lines[i]->text);
                shape_lines[i]->length = u32_strlen(shape_lines[i]->text);
                shape_lines[i]->elastic = opt.design->shape[side_shapes[shape_idx]].elastic;
            }
        }
    }

    return shape_lines;
}



static void free_shape_lines(shape_line_ctx_t **shape_lines)
{
    if (shape_lines != NULL) {
        for (shape_line_ctx_t **p = shape_lines; *p != NULL; p++) {
            BFREE((*p)->text);
            BFREE(*p);
        }
        BFREE(shape_lines);
    }
}



/**
 * Take a shape line and shorten it by cutting off blanks from both ends.
 * @param shape_line_ctx info record on the shape line to work on. Contains the original shape line, unshortened.
 * @param quality (IN/OUT) the current quality, here the value that was last tested. We will reduce this by one.
 * @param prefer_left if 1, first cut blanks from the start of the shape line, if 0, first cut at the end
 * @return the shortened shape line, in new memory, or NULL if further shortening was not possible
 */
static uint32_t *shorten(shape_line_ctx_t *shape_line_ctx, size_t *quality, int prefer_left)
{
    uint32_t *s = shape_line_ctx->text;
    uint32_t *e = shape_line_ctx->text + shape_line_ctx->length;
    size_t reduction_steps = shape_line_ctx->length - *quality + 1;
    for (size_t i = 0; i < reduction_steps; i++) {
        if (prefer_left) {
            if (is_blank(*s) && s < e) {
                s++;
            }
            else if (is_blank(*(e - 1)) && e > s) {
                e--;
            }
            else {
                break;
            }
        }
        else {
            if (is_blank(*(e - 1)) && e > s) {
                e--;
            }
            else if (is_blank(*s) && s < e) {
                s++;
            }
            else {
                break;
            }
        }
    }

    uint32_t *result = NULL;
    size_t new_quality = e - s;
    if (new_quality < *quality) {
        result = u32_strdup(s);
        set_char_at(result, new_quality, char_nul);
        *quality = new_quality;
    }
    return result;
}



static void match_vertical_side(remove_ctx_t *ctx, int vside, shape_line_ctx_t **shape_lines, uint32_t *input_line,
    size_t line_idx, size_t input_length, size_t input_indent, size_t input_trailing)
{
    line_ctx_t *line_ctx = ctx->body + (line_idx - ctx->top_end_idx);

    for (shape_line_ctx_t **shape_line_ctx = shape_lines; *shape_line_ctx != NULL; shape_line_ctx++) {
        if ((*shape_line_ctx)->empty) {
            continue;
        }

        size_t max_quality = (*shape_line_ctx)->length;
        size_t quality = max_quality;
        uint32_t *shape_text = (*shape_line_ctx)->text;
        uint32_t *to_free = NULL;
        while(shape_text != NULL) {
            uint32_t *p;
            if (vside == BLEF) {
                p = u32_strstr(input_line, shape_text);
            }
            else {
                p = u32_strnrstr(input_line, shape_text, quality, 0);
            }
            BFREE(to_free);
            shape_text = NULL;

            if (p == NULL) {
                shape_text = shorten(*shape_line_ctx, &quality, vside == BLEF);
                to_free = shape_text;
            }
            else if (vside == BLEF && ((size_t) (p - input_line) <= input_indent + (max_quality - quality))) {
                if (quality > line_ctx->west_quality) {
                    line_ctx->west_start = (size_t) (p - input_line);
                    line_ctx->west_end = line_ctx->west_start + quality;
                    line_ctx->west_quality = quality;
                    BFREE(line_ctx->input_line_used);
                    line_ctx->input_line_used = u32_strdup(input_line);
                }
            }
            else if (vside == BRIG && ((size_t) (p - input_line) >= input_length - input_trailing - quality)) {
                if (quality > line_ctx->east_quality) {
                    line_ctx->east_start = (size_t) (p - input_line);
                    line_ctx->east_end = line_ctx->east_start + quality;
                    line_ctx->east_quality = quality;
                    BFREE(line_ctx->input_line_used);
                    line_ctx->input_line_used = u32_strdup(input_line);
                }
            }
        }
    }
}



static int sufficient_body_quality(remove_ctx_t *ctx)
{
    size_t num_body_lines = ctx->bottom_start_idx - ctx->top_end_idx;
    size_t total_quality = 0;
    line_ctx_t *body = ctx->body;
    for (size_t body_line_idx = 0; body_line_idx < num_body_lines; body_line_idx++) {
        line_ctx_t line_ctx = body[body_line_idx];
        total_quality += line_ctx.east_quality + line_ctx.west_quality;
    }

    size_t max_quality = (opt.design->shape[NW].width + opt.design->shape[NE].width) * num_body_lines;
    /* If we manage to match 50%, then it is unlikely to improve with a different comparison mode. */
    int sufficient = total_quality > 0.5 * max_quality;
    #ifdef DEBUG
        fprintf(stderr, "sufficient_body_quality() found body match quality of %d/%d (%s).\n",
                (int) total_quality, (int) max_quality, sufficient ? "sufficient" : "NOT sufficient");
    #endif
    return sufficient;
}



static void find_vertical_shapes(remove_ctx_t *ctx)
{
    int west_empty = ctx->empty_side[BLEF];
    int east_empty = ctx->empty_side[BRIG];
    if (west_empty && east_empty) {
        return;
    }

    for (comparison_t comp_type = 0; comp_type < NUM_COMPARISON_TYPES; comp_type++) {
        if (!comp_type_is_viable(comp_type, ctx->input_is_mono, ctx->design_is_mono)) {
            continue;
        }

        shape_line_ctx_t **shape_lines_west = NULL;
        if (!west_empty) {
            shape_lines_west = prepare_comp_shapes_vert(BLEF, comp_type);
        }
        shape_line_ctx_t **shape_lines_east = NULL;
        if (!east_empty) {
            shape_lines_east = prepare_comp_shapes_vert(BRIG, comp_type);
        }

        for (size_t input_line_idx = ctx->top_end_idx; input_line_idx < ctx->bottom_start_idx; input_line_idx++) {
            size_t input_indent = 0;
            size_t input_trailing = 0;
            uint32_t *input_line = prepare_comp_input(input_line_idx, 0, comp_type, 0, &input_indent, &input_trailing);
            size_t input_length = u32_strlen(input_line);

            if (!west_empty) {
                match_vertical_side(ctx, BLEF, shape_lines_west,
                        input_line, input_line_idx, input_length, input_indent, input_trailing);
            }
            if (!east_empty) {
                match_vertical_side(ctx, BRIG, shape_lines_east,
                        input_line, input_line_idx, input_length, input_indent, input_trailing);
            }
        }

        free_shape_lines(shape_lines_west);
        free_shape_lines(shape_lines_east);

        if (sufficient_body_quality(ctx)) {
            break;
        }
        memset(ctx->body, 0, (ctx->bottom_start_idx - ctx->top_end_idx) * sizeof(line_ctx_t));
    }
}



/**
 * If the user didn't specify a design to remove, autodetect it.
 * Since this requires knowledge of all available designs, the entire config file had to be parsed (earlier).
 */
static void detect_design_if_needed()
{
    if (opt.design_choice_by_user == 0) {
        design_t *tmp = autodetect_design();
        if (tmp) {
            opt.design = tmp;
            #ifdef DEBUG
                fprintf(stderr, "Design autodetection: Removing box of design \"%s\".\n", opt.design->name);
            #endif
        }
        else {
            fprintf(stderr, "%s: Box design autodetection failed. Use -d option.\n", PROJECT);
            exit(EXIT_FAILURE);
        }
    }
}



static void toblank_plain(uint32_t **s, size_t from, size_t n)
{
    if (n > 0) {
        size_t cols = (size_t) u32_width((*s) + from, n, encoding);
        if (cols > n) {
            u32_insert_space_at(s, from, cols - n);
        }
        u32_set((*s) + from, char_space, cols);
    }
}



static void toblank_mixed(bxstr_t *org_line, uint32_t **s, size_t from, size_t to, size_t n)
{
    if (n > 0) {
        for (int i = (int) to - 1; i >= (int) from; i--) {
            if (bxs_is_visible_char(org_line, (size_t) i)) {
                size_t cols = (size_t) uc_width((*s)[i], encoding);
                if (cols > 1) {
                    u32_insert_space_at(s, (size_t) i + 1, cols - 1);
                }
                u32_set((*s) + i, char_space, 1);
            }
        }
    }
}



static size_t count_leading_spaces_plain(uint32_t *s, size_t max)
{
    size_t result = 0;
    for (size_t i = 0; i < max && is_char_at(s, i, char_space); i++, result++)
        ;
    return result;
}



static void clip_spaces_carefully_plain(remove_ctx_t *ctx, uint32_t *s)
{
    if (!ctx->empty_side[BLEF]) {
        size_t num_remove = count_leading_spaces_plain(s, opt.design->shape[NW].width);
        u32_move(s, s + num_remove, u32_strlen(s) - num_remove + 1);
    }
}



static uint32_t *clip_spaces_carefully_mixed(remove_ctx_t *ctx, uint32_t *s)
{
    if (!ctx->empty_side[BLEF]) {
        bxstr_t *bs = bxs_from_unicode(s);
        return bxs_ltrim(bs, opt.design->shape[NW].width);
    }
    return NULL;
}



static void remove_vertical_shapes(remove_ctx_t *ctx)
{
    size_t num_body_lines = ctx->bottom_start_idx - ctx->top_end_idx;
    line_ctx_t *body = ctx->body;
    for (size_t body_line_idx = 0; body_line_idx < num_body_lines; body_line_idx++) {
        line_ctx_t line_ctx = body[body_line_idx];
        bxstr_t *org_line = input.lines[ctx->top_end_idx + body_line_idx].text;

        if (org_line->num_chars_invisible > 0) {
            toblank_mixed(
                    org_line, &line_ctx.input_line_used, line_ctx.east_start, line_ctx.east_end, line_ctx.east_quality);
            toblank_mixed(
                    org_line, &line_ctx.input_line_used, line_ctx.west_start, line_ctx.west_end, line_ctx.west_quality);
            uint32_t *clipped = clip_spaces_carefully_mixed(ctx, line_ctx.input_line_used);
            if (clipped) {
                BFREE(line_ctx.input_line_used);
                line_ctx.input_line_used = clipped;
            }
        }
        else {
            toblank_plain(&line_ctx.input_line_used, line_ctx.east_start, line_ctx.east_quality); /* east first */
            toblank_plain(&line_ctx.input_line_used, line_ctx.west_start, line_ctx.west_quality);
            clip_spaces_carefully_plain(ctx, line_ctx.input_line_used);
        }
    }
}



static void free_line_text(line_t *line)
{
    BFREE(line->cache_visible);
    bxs_free(line->text);
    line->text = NULL;
}



static void free_line(line_t *line)
{
    free_line_text(line);
    BFREE(line->tabpos);
    line->tabpos_len = 0;
}



static void killblank(remove_ctx_t *ctx)
{
    while (ctx->top_end_idx < ctx->bottom_start_idx && empty_line(input.lines + ctx->top_end_idx)) {
        #ifdef DEBUG
            fprintf(stderr, "Killing leading blank line in box body.\n");
        #endif
        ++(ctx->top_end_idx);
    }
    while (ctx->bottom_start_idx > ctx->top_end_idx && empty_line(input.lines + ctx->bottom_start_idx - 1)) {
        #ifdef DEBUG
            fprintf(stderr, "Killing trailing blank line in box body.\n");
        #endif
        --(ctx->bottom_start_idx);
    }
}



static void apply_results_to_input(remove_ctx_t *ctx)
{
    for (size_t j = ctx->top_end_idx; j < ctx->bottom_start_idx; ++j) {
        free_line_text(input.lines + j);
        input.lines[j].text = bxs_from_unicode(ctx->body[j - ctx->top_end_idx].input_line_used);
        // FIXME now we might have lost the input colors
    }

    if (opt.killblank) {
        killblank(ctx);
    }

    if (ctx->top_end_idx > ctx->top_start_idx) {
        for (size_t j = ctx->top_start_idx; j < ctx->top_end_idx; ++j) {
            free_line(input.lines + j);
        }
        memmove(input.lines + ctx->top_start_idx, input.lines + ctx->top_end_idx,
                (input.num_lines - ctx->top_end_idx) * sizeof(line_t));
        size_t num_lines_removed = ctx->top_end_idx - ctx->top_start_idx;
        input.num_lines -= num_lines_removed;
        ctx->bottom_start_idx -= num_lines_removed;
        ctx->bottom_end_idx -= num_lines_removed;
    }

    if (ctx->bottom_end_idx > ctx->bottom_start_idx) {
        for (size_t j = ctx->bottom_start_idx; j < ctx->bottom_end_idx; ++j) {
            free_line(input.lines + j);
        }
        if (ctx->bottom_end_idx < input.num_lines) {
            memmove(input.lines + ctx->bottom_start_idx, input.lines + ctx->bottom_end_idx,
                    (input.num_lines - ctx->bottom_end_idx) * sizeof(line_t));
        }
        input.num_lines -= ctx->bottom_end_idx - ctx->bottom_start_idx;
    }

    input.maxline = 0;
    input.indent = SIZE_MAX;
    for (size_t j = 0; j < input.num_lines; ++j) {
        if (input.lines[j].text->num_columns > input.maxline) {
            input.maxline = input.lines[j].text->num_columns;
        }
        if (input.lines[j].text->indent < input.indent) {
            input.indent = input.lines[j].text->indent;
        }
    }

    size_t num_lines_removed = BMAX(ctx->top_end_idx - ctx->top_start_idx, (size_t) 0)
            + BMAX(ctx->bottom_end_idx - ctx->bottom_start_idx, (size_t) 0);
    memset(input.lines + input.num_lines, 0, num_lines_removed * sizeof(line_t));

    #ifdef DEBUG
        print_input_lines(" (remove_box) after box removal");
        fprintf(stderr, "Number of lines shrunk by %d.\n", (int) num_lines_removed);
    #endif
}



int remove_box()
{
    detect_design_if_needed();

    remove_ctx_t *ctx = (remove_ctx_t *) calloc(1, sizeof(remove_ctx_t));
    ctx->empty_side[BTOP] = empty_side(opt.design->shape, BTOP);
    ctx->empty_side[BRIG] = empty_side(opt.design->shape, BRIG);
    ctx->empty_side[BBOT] = empty_side(opt.design->shape, BBOT);
    ctx->empty_side[BLEF] = empty_side(opt.design->shape, BLEF);

    ctx->design_is_mono = design_is_mono(opt.design);
    ctx->input_is_mono = input_is_mono();

    ctx->top_start_idx = find_first_line();
    if (ctx->top_start_idx >= input.num_lines) {
        return 0;  /* all lines were already blank, so there is no box to remove */
    }

    if (ctx->empty_side[BTOP]) {
        ctx->top_end_idx = ctx->top_start_idx;
    }
    else {
        ctx->top_end_idx = find_top_side(ctx);
    }

    ctx->bottom_end_idx = find_last_line() + 1;
    if (ctx->empty_side[BBOT]) {
        ctx->bottom_start_idx = ctx->bottom_end_idx;
    }
    else {
        ctx->bottom_start_idx = find_bottom_side(ctx);
    }

    ctx->body = (line_ctx_t *) calloc(ctx->bottom_start_idx - ctx->top_end_idx, sizeof(line_ctx_t));
    if (ctx->bottom_start_idx > ctx->top_end_idx) {
        find_vertical_shapes(ctx);
    }

    remove_vertical_shapes(ctx);

    apply_results_to_input(ctx);

    for (size_t i = 0; i < ctx->bottom_start_idx - ctx->top_end_idx; i++) {
        BFREE(ctx->body[i].input_line_used);
    }
    BFREE(ctx->body);
    BFREE(ctx);
    return 1;
}



void output_input(const int trim_only)
{
    size_t indent;
    int ntabs, nspcs;

    #ifdef DEBUG
        fprintf(stderr, "output_input() - enter (trim_only=%d)\n", trim_only);
    #endif
    for (size_t j = 0; j < input.num_lines; ++j) {
        if (input.lines[j].text == NULL) {
            continue;
        }
        bxstr_t *temp = bxs_rtrim(input.lines[j].text);
        bxs_free(input.lines[j].text);
        input.lines[j].text = temp;
        if (trim_only) {
            continue;
        }

        char *indentspc = NULL;
        if (opt.tabexp == 'u') {
            indent = input.lines[j].text->indent;
            ntabs = indent / opt.tabstop;
            nspcs = indent % opt.tabstop;
            indentspc = (char *) malloc(ntabs + nspcs + 1);
            if (indentspc == NULL) {
                perror(PROJECT);
                return;
            }
            memset(indentspc, (int) '\t', ntabs);
            memset(indentspc + ntabs, (int) ' ', nspcs);
            indentspc[ntabs + nspcs] = '\0';
        }
        else if (opt.tabexp == 'k') {
            uint32_t *indent32 = tabbify_indent(j, NULL, input.indent);
            indentspc = u32_strconv_to_output(indent32);
            BFREE(indent32);
            indent = input.indent;
        }
        else {
            indentspc = (char *) strdup("");
            indent = 0;
        }

        char *outtext = u32_strconv_to_output(bxs_first_char_ptr(input.lines[j].text, indent));
        fprintf(opt.outfile, "%s%s%s", indentspc, outtext,
                (input.final_newline || j < input.num_lines - 1 ? opt.eol : ""));
        BFREE(outtext);
        BFREE(indentspc);
    }
}


/* vim: set sw=4: */
