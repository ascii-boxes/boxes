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

#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistr.h>
#include <uniwidth.h>

#include "boxes.h"
#include "detect.h"
#include "logging.h"
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

    /** The current comparison type. This changes whenever another comparison type is tried. */
    comparison_t comp_type;

    /** number of lines in `body` */
    size_t body_num_lines;

    /** Information on the vertical east and west shapes in body lines, one entry for each line between `top_end_idx`
     *  (inclusive) and `bottom_start_idx` (exclusive) */
    line_ctx_t *body;
} remove_ctx_t;



static void debug_print_remove_ctx(remove_ctx_t *ctx, char *heading)
{
    if (is_debug_logging(MAIN)) {
        log_debug(__FILE__, MAIN, "Remove Context %s:\n", heading);
        log_debug(__FILE__, MAIN, "    - empty_side[BTOP] = %s\n", ctx->empty_side[BTOP] ? "true" : "false");
        log_debug(__FILE__, MAIN, "    - empty_side[BRIG] = %s\n", ctx->empty_side[BRIG] ? "true" : "false");
        log_debug(__FILE__, MAIN, "    - empty_side[BBOT] = %s\n", ctx->empty_side[BBOT] ? "true" : "false");
        log_debug(__FILE__, MAIN, "    - empty_side[BLEF] = %s\n", ctx->empty_side[BLEF] ? "true" : "false");
        log_debug(__FILE__, MAIN, "    - design_is_mono = %s\n", ctx->design_is_mono ? "true" : "false");
        log_debug(__FILE__, MAIN, "    - input_is_mono = %s\n", ctx->input_is_mono ? "true" : "false");
        log_debug(__FILE__, MAIN, "    - top_start_idx = %d\n", (int) ctx->top_start_idx);
        log_debug(__FILE__, MAIN, "    - top_end_idx = %d\n", (int) ctx->top_end_idx);
        log_debug(__FILE__, MAIN, "    - bottom_start_idx = %d\n", (int) ctx->bottom_start_idx);
        log_debug(__FILE__, MAIN, "    - bottom_end_idx = %d\n", (int) ctx->bottom_end_idx);
        log_debug(__FILE__, MAIN, "    - comp_type = %s\n", comparison_name[ctx->comp_type]);
        log_debug(__FILE__, MAIN, "    - body (%d lines):\n", (int) ctx->body_num_lines);
        for (size_t i = 0; i < ctx->body_num_lines; i++) {
            if (ctx->body[i].input_line_used != NULL) {
                char *out_input_line_used = u32_strconv_to_output(ctx->body[i].input_line_used);
                log_debug(__FILE__, MAIN, "        - lctx: \"%s\" (%d characters)\n", out_input_line_used,
                    (int) u32_strlen(ctx->body[i].input_line_used));
                BFREE(out_input_line_used);
            }
            else {
                log_debug(__FILE__, MAIN, "        - lctx: (null)\n");
            }
            bxstr_t *orgline = input.lines[ctx->top_end_idx + i].text;
            if (orgline != NULL) {
                char *out_orgline = bxs_to_output(orgline);
                log_debug(__FILE__, MAIN, "          orgl: \"%s\" (%d characters, %d columns)\n", out_orgline,
                    (int) orgline->num_chars, (int) orgline->num_columns);
                BFREE(out_orgline);
            }
            else {
                log_debug(__FILE__, MAIN, "          orgl: (null)\n");
            }
            log_debug(__FILE__, MAIN, "                west: %d-%d (quality: %d), east: %d-%d (quality: %d)\n",
                (int) ctx->body[i].west_start, (int) ctx->body[i].west_end, (int) ctx->body[i].west_quality,
                (int) ctx->body[i].east_start, (int) ctx->body[i].east_end, (int) ctx->body[i].east_quality);
        }
    }
}



static void debug_print_shapes_relevant(shape_line_ctx_t *shapes_relevant)
{
    if (is_debug_logging(MAIN)) {
        log_debug(__FILE__, MAIN, "  shapes_relevant = {");
        for (size_t ds = 0; ds < SHAPES_PER_SIDE; ds++) {
            if (shapes_relevant[ds].empty) {
                log_debug_cont(MAIN, "-");
            }
            else {
                char *out_shp_text = bxs_to_output(shapes_relevant[ds].text);
                log_debug_cont(MAIN, "\"%s\"(%d%s)", out_shp_text, (int) shapes_relevant[ds].text->num_chars,
                    shapes_relevant[ds].elastic ? "E" : "");
                BFREE(out_shp_text);
            }
            if (ds < SHAPES_PER_SIDE - 1) {
                log_debug_cont(MAIN, ", ");
            }
        }
        log_debug_cont(MAIN, "}\n");
    }
}



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



static int is_shape_line_empty(shape_line_ctx_t *shapes_relevant, size_t shape_idx)
{
    if (shape_idx < SHAPES_PER_SIDE) {
        return shapes_relevant[shape_idx].empty || bxs_is_blank(shapes_relevant[shape_idx].text);
    }
    return 1;
}



static int non_empty_shapes_after(shape_line_ctx_t *shapes_relevant, size_t shape_idx)
{
    /* CHECK Can we use shape->is_blank_rightward? */
    for (size_t i = shape_idx + 1; i < SHAPES_PER_SIDE - 1; i++) {
        if (!is_shape_line_empty(shapes_relevant, i)) {
            return 1;
        }
    }
    return 0;
}



static int is_blank_between(uint32_t *start, uint32_t *end)
{
    for (uint32_t *p = start; p < end; p++) {
        if (!is_blank(*p)) {
            return 0;
        }
    }
    return 1;
}



/**
 * Take a shape line and shorten it by cutting off blanks from both ends.
 * @param shape_line_ctx info record on the shape line to work on. Contains the original shape line, unshortened.
 * @param quality (IN/OUT) the current quality, here the value that was last tested. We will reduce this by one.
 * @param prefer_left if 1, first cut all blanks from the start of the shape line, if 0, first cut at the end
 * @param allow_left if 1, blanks may be cut from the left of the shape line, if 0, we never cut from the left
 * @param allow_right if 1, blanks may be cut from the right of the shape line, if 0, we never cut from the right
 * @return the shortened shape line, in new memory, or NULL if further shortening was not possible
 */
uint32_t *shorten(shape_line_ctx_t *shape_line_ctx, size_t *quality, int prefer_left, int allow_left, int allow_right)
{
    if (shape_line_ctx == NULL || shape_line_ctx->text == NULL || quality == NULL
            || *quality > shape_line_ctx->text->num_chars) {
        return NULL;
    }

    uint32_t *s = shape_line_ctx->text->memory;
    uint32_t *e = shape_line_ctx->text->memory + shape_line_ctx->text->num_chars;
    prefer_left = allow_left ? prefer_left : 0;
    size_t reduction_steps = shape_line_ctx->text->num_chars - *quality + 1;
    for (size_t i = 0; i < reduction_steps; i++) {
        if (prefer_left) {
            if (s < e && is_blank(*s)) {
                s++;
            }
            else if (e > s && allow_right && is_blank(*(e - 1))) {
                e--;
            }
            else {
                break;
            }
        }
        else {
            if (e > s && allow_right && is_blank(*(e - 1))) {
                e--;
            }
            else if (s < e && allow_left && is_blank(*s)) {
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



static int hmm_shiftable(shape_line_ctx_t *shapes_relevant, uint32_t *cur_pos, size_t shape_idx, uint32_t *end_pos,
        int anchored_right);



/**
 * (horizontal middle match)
 * Recursive helper function for match_horiz_line(), uses backtracking.
 * @param shapes_relevant the prepared shape lines to be concatenated
 * @param cur_pos current position in the input line being matched
 * @param shape_idx index into `shapes_relevant` indicating which shape to try now
 * @param end_pos first character of the east corner
 * @param anchored_left flag indicating that `cur_pos` is already "anchored" or still "shiftable". "Anchored" means
 *      that we have matched a non-blank shape line already (corner shape line was not blank). Else "shiftable".
 * @param anchored_right flag indicating that the east corner shape was not blank. If this is `false`, it means that
 *      a shape may be shortened right if only blank shape lines follow.
 * @return `== 1`: success;
 *         `== 0`: failed to match
 */
int hmm(shape_line_ctx_t *shapes_relevant, uint32_t *cur_pos, size_t shape_idx, uint32_t *end_pos, int anchored_left,
        int anchored_right)
{
    if (is_debug_logging(MAIN)) {
        char *out_cur_pos = u32_strconv_to_output(cur_pos);
        char *out_end_pos = u32_strconv_to_output(end_pos);
        log_debug(__FILE__, MAIN, "hmm(shapes_relevant, \"%s\", %d, \"%s\", %s, %s) - enter\n", out_cur_pos,
                (int) shape_idx, out_end_pos, anchored_left ? "true" : "false", anchored_right ? "true" : "false");
        BFREE(out_cur_pos);
        BFREE(out_end_pos);
    }

    int result = 0;
    if (!anchored_left) {
        result = hmm_shiftable(shapes_relevant, cur_pos, shape_idx, end_pos, anchored_right);
    }
    else if (cur_pos > end_pos) {
        /* invalid input */
        result = 0;
    }
    else if (cur_pos == end_pos) {
        /* we are at the end, which is fine if there is nothing else to match */
        result = (shape_idx == (SHAPES_PER_SIDE - 1) && anchored_right)
                || ((shapes_relevant[shape_idx].empty || bxs_is_blank(shapes_relevant[shape_idx].text))
                    && !non_empty_shapes_after(shapes_relevant, shape_idx) ? 1 : 0);
    }
    else if (shape_idx >= SHAPES_PER_SIDE - 1) {
        /* no more shapes to try, which is fine if the rest of the line is blank */
        result = u32_is_blank(cur_pos);
    }
    else if (shapes_relevant[shape_idx].empty) {
        /* the current shape line is empty, try the next one */
        result = hmm(shapes_relevant, cur_pos, shape_idx + 1, end_pos, 1, anchored_right);
    }
    else {
        uint32_t *shape_line = u32_strdup(shapes_relevant[shape_idx].text->memory);
        size_t quality = shapes_relevant[shape_idx].text->num_chars;
        while (shape_line != NULL && quality > 0) {
            if (u32_strncmp(cur_pos, shape_line, quality) == 0) {
                BFREE(shape_line);
                cur_pos = cur_pos + quality;
                if (cur_pos == end_pos && !non_empty_shapes_after(shapes_relevant, shape_idx)) {
                    result = 1; /* success */
                }
                else {
                    int rc = 0;
                    if (shapes_relevant[shape_idx].elastic) {
                        rc = hmm(shapes_relevant, cur_pos, shape_idx, end_pos, 1, anchored_right);
                    }
                    if (rc == 0) {
                        result = hmm(shapes_relevant, cur_pos, shape_idx + 1, end_pos, 1, anchored_right);
                    }
                    else {
                        result = rc;
                    }
                }
            }
            else if (!anchored_right) {
                shape_line = shorten(shapes_relevant + shape_idx, &quality, 0, 0, 1);
                if (is_debug_logging(MAIN)) {
                    char *out_shape_line = u32_strconv_to_output(shape_line);
                    log_debug(__FILE__, MAIN, "hmm() - shape_line shortened to %d (\"%s\")\n",
                            (int) quality, out_shape_line);
                    BFREE(out_shape_line);
                }
            }
            else {
                BFREE(shape_line);
            }
        }
    }

    log_debug(__FILE__, MAIN, "hmm() - exit, result = %d\n", result);
    return result;
}



static int hmm_shiftable(shape_line_ctx_t *shapes_relevant, uint32_t *cur_pos, size_t shape_idx, uint32_t *end_pos,
        int anchored_right)
{
    int result = 0;
    int shapes_are_empty = 1;
    for (size_t i = shape_idx; i < SHAPES_PER_SIDE - 1; i++) {
        if (!is_shape_line_empty(shapes_relevant, i)) {
            shapes_are_empty = 0;
            int can_shorten_right = -1;
            size_t quality = shapes_relevant[i].text->num_chars;
            uint32_t *shape_line = shapes_relevant[i].text->memory;
            while (shape_line != NULL) {
                uint32_t *p = u32_strstr(cur_pos, shape_line);
                if (p != NULL && p < end_pos && is_blank_between(cur_pos, p)) {
                    result = hmm(shapes_relevant, p + quality, i + (shapes_relevant[i].elastic ? 0 : 1),
                            end_pos, 1, anchored_right);
                    if (result == 0 && shapes_relevant[i].elastic) {
                        result = hmm(shapes_relevant, p + quality, i + 1, end_pos, 1, anchored_right);
                    }
                    break;
                }
                if (can_shorten_right == -1) {
                    /* we can only shorten right if the east corner shape line is also empty */
                    can_shorten_right = non_empty_shapes_after(shapes_relevant, i)
                            || !is_shape_line_empty(shapes_relevant, SHAPES_PER_SIDE - 1) ? 0 : 1;
                }
                shape_line = shorten(shapes_relevant + i, &quality, 0, 1, can_shorten_right);
            }
            break;
        }
    }
    if (shapes_are_empty) {
        /* all shapes were empty, which is fine if line was blank */
        result = is_blank_between(cur_pos, end_pos);
    }
    return result;
}



static shape_line_ctx_t *prepare_comp_shapes_horiz(int hside, comparison_t comp_type, size_t shape_line_idx)
{
    shape_t *side_shapes = hside == BTOP ? north_side : south_side_rev;
    shape_line_ctx_t *shapes_relevant = (shape_line_ctx_t *) calloc(SHAPES_PER_SIDE, sizeof(shape_line_ctx_t));

    for (size_t i = 0; i < SHAPES_PER_SIDE; i++) {
        shapes_relevant[i].elastic = opt.design->shape[side_shapes[i]].elastic;
        shapes_relevant[i].empty = isempty(opt.design->shape + side_shapes[i]);
        if (!shapes_relevant[i].empty) {
            uint32_t *s = prepare_comp_shape(opt.design, side_shapes[i], shape_line_idx, comp_type, 0,
                    i == SHAPES_PER_SIDE - 1);
            shapes_relevant[i].text = bxs_from_unicode(s);
            BFREE(s);
        }
    }

    return shapes_relevant;
}



static match_result_t *new_match_result(uint32_t *p, size_t p_idx, size_t len, int shiftable)
{
    match_result_t *result = (match_result_t *) calloc(1, sizeof(match_result_t));
    result->p = p;
    result->p_idx = p_idx;
    result->len = len;
    result->shiftable = shiftable;
    return result;
}



/**
 * Match a `shape_line` at the beginning (`vside` == `BLEF`) or the end (`vside` == `BRIG`) of an `input_line`.
 * Both `input_line` and `shape_line` may contain invisible characters, who are then matched, too, just like any other
 * characters.
 * @param vside BLEF or BRIG
 * @param input_line the input line to examine. We expect that it was NOT trimmed.
 * @param shape_line the shape line to match, also NOT trimmed
 * @return pointer to the match result (in existing memory of `input_line->memory`), or `NULL` if no match
 */
match_result_t *match_outer_shape(int vside, bxstr_t *input_line, bxstr_t *shape_line)
{
    if (input_line == NULL || input_line->num_chars == 0 || shape_line == NULL || shape_line->num_chars == 0) {
        return NULL;
    }

    if (vside == BLEF) {
        if (bxs_is_blank(shape_line)) {
            return new_match_result(input_line->memory, 0, 0, 1);
        }
        for (uint32_t *s = shape_line->memory; s == shape_line->memory || is_blank(*s); s++) {
            uint32_t *p = u32_strstr(input_line->memory, s);
            size_t p_idx = p != NULL ? p - input_line->memory : 0;
            if (p == NULL || p_idx > input_line->first_char[input_line->indent]) {
                continue;  /* not found or found too far in */
            }
            return new_match_result(p, p_idx, shape_line->num_chars - (s - shape_line->memory), 0);
        }
    }
    else {
        if (bxs_is_blank(shape_line)) {
            uint32_t *p = bxs_last_char_ptr(input_line);
            size_t p_idx = p - input_line->memory;
            return new_match_result(p, p_idx, 0, 1);
        }
        int slen = shape_line->num_chars;
        uint32_t *s = u32_strdup(shape_line->memory);
        for (; slen == (int) shape_line->num_chars || is_blank(s[slen]); slen--) {
            s[slen] = char_nul;
            uint32_t *p = u32_strnrstr(input_line->memory, s, slen);
            size_t p_idx = p != NULL ? p - input_line->memory : 0;
            if (p == NULL || p_idx + slen
                    < input_line->first_char[input_line->num_chars_visible - input_line->trailing]) {
                continue; /* not found or found too far in */
            }
            BFREE(s);
            return new_match_result(p, p_idx, (size_t) slen, 0);
        }
        BFREE(s);
    }
    return NULL;
}



static int match_horiz_line(remove_ctx_t *ctx, int hside, size_t input_line_idx, size_t shape_line_idx)
{
    log_debug(__FILE__, MAIN, "match_horiz_line(ctx, %s, %d, %d)\n",
                hside == BTOP ? "BTOP" : "BBOT", (int) input_line_idx, (int) shape_line_idx);

    int result = 0;
    for (comparison_t comp_type = 0; comp_type < NUM_COMPARISON_TYPES; comp_type++) {
        if (!comp_type_is_viable(comp_type, ctx->input_is_mono, ctx->design_is_mono)) {
            continue;
        }
        ctx->comp_type = comp_type;
        log_debug(__FILE__, MAIN, "  Setting comparison type to: %s\n", comparison_name[comp_type]);

        shape_line_ctx_t *shapes_relevant = prepare_comp_shapes_horiz(hside, comp_type, shape_line_idx);
        debug_print_shapes_relevant(shapes_relevant);

        bxstr_t *input_prepped1 = bxs_from_unicode(prepare_comp_input(input_line_idx, 0, comp_type, 0, NULL, NULL));
        bxstr_t *input_prepped = bxs_rtrim(input_prepped1);
        bxs_append_spaces(input_prepped, opt.design->shape[NW].width + opt.design->shape[NE].width);
        bxs_free(input_prepped1);
        if (is_debug_logging(MAIN)) {
            char *out_input_prepped = bxs_to_output(input_prepped);
            log_debug(__FILE__, MAIN, "  input_prepped = \"%s\"\n", out_input_prepped);
            BFREE(out_input_prepped);
        }

        uint32_t *cur_pos = input_prepped->memory;
        match_result_t *mrl = NULL;
        if (!ctx->empty_side[BLEF]) {
            mrl = match_outer_shape(BLEF, input_prepped, shapes_relevant[0].text);
            if (mrl != NULL) {
                cur_pos = mrl->p + mrl->len;
            }
        }

        uint32_t *end_pos = bxs_last_char_ptr(input_prepped);
        match_result_t *mrr = NULL;
        if (!ctx->empty_side[BRIG]) {
            mrr = match_outer_shape(BRIG, input_prepped, shapes_relevant[SHAPES_PER_SIDE - 1].text);
            if (mrr != NULL) {
                end_pos = mrr->p;
            }
        }
        if (is_debug_logging(MAIN)) {
            char *out_cur_pos = u32_strconv_to_output(cur_pos);
            char *out_end_pos = u32_strconv_to_output(end_pos);
            log_debug(__FILE__, MAIN, "  cur_pos = \"%s\" (index %d)\n",
                    out_cur_pos, (int) BMAX(cur_pos - input_prepped->memory, 0));
            log_debug(__FILE__, MAIN, "  end_pos = \"%s\" (index %d)\n",
                    out_end_pos, (int) BMAX(end_pos - input_prepped->memory, 0));
            BFREE(out_cur_pos);
            BFREE(out_end_pos);
        }

        result = hmm(shapes_relevant, cur_pos, 1, end_pos, (mrl == NULL) || mrl->shiftable ? 0 : 1,
                (mrr == NULL) || mrr->shiftable ? 0 : 1);

        BFREE(mrl);
        BFREE(mrr);
        for (size_t i = 0; i < SHAPES_PER_SIDE; i++) {
            bxs_free(shapes_relevant[i].text);
        }
        BFREE(shapes_relevant);

        if (result) {
            log_debug(__FILE__, MAIN, "Matched %s side line using comp_type=%s and shape_line_idx=%d\n",
                    hside == BTOP ? "top" : "bottom", comparison_name[comp_type], (int) shape_line_idx);
            break;
        }
    }

    return result;
}



static size_t find_top_side(remove_ctx_t *ctx)
{
    size_t result = ctx->top_start_idx;
    sentry_t *shapes = opt.design->shape;
    for (size_t input_line_idx = ctx->top_start_idx;
            input_line_idx < input.num_lines && input_line_idx < ctx->top_start_idx + shapes[NE].height;
            input_line_idx++)
    {
        int matched = 0;
        size_t shape_lines_tested = 0;
        for (size_t shape_line_idx = (input_line_idx - ctx->top_start_idx) % shapes[NE].height;
                shape_lines_tested < shapes[NE].height;
                shape_line_idx = (shape_line_idx + 1) % shapes[NE].height, shape_lines_tested++)
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
    size_t result = ctx->bottom_end_idx;
    sentry_t *shapes = opt.design->shape;
    for (long input_line_idx = (long) ctx->bottom_end_idx - 1;
            input_line_idx >= 0 && input_line_idx >= (long) ctx->bottom_end_idx - (long) shapes[SE].height;
            input_line_idx--)
    {
        int matched = 0;
        size_t shape_lines_tested = 0;
        for (long shape_line_idx = shapes[SE].height - (ctx->bottom_end_idx - input_line_idx);
                shape_line_idx >= 0 && shape_lines_tested < shapes[SE].height;
                shape_lines_tested++,
                shape_line_idx = shape_line_idx == 0 ? (long) (shapes[SE].height - 1) : (long) (shape_line_idx - 1))
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
    shape_t west_side_shapes[SHAPES_PER_SIDE - CORNERS_PER_SIDE] = {WNW, W, WSW};
    shape_t east_side_shapes[SHAPES_PER_SIDE - CORNERS_PER_SIDE] = {ENE, E, ESE};
    shape_t side_shapes[SHAPES_PER_SIDE - CORNERS_PER_SIDE];
    if (vside == BLEF) {
        memcpy(side_shapes, west_side_shapes, (SHAPES_PER_SIDE - CORNERS_PER_SIDE) * sizeof(shape_t));
    }
    else {
        memcpy(side_shapes, east_side_shapes, (SHAPES_PER_SIDE - CORNERS_PER_SIDE) * sizeof(shape_t));
    }

    size_t num_shape_lines = count_shape_lines(side_shapes);

    shape_line_ctx_t **shape_lines = (shape_line_ctx_t **) calloc(num_shape_lines + 1, sizeof(shape_line_ctx_t *));
    for (size_t i = 0; i < num_shape_lines; i++) {
        shape_lines[i] = (shape_line_ctx_t *) calloc(1, sizeof(shape_line_ctx_t));
    }

    for (size_t shape_idx = 0, i = 0; shape_idx < SHAPES_PER_SIDE - CORNERS_PER_SIDE; shape_idx++) {
        if (!isempty(opt.design->shape + side_shapes[shape_idx])) {
            int deep_empty = isdeepempty(opt.design->shape + side_shapes[shape_idx]);
            for (size_t slno = 0; slno < opt.design->shape[side_shapes[shape_idx]].height; slno++, i++) {
                uint32_t *s = prepare_comp_shape(opt.design, side_shapes[shape_idx], slno, comp_type, 0, 0);
                shape_lines[i]->text = bxs_from_unicode(s);
                shape_lines[i]->empty = deep_empty;
                shape_lines[i]->elastic = opt.design->shape[side_shapes[shape_idx]].elastic;
                BFREE(s);
            }
        }
    }

    return shape_lines;
}



static void free_shape_lines(shape_line_ctx_t **shape_lines)
{
    if (shape_lines != NULL) {
        for (shape_line_ctx_t **p = shape_lines; *p != NULL; p++) {
            bxs_free((*p)->text);
            BFREE(*p);
        }
        BFREE(shape_lines);
    }
}



static void match_vertical_side(remove_ctx_t *ctx, int vside, shape_line_ctx_t **shape_lines, uint32_t *input_line,
    size_t line_idx, size_t input_length, size_t input_indent, size_t input_trailing)
{
    line_ctx_t *line_ctx = ctx->body + (line_idx - ctx->top_end_idx);

    for (shape_line_ctx_t **shape_line_ctx = shape_lines; *shape_line_ctx != NULL; shape_line_ctx++) {
        if ((*shape_line_ctx)->empty) {
            continue;
        }

        size_t max_quality = (*shape_line_ctx)->text->num_chars;
        size_t quality = max_quality;
        uint32_t *shape_text = (*shape_line_ctx)->text->memory;
        uint32_t *to_free = NULL;
        while(shape_text != NULL) {
            uint32_t *p;
            if (vside == BLEF) {
                p = u32_strstr(input_line, shape_text);
            }
            else {
                p = u32_strnrstr(input_line, shape_text, quality);
            }
            BFREE(to_free);
            shape_text = NULL;

            if ((p == NULL)
                    || (vside == BLEF && ((size_t) (p - input_line) > input_indent + (max_quality - quality)))
                    || (vside == BRIG && ((size_t) (p - input_line) < input_length - input_trailing - quality))) {
                shape_text = shorten(*shape_line_ctx, &quality, vside == BLEF, 1, 1);
                to_free = shape_text;
            }
            else if (vside == BLEF) {
                if (quality > line_ctx->west_quality) {
                    line_ctx->west_start = (size_t) (p - input_line);
                    line_ctx->west_end = line_ctx->west_start + quality;
                    line_ctx->west_quality = quality;
                    BFREE(line_ctx->input_line_used);
                    line_ctx->input_line_used = u32_strdup(input_line);
                    break;
                }
            }
            else if (vside == BRIG) {
                if (quality > line_ctx->east_quality) {
                    line_ctx->east_start = (size_t) (p - input_line);
                    line_ctx->east_end = line_ctx->east_start + quality;
                    line_ctx->east_quality = quality;
                    BFREE(line_ctx->input_line_used);
                    line_ctx->input_line_used = u32_strdup(input_line);
                    break;
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

    size_t max_quality = 0;
    if (!ctx->empty_side[BLEF]) {
        max_quality += opt.design->shape[NW].width;
    }
    if (!ctx->empty_side[BRIG]) {
        max_quality += opt.design->shape[NE].width;
    }
    max_quality = max_quality * num_body_lines;

    /* If we manage to match 50%, then it is unlikely to improve with a different comparison mode. */
    int sufficient = (max_quality == 0 && total_quality == 0)
            || (max_quality > 0 && (total_quality > 0.5 * max_quality));
    log_debug(__FILE__, MAIN, "sufficient_body_quality() found body match quality of %d/%d (%s).\n",
            (int) total_quality, (int) max_quality, sufficient ? "sufficient" : "NOT sufficient");

    return sufficient;
}



static void reset_body(remove_ctx_t *ctx)
{
    if (ctx->body != NULL) {
        for (size_t i = 0; i < ctx->body_num_lines; i++) {
            BFREE(ctx->body[i].input_line_used);
        }
        memset(ctx->body, 0, ctx->body_num_lines * sizeof(line_ctx_t));
    }
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
        ctx->comp_type = comp_type;
        log_debug(__FILE__, MAIN, "find_vertical_shapes(): comp_type = %s\n", comparison_name[comp_type]);
        reset_body(ctx);

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
            log_debug(__FILE__, MAIN, "Design autodetection: Removing box of design \"%s\".\n", opt.design->name);
        }
        else {
            fprintf(stderr, "%s: Box design autodetection failed. Use -d option.\n", PROJECT);
            exit(EXIT_FAILURE);
        }
    }
    else {
        log_debug(__FILE__, MAIN, "Design was chosen by user: %s\n", opt.design->name);
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
    size_t lines_removed = 0;
    size_t max_lines_removable = opt.mend && !opt.killblank ? (size_t) BMAX(opt.design->padding[BTOP], 0) : SIZE_MAX;
    while (ctx->top_end_idx < ctx->bottom_start_idx && lines_removed < max_lines_removable
            && empty_line(input.lines + ctx->top_end_idx))
    {
        log_debug(__FILE__, MAIN, "Killing leading blank line in box body.\n");
        ++(ctx->top_end_idx);
        --(ctx->body_num_lines);
        ++lines_removed;
    }

    lines_removed = 0;
    max_lines_removable = opt.mend && !opt.killblank ? (size_t) BMAX(opt.design->padding[BBOT], 0) : SIZE_MAX;
    while (ctx->bottom_start_idx > ctx->top_end_idx && lines_removed < max_lines_removable
            && empty_line(input.lines + ctx->bottom_start_idx - 1))
    {
        log_debug(__FILE__, MAIN, "Killing trailing blank line in box body.\n");
        --(ctx->bottom_start_idx);
        --(ctx->body_num_lines);
        ++lines_removed;
    }
}



static int org_is_not_blank(bxstr_t *org_line, comparison_t comp_type, size_t idx)
{
    if (comp_type == literal || comp_type == ignore_invisible_shape) {
        return !is_blank(org_line->memory[idx]);
    }
    return !is_blank(org_line->memory[org_line->visible_char[idx]]);
}



static size_t max_chars_line(bxstr_t *org_line, comparison_t comp_type)
{
    return (comp_type == literal || comp_type == ignore_invisible_shape)
            ? org_line->num_chars : org_line->num_chars_visible;
}



static size_t confirmed_padding(bxstr_t *org_line, comparison_t comp_type, size_t start_idx, size_t num_padding)
{
    size_t result = 0;
    size_t max_chars = max_chars_line(org_line, comp_type);
    for (size_t i = start_idx; i < BMIN(max_chars, start_idx + num_padding); i++) {
        if (org_is_not_blank(org_line, comp_type, i)) {
            break;
        }
        result++;
    }
    return result;
}



static void remove_top_from_input(remove_ctx_t *ctx)
{
    if (ctx->top_end_idx > ctx->top_start_idx) {
        for (size_t j = ctx->top_start_idx; j < ctx->top_end_idx; ++j) {
            free_line(input.lines + j);
        }
        memmove(input.lines + ctx->top_start_idx, input.lines + ctx->top_end_idx,
                (input.num_lines - ctx->top_end_idx) * sizeof(line_t));
        input.num_lines -= ctx->top_end_idx - ctx->top_start_idx;
    }
}



static size_t calculate_start_idx(remove_ctx_t *ctx, size_t body_line_idx)
{
    size_t input_line_idx = ctx->top_end_idx + body_line_idx;
    line_ctx_t *lctx = ctx->body + body_line_idx;
    bxstr_t *org_line = input.lines[input_line_idx].text;

    size_t s_idx = 0;
    if (lctx->west_quality > 0) {
        s_idx = lctx->west_end + confirmed_padding(org_line, ctx->comp_type, lctx->west_end, opt.design->padding[BLEF]);
    }
    if (ctx->comp_type == ignore_invisible_input || ctx->comp_type == ignore_invisible_all) {
        /* our line context worked with visible characters only, convert back to org_line */
        s_idx = org_line->first_char[s_idx];
    }
    return s_idx;
}



static size_t calculate_end_idx(remove_ctx_t *ctx, size_t body_line_idx)
{
    size_t input_line_idx = ctx->top_end_idx + body_line_idx;
    line_ctx_t *lctx = ctx->body + body_line_idx;
    bxstr_t *org_line = input.lines[input_line_idx].text;

    size_t e_idx = lctx->east_quality > 0 ? lctx->east_start : max_chars_line(org_line, ctx->comp_type);
    if (ctx->comp_type == ignore_invisible_input || ctx->comp_type == ignore_invisible_all) {
        e_idx = org_line->first_char[e_idx];
    }
    return e_idx;
}



static void remove_vertical_from_input(remove_ctx_t *ctx)
{
    for (size_t body_line_idx = 0; body_line_idx < ctx->body_num_lines; body_line_idx++) {
        size_t input_line_idx = ctx->top_end_idx + body_line_idx;
        bxstr_t *org_line = input.lines[input_line_idx].text;
        size_t s_idx = calculate_start_idx(ctx, body_line_idx);
        size_t e_idx = calculate_end_idx(ctx, body_line_idx);
        log_debug(__FILE__, MAIN, "remove_vertical_from_input(): body_line_idx=%d, input_line_idx=%d, s_idx=%d, "
                "e_idx=%d, input.indent=%d\n", (int) body_line_idx, (int) input_line_idx, (int) s_idx, (int) e_idx,
                (int) input.indent);

        bxstr_t *temp2 = bxs_substr(org_line, s_idx, e_idx);
        if (opt.indentmode == 'b' || opt.indentmode == '\0') {
            /* restore indentation */
            bxstr_t *temp = bxs_prepend_spaces(temp2, input.indent);
            free_line_text(input.lines + input_line_idx);
            input.lines[input_line_idx].text = temp;
            bxs_free(temp2);
        }
        else {
            /* remove indentation */
            free_line_text(input.lines + input_line_idx);
            input.lines[input_line_idx].text = temp2;
        }
    }
}



static void remove_bottom_from_input(remove_ctx_t *ctx)
{
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
}



static void remove_default_padding(remove_ctx_t *ctx, int num_blanks)
{
    if (num_blanks > 0) {
        for (size_t body_line_idx = 0; body_line_idx < ctx->body_num_lines; body_line_idx++) {
            size_t input_line_idx = ctx->top_start_idx + body_line_idx; /* top_start_idx, because top was removed! */
            bxstr_t *temp = bxs_cut_front(input.lines[input_line_idx].text, (size_t) num_blanks);
            free_line_text(input.lines + input_line_idx);
            input.lines[input_line_idx].text = temp;
        }
        input.indent -= (size_t) num_blanks;
        input.maxline -= (size_t) num_blanks;
    }
}



static void apply_results_to_input(remove_ctx_t *ctx)
{
    remove_vertical_from_input(ctx);

    if (opt.killblank || opt.mend) {
        killblank(ctx);
    }
    remove_bottom_from_input(ctx);
    remove_top_from_input(ctx);

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
    if (ctx->empty_side[BLEF]) {
        /* If the side were not open, default padding would have been removed when the side was removed. */
        remove_default_padding(ctx, BMIN((int) input.indent, opt.design->padding[BLEF]));
    }

    size_t num_lines_removed = BMAX(ctx->top_end_idx - ctx->top_start_idx, (size_t) 0)
            + BMAX(ctx->bottom_end_idx - ctx->bottom_start_idx, (size_t) 0);
    memset(input.lines + input.num_lines, 0, num_lines_removed * sizeof(line_t));

    if (is_debug_logging(MAIN)) {
        print_input_lines(" (remove_box) after box removal");
        log_debug(__FILE__, MAIN, "Number of lines shrunk by %d.\n", (int) num_lines_removed);
    }
}



int remove_box()
{
    detect_design_if_needed();

    remove_ctx_t *ctx = (remove_ctx_t *) calloc(1, sizeof(remove_ctx_t));
    ctx->empty_side[BTOP] = empty_side(opt.design->shape, BTOP);
    ctx->empty_side[BRIG] = empty_side(opt.design->shape, BRIG);
    ctx->empty_side[BBOT] = empty_side(opt.design->shape, BBOT);
    ctx->empty_side[BLEF] = empty_side(opt.design->shape, BLEF);
    log_debug(__FILE__, MAIN, "Empty sides? Top: %d, Right: %d, Bottom: %d, Left: %d\n",
            ctx->empty_side[BTOP], ctx->empty_side[BRIG], ctx->empty_side[BBOT], ctx->empty_side[BLEF]);

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
    log_debug(__FILE__, MAIN, "ctx->top_start_idx = %d, ctx->top_end_idx = %d\n", (int) ctx->top_start_idx,
            (int) ctx->top_end_idx);

    ctx->bottom_end_idx = find_last_line() + 1;
    if (ctx->empty_side[BBOT]) {
        ctx->bottom_start_idx = ctx->bottom_end_idx;
    }
    else {
        ctx->bottom_start_idx = find_bottom_side(ctx);
    }
    log_debug(__FILE__, MAIN, "ctx->bottom_start_idx = %d, ctx->bottom_end_idx = %d\n", (int) ctx->bottom_start_idx,
            (int) ctx->bottom_end_idx);
    if (ctx->bottom_start_idx > ctx->top_end_idx) {
        ctx->body_num_lines = ctx->bottom_start_idx - ctx->top_end_idx;
    }

    if (ctx->body_num_lines > 0) {
        ctx->body = (line_ctx_t *) calloc(ctx->body_num_lines, sizeof(line_ctx_t));
        find_vertical_shapes(ctx);
    }

    debug_print_remove_ctx(ctx, "before apply_results_to_input()");
    apply_results_to_input(ctx);

    if (ctx->body != NULL) {
        for (size_t i = 0; i < ctx->body_num_lines; i++) {
            BFREE(ctx->body[i].input_line_used);
        }
        BFREE(ctx->body);
    }
    BFREE(ctx);
    return 0;
}



void output_input(const int trim_only)
{
    size_t indent;
    int ntabs, nspcs;

    log_debug(__FILE__, MAIN, "output_input() - enter (trim_only=%d)\n", trim_only);

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
