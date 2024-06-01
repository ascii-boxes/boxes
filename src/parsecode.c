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
 * C code used exclusively by the parser.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unictype.h>
#include <unistr.h>

#include "discovery.h"
#include "logging.h"
#include "parsecode.h"
#include "parsing.h"
#include "query.h"
#include "regulex.h"
#include "shape.h"
#include "tools.h"
#include "unicode.h"

#include "parser.h"

#include "lex.yy.h"


static pcre2_code *eol_pattern = NULL;



static int check_sizes(pass_to_bison *bison_args)
/*
 *  For the author's convenience, it is required that shapes on one side
 *  have equal width (vertical sides) and height (horizontal sides).
 *
 *  RETURNS:  == 0   no problem detected
 *            != 0   on error (prints error message, too)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i, j, k;

    log_debug(__FILE__, PARSER, " Parser: check_sizes()\n");

    for (i = 0; i < NUM_SIDES; ++i) {
        if (i == 0 || i == 2) {
            /*
             *  horizontal
             */
            for (j = 0; j < SHAPES_PER_SIDE - 1; ++j) {
                if (curdes.shape[sides[i][j]].height == 0) {
                    continue;
                }
                for (k = j + 1; k < SHAPES_PER_SIDE; ++k) {
                    if (curdes.shape[sides[i][k]].height == 0) {
                        continue;
                    }
                    if (curdes.shape[sides[i][j]].height != curdes.shape[sides[i][k]].height) {
                        yyerror(bison_args, "All shapes on horizontal sides must be of "
                                "equal height (%s: %d, %s: %d)\n",
                                shape_name[sides[i][j]], curdes.shape[sides[i][j]].height,
                                shape_name[sides[i][k]], curdes.shape[sides[i][k]].height);
                        return 1;
                    }
                }
            }
        }
        else {
            /*
             *  vertical
             */
            for (j = 0; j < SHAPES_PER_SIDE - 1; ++j) {
                if (curdes.shape[sides[i][j]].width == 0) {
                    continue;
                }
                for (k = j + 1; k < SHAPES_PER_SIDE; ++k) {
                    if (curdes.shape[sides[i][k]].width == 0) {
                        continue;
                    }
                    if (curdes.shape[sides[i][j]].width != curdes.shape[sides[i][k]].width) {
                        yyerror(bison_args, "All shapes on vertical sides must be of "
                                "equal width (%s: %d, %s: %d)\n",
                                shape_name[sides[i][j]], curdes.shape[sides[i][j]].width,
                                shape_name[sides[i][k]], curdes.shape[sides[i][k]].width);
                        return 1;
                    }
                }
            }
        }
    }

    return 0; /* all clear */
}



static int corner_check(pass_to_bison *bison_args)
/*
 *  Check that no corners are elastic.
 *
 *  RETURNS:  == 0   no problem detected
 *            != 0   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int c;

    log_debug(__FILE__, PARSER, " Parser: corner_check()\n");

    for (c = 0; c < NUM_CORNERS; ++c) {
        if (curdes.shape[corners[c]].elastic) {
            yyerror(bison_args, "Corners may not be elastic (%s)", shape_name[corners[c]]);
            return 1;
        }
    }

    return 0; /* all clear */
}



static shape_t non_existent_elastics(pass_to_bison *bison_args)
{
    shape_t i;

    log_debug(__FILE__, PARSER, " Parser: non_existent_elastics()\n");

    for (i = 0; i < NUM_SHAPES; ++i) {
        if (curdes.shape[i].elastic && isempty(curdes.shape + i)) {
            return i;
        }
    }

    return (shape_t) NUM_SHAPES; /* all elastic shapes exist */
}



static int insufficient_elasticity(pass_to_bison *bison_args)
{
    int i, j, ef;

    log_debug(__FILE__, PARSER, " Parser: insufficient_elasticity()\n");

    for (i = 0; i < NUM_SIDES; ++i) {
        for (j = 1, ef = 0; j < 4; ++j) {
            if (curdes.shape[sides[i][j]].elastic) {
                ++ef;
            }
        }
        if (ef != 1 && ef != 2) {
            return 1; /* error */
        }
    }

    return 0; /* all clear */
}



static int adjoining_elastics(pass_to_bison *bison_args)
{
    int i, j, ef;

    log_debug(__FILE__, PARSER, " Parser: adjoining_elastics()\n");

    for (i = 0; i < NUM_SIDES; ++i) {
        ef = 0;
        for (j = 1; j < 4; ++j) {
            if (isempty(curdes.shape + sides[i][j])) {
                continue;
            }
            if (curdes.shape[sides[i][j]].elastic) {
                if (ef) {
                    return 1; /* error detected */
                }
                else {
                    ef = 1;
                }
            }
            else {
                ef = 0;
            }
        }
    }

    return 0; /* all clear */
}



int perform_se_check(pass_to_bison *bison_args)
{
    shape_t s_rc;

    s_rc = non_existent_elastics(bison_args);
    if (s_rc != NUM_SHAPES) {
        yyerror(bison_args, "Shape %s has been specified as elastic but doesn't exist", shape_name[s_rc]);
        return 1;
    }

    if (corner_check(bison_args)) {
        /* Error message printed in check func */
        return 1;
    }

    if (insufficient_elasticity(bison_args)) {
        yyerror(bison_args, "There must be exactly one or two elastic shapes per side");
        return 1;
    }

    if (adjoining_elastics(bison_args)) {
        yyerror(bison_args, "Two adjoining shapes may not be elastic");
        return 1;
    }

    return 0;
}



static void init_design(pass_to_bison *bison_args, design_t *design)
{
    memset(design, 0, sizeof(design_t));
    design->aliases = (char **) calloc(1, sizeof(char *));
    design->indentmode = DEF_INDENTMODE;
    design->defined_in = bison_args->config_file;
    design->tags = (char **) calloc(1, sizeof(char *));
}



/**
 * Reset parser to neutral state, so a new design can be parsed.
 * @param bison_args the parser state
 */
void recover(pass_to_bison *bison_args)
{
    bison_args->num_mandatory = 0;
    bison_args->time_for_se_check = 0;
    bison_args->num_shapespec = 0;

    /*
     *  Clear current design
     */
    BFREE(curdes.name);
    BFREE(curdes.author);
    BFREE(curdes.aliases);
    BFREE(curdes.designer);
    BFREE(curdes.sample);
    BFREE(curdes.tags);
    init_design(bison_args, &(curdes));
}



static int design_has_alias(design_t *design, char *alias)
{
    int result = 0;
    for (size_t aidx = 0; design->aliases[aidx] != NULL; ++aidx) {
        if (strcasecmp(alias, design->aliases[aidx]) == 0) {
            result = 1;
            break;
        }
    }
    return result;
}



static int design_has_name(design_t *design, char *name)
{
    int result = 0;
    if (strcasecmp(name, design->name) == 0) {
        result = 1;
    }
    else {
        result = design_has_alias(design, name);
    }
    return result;
}



static int full_parse_required()
{
    int result = 0;
    if (!opt.design_choice_by_user) {
        result = opt.r || opt.l || (opt.query != NULL && !opt.qundoc);
    }
    log_debug(__FILE__, MAIN, " Parser: full_parse_required() -> %s\n", result ? "true" : "false");
    return result;
}



/**
 * Determine if the design currently being parsed is one that we will need.
 * @param bison_args the bison state
 * @return result flag
 */
static int design_needed(pass_to_bison *bison_args)
{
    if (opt.design_choice_by_user) {
        return design_has_name(&(curdes), (char *) opt.design);
    }
    else {
        if (full_parse_required() || bison_args->design_idx == 0) {
            return 1;
        }
    }
    return 0;
}



static int design_name_exists(pass_to_bison *bison_args, char *name)
{
    int result = 0;
    for (int i = 0; i < bison_args->design_idx; ++i) {
        if (design_has_name(bison_args->designs + i, name)) {
            result = 1;
            break;
        }
    }
    return result;
}



static int alias_exists_in_child_configs(pass_to_bison *bison_args, char *alias)
{
    int result = 0;
    for (size_t i = 0; i < bison_args->num_child_configs; ++i) {
        if (design_has_alias(bison_args->child_configs + i, alias)) {
            result = 1;
            break;
        }
    }
    return result;
}



static int tag_add(pass_to_bison *bison_args, bxstr_t *tag)
{
    int rc = RC_SUCCESS;
    if (is_ascii_id(tag, 1)) {
        if (!array_contains0(curdes.tags, tag->ascii)) {
            size_t num_tags = array_count0(curdes.tags);
            curdes.tags = (char **) realloc(curdes.tags, (num_tags + 2) * sizeof(char *));
            curdes.tags[num_tags] = (char *) strdup(tag->ascii);
            curdes.tags[num_tags + 1] = NULL;
        }
        else {
            yyerror(bison_args, "duplicate tag -- %s", bxs_to_output(tag));
            rc = RC_ERROR;
        }
    }
    else {
        yyerror(bison_args, "invalid tag -- %s", bxs_to_output(tag));
        rc = RC_ERROR;
    }
    return rc;
}



static int tag_split_add(pass_to_bison *bison_args, bxstr_t *tag)
{
    int rc = RC_SUCCESS;
    uint32_t *c = NULL;
    int vis_start = 0;
    int cursor = -1;
    do {
        c = bxs_strchr(tag, to_utf32(','), &cursor);
        bxstr_t *single_tag = bxs_trimdup(tag, vis_start, c != NULL ? (size_t) cursor : tag->num_chars_visible);
        int rc_add = tag_add(bison_args, single_tag);
        bxs_free(single_tag);
        if (rc_add != 0) {
            rc = rc_add;
        }
        vis_start = cursor + 1;
    } while (c != NULL);
    return rc;
}



int tag_record(pass_to_bison *bison_args, bxstr_t *tag)
{
    int rc = RC_SUCCESS;
    if (tag->num_chars_invisible > 0) {
        yyerror(bison_args, "invalid tag");
        return RC_ERROR;
    }
    if (bxs_strchr(tag, to_utf32(','), NULL) != NULL) {
        rc = tag_split_add(bison_args, tag);
    }
    else {
        bxstr_t *trimmed = bxs_trimdup(tag, 0, tag->num_chars_visible);
        rc = tag_add(bison_args, trimmed);
        bxs_free(trimmed);
    }
    return rc;
}



/**
 * Rule action called when a shape list was fully parsed.
 * @param bison_args the parser state
 * @return 0: success;
 *         1: YYERROR must be invoked;
 *         2: YYABORT must be invoked
 */
int action_finalize_shapes(pass_to_bison *bison_args)
{
    int i, j;
    shape_t fshape; /* found shape */
    int fside;      /* first side */
    int sc;         /* side counter */
    int side;       /* effective side */
    int rc;         /* received return code */

    /*
     *  At least one shape must be specified
     */
    if (bison_args->num_shapespec < 1) {
        yyerror(bison_args, "must specify at least one non-empty shape per design");
        return RC_ERROR;
    }

    /*
     *  Ensure that all corners have been specified. Generate corners
     *  as necessary, starting at any side which already includes at
     *  least one shape in order to ensure correct measurements.
     */
    fshape = findshape(curdes.shape, NUM_SHAPES);
    if (fshape == NUM_SHAPES) {
        yyerror(bison_args, "internal error");
        return RC_ABORT;
    }
    fside = on_side(fshape, 0);
    if (fside == NUM_SIDES) {
        yyerror(bison_args, "internal error");
        return RC_ABORT;
    }

    for (sc = 0, side = fside; sc < NUM_SIDES; ++sc, side = (side + 1) % NUM_SIDES) {
        shape_t nshape; /* next shape */
        sentry_t *c;    /* corner to be processed */
        c = curdes.shape + sides[side][SHAPES_PER_SIDE - 1];

        if (isempty(c)) {
            nshape = findshape(c, SHAPES_PER_SIDE);
            if (side == BLEF || side == BRIG) {
                if (nshape == SHAPES_PER_SIDE) {
                    c->height = 1;
                }
                else {
                    c->height = c[nshape].height;
                }
                c->width = curdes.shape[fshape].width;
            }
            else {
                if (nshape == SHAPES_PER_SIDE) {
                    c->width = 1;
                }
                else {
                    c->width = c[nshape].width;
                }
                c->height = curdes.shape[fshape].height;
            }
            c->elastic = 0;
            rc = genshape(c->width, c->height, &(c->chars), &(c->mbcs));
            if (rc) {
                return RC_ABORT;
            }
        }

        fshape = sides[side][SHAPES_PER_SIDE - 1];
    }

    /*
     *  For all sides whose side shapes have not been defined, generate
     *  an elastic middle side shape.
     */
    for (side = 0; side < NUM_SIDES; ++side) {
        int found = 0;
        for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
            if (isempty(curdes.shape + sides[side][i])) {
                continue;
            }
            else {
                found = 1;
            }
        }
        if (!found) {
            sentry_t *c = curdes.shape + sides[side][SHAPES_PER_SIDE / 2];
            if (side == BLEF || side == BRIG) {
                c->width = curdes.shape[sides[side][0]].width;
                c->height = 1;
            }
            else {
                c->width = 1;
                c->height = curdes.shape[sides[side][0]].height;
            }
            c->elastic = 1;
            rc = genshape(c->width, c->height, &(c->chars), &(c->mbcs));
            if (rc) {
                return RC_ABORT;
            }
        }
    }

    if (check_sizes(bison_args)) {
        return RC_ERROR;
    }

    ++(bison_args->num_mandatory);
    if (++(bison_args->time_for_se_check) > 1) {
        if (perform_se_check(bison_args) != 0) {
            return RC_ERROR;
        }
    }

    /*
     *  Compute minimum height/width of a box of current design
     */
    for (i = 0; i < NUM_SIDES; ++i) {
        size_t c = 0;
        if (i % 2) { /* vertical sides */
            for (j = 0; j < SHAPES_PER_SIDE; ++j) {
                if (!isempty(curdes.shape + sides[i][j])) {
                    c += curdes.shape[sides[i][j]].height;
                }
            }
            if (c > curdes.minheight) {
                curdes.minheight = c;
            }
        }
        else { /* horizontal sides */
            for (j = 0; j < SHAPES_PER_SIDE; ++j) {
                if (!isempty(curdes.shape + sides[i][j])) {
                    c += curdes.shape[sides[i][j]].width;
                }
            }
            if (c > curdes.minwidth) {
                curdes.minwidth = c;
            }
        }
    }

    /*
     *  Compute height of highest shape in design
     */
    for (i = 0; i < NUM_SHAPES; ++i) {
        if (isempty(curdes.shape + i)) {
            continue;
        }
        if (curdes.shape[i].height > curdes.maxshapeheight) {
            curdes.maxshapeheight = curdes.shape[i].height;
        }
    }
    log_debug(__FILE__, PARSER, " Parser: Minimum box dimensions: width %d height %d\n",
                (int) curdes.minwidth, (int) curdes.minheight);
    log_debug(__FILE__, PARSER, " Parser: Maximum shape height: %d\n", (int) curdes.maxshapeheight);

    /*
     *  Set name of each shape
     */
    for (i = 0; i < NUM_SHAPES; ++i) {
        curdes.shape[i].name = i;
    }
    return RC_SUCCESS;
}



/**
 * Rule action called when a new box design is starting to be parsed.
 * @param bison_args the parser state
 * @param design_name the primary name of the design
 * @return 0: success;
 *         1: YYERROR must be invoked;
 *         2: YYABORT must be invoked
 */
int action_start_parsing_design(pass_to_bison *bison_args, char *design_name)
{
    bison_args->speeding = 0;
    bison_args->skipping = 0;

    curdes.name = (char *) strdup(design_name);
    if (curdes.name == NULL) {
        perror(PROJECT);
        return RC_ABORT;
    }

    if (!design_needed(bison_args)) {
        bison_args->speeding = 1;
        log_debug(__FILE__, PARSER, " Parser: Skipping to next design (lexer doesn't know!)\n");
        return RC_ERROR; /* trigger the parser's `error` rule, which will skip to the next design */
    }
    return RC_SUCCESS;
}



int action_parent_config(pass_to_bison *bison_args, bxstr_t *filepath)
{
    if (is_debug_logging(PARSER)) {
        char *out_filepath = bxs_to_output(filepath);
        log_debug(__FILE__, PARSER, " Parser: parent config file specified: [%s]\n", out_filepath);
        BFREE(out_filepath);
    }

    if (bxs_is_empty(filepath)) {
        bison_args->skipping = 1;
        yyerror(bison_args, "parent reference is empty");
        return RC_ERROR;
    }
    else if (strcasecmp(filepath->ascii, ":global:") == 0) { /* special token */
        filepath = discover_config_file(1);
        if (filepath == NULL) {
            bison_args->skipping = 1; /* prevent redundant "skipping to next design" message */
            yyerror(bison_args, "parent reference to global config which cannot be found");
            return RC_ERROR;
        }
    }
    else if (!bxs_valid_in_filename(filepath, NULL)) {
        yyerror(bison_args, "parent reference contains invalid characters: %s", bxs_to_output(filepath));
        return RC_ERROR;
    }
    else {
        FILE *f = bx_fopens(filepath, "r");
        if (f == NULL) {
            bison_args->skipping = 1;
            yyerror(bison_args, "parent config file not found: %s", bxs_to_output(filepath));
            return RC_ERROR;
        }
        else {
            fclose(f);
        }
    }
    if (is_debug_logging(PARSER)) {
        char *out_filepath = bxs_to_output(filepath);
        log_debug(__FILE__, PARSER, " Parser: parent config file path resolved: [%s]\n", out_filepath);
        BFREE(out_filepath);
    }

    int is_new = !array_contains_bxs(bison_args->parent_configs, bison_args->num_parent_configs, filepath);
    if (is_new) {
        bison_args->parent_configs
                = realloc(bison_args->parent_configs, (bison_args->num_parent_configs + 1) * sizeof(bxstr_t *));
        bison_args->parent_configs[bison_args->num_parent_configs] = filepath;
        ++(bison_args->num_parent_configs);
    }
    else if (is_debug_logging(PARSER)) {
        char *out_filepath = bxs_to_output(filepath);
        log_debug(__FILE__, PARSER, " Parser: duplicate parent / cycle: [%s]\n", out_filepath);
        BFREE(out_filepath);
    }
    return RC_SUCCESS;
}



int action_add_design(pass_to_bison *bison_args, char *design_primary_name, char *name_at_end)
{
    design_t *tmp;

    log_debug(__FILE__, PARSER, "--------- ADDING DESIGN \"%s\".\n", design_primary_name);

    if (strcasecmp(design_primary_name, name_at_end)) {
        yyerror(bison_args, "box design name differs at BOX and END");
        return RC_ERROR;
    }
    if (bison_args->num_mandatory < 3) {
        yyerror(bison_args, "entries SAMPLE, SHAPES, and ELASTIC are mandatory");
        return RC_ERROR;
    }
    if (design_name_exists(bison_args, design_primary_name)) {
        yyerror(bison_args, "duplicate box design name -- %s", design_primary_name);
        return RC_ERROR;
    }

    bison_args->num_mandatory = 0;
    bison_args->time_for_se_check = 0;
    bison_args->num_shapespec = 0;

    /*
     *  Check if we need to continue parsing. If not, return.
     */
    if (!full_parse_required()) {
        bison_args->num_designs = bison_args->design_idx + 1;
        return RC_ACCEPT;
    }

    /*
     *  Allocate space for next design
     */
    ++(bison_args->design_idx);
    tmp = (design_t *) realloc(bison_args->designs, (bison_args->design_idx + 1) * sizeof(design_t));
    if (tmp == NULL) {
        perror(PROJECT);
        return RC_ABORT;
    }
    bison_args->designs = tmp;
    init_design(bison_args, &(curdes));

    return RC_SUCCESS;
}



int action_record_keyword(pass_to_bison *bison_args, char *keyword, bxstr_t *value)
{
    if (is_debug_logging(PARSER)) {
        char *out_value = bxs_to_output(value);
        log_debug(__FILE__, PARSER, " Parser: entry rule fulfilled [%s = %s]\n", keyword, out_value);
        BFREE(out_value);
    }

    size_t error_pos = 0;
    if (!bxs_valid_in_kv_string(value, &error_pos)) {
        yyerror(bison_args, "invalid character in string value at position %d", (int) error_pos);
        return RC_ERROR;
    }

    if (strcasecmp(keyword, "author") == 0) {
        curdes.author = bxs_strdup(value);
        if (curdes.author == NULL) {
            perror(PROJECT);
            return RC_ABORT;
        }
    }
    else if (strcasecmp(keyword, "designer") == 0) {
        curdes.designer = bxs_strdup(value);
        if (curdes.designer == NULL) {
            perror(PROJECT);
            return RC_ABORT;
        }
    }
    else if (strcasecmp(keyword, "tags") == 0) {
        tag_record(bison_args, value); /* discard return code (we print warnings, but tolerate the problem) */
    }
    else if (strcasecmp(keyword, "indent") == 0) {
        char *val = value->ascii;
        if (strcasecmp(val, "text") == 0 || strcasecmp(val, "box") == 0 || strcasecmp(val, "none") == 0) {
            curdes.indentmode = val[0];
        }
        else {
            yyerror(bison_args, "indent keyword must be followed by \"text\", \"box\", or \"none\"");
            return RC_ERROR;
        }
    }
    else {
        yyerror(bison_args, "internal parser error (unrecognized: %s) in line %d of %s.", keyword, __LINE__, __FILE__);
        return RC_ERROR;
    }
    return RC_SUCCESS;
}



int action_padding_entry(pass_to_bison *bison_args, char *area, int value)
{
    if (value < 0) {
        yyerror(bison_args, "padding must be a positive integer (%s %d) (ignored)", area, value);
    }
    else {
        size_t len1 = strlen(area);
        if (len1 <= 3 && strncasecmp("all", area, len1) == 0) {
            curdes.padding[BTOP] = value;
            curdes.padding[BBOT] = value;
            curdes.padding[BLEF] = value;
            curdes.padding[BRIG] = value;
        }
        else if (len1 <= 10 && strncasecmp("horizontal", area, len1) == 0) {
            curdes.padding[BRIG] = value;
            curdes.padding[BLEF] = value;
        }
        else if (len1 <= 8 && strncasecmp("vertical", area, len1) == 0) {
            curdes.padding[BTOP] = value;
            curdes.padding[BBOT] = value;
        }
        else if (len1 <= 3 && strncasecmp("top", area, len1) == 0) {
            curdes.padding[BTOP] = value;
        }
        else if (len1 <= 5 && strncasecmp("right", area, len1) == 0) {
            curdes.padding[BRIG] = value;
        }
        else if (len1 <= 4 && strncasecmp("left", area, len1) == 0) {
            curdes.padding[BLEF] = value;
        }
        else if (len1 <= 6 && strncasecmp("bottom", area, len1) == 0) {
            curdes.padding[BBOT] = value;
        }
        else {
            yyerror(bison_args, "invalid padding area %s (ignored)", area);
        }
    }
    return RC_SUCCESS;
}



int action_init_parser(pass_to_bison *bison_args)
{
    bison_args->designs = (design_t *) calloc(1, sizeof(design_t));
    if (bison_args->designs == NULL) {
        perror(PROJECT);
        return RC_ABORT;
    }
    bison_args->num_designs = 1;
    init_design(bison_args, bison_args->designs);
    return RC_SUCCESS;
}



int action_add_alias(pass_to_bison *bison_args, char *alias_name)
{
    if (design_name_exists(bison_args, alias_name)) {
        yyerror(bison_args, "alias already in use -- %s", alias_name);
        return RC_ERROR;
    }
    if (alias_exists_in_child_configs(bison_args, alias_name)) {
        log_debug(__FILE__, PARSER, " Parser: alias already used by child config, dropping: %s\n", alias_name);
    }
    else {
        size_t num_aliases = array_count0(curdes.aliases);
        curdes.aliases = (char **) realloc(curdes.aliases, (num_aliases + 2) * sizeof(char *));
        curdes.aliases[num_aliases] = strdup(alias_name);
        curdes.aliases[num_aliases + 1] = NULL;
    }
    return RC_SUCCESS;
}



static bxstr_t *adjust_eols(uint32_t *sample)
{
    if (eol_pattern == NULL) {
        eol_pattern = compile_pattern("(?:(\r\n?)|(\n))");
    }
    uint32_t *replaced = regex_replace(eol_pattern, opt.eol, sample, u32_strlen(sample), 1);
    bxstr_t *result = bxs_from_unicode(replaced);
    BFREE(replaced);
    return result;
}



static uint32_t *find_first_nonblank_line(bxstr_t *sample)
{
    uint32_t *result = sample->memory;
    uint32_t *p = sample->memory;
    while (*p != char_nul) {
        while (uc_is_blank(*p) || *p == char_cr) {
            p++;
        }
        if (*p == char_newline) {
            result = ++p;
        }
        else {
            break;
        }
    }
    return result;
}



int action_sample_block(pass_to_bison *bison_args, bxstr_t *sample)
{
    log_debug(__FILE__, PARSER, " Parser: SAMPLE block rule satisfied\n");

    if (curdes.sample) {
        yyerror(bison_args, "duplicate SAMPLE block");
        return RC_ERROR;
    }

    uint32_t *p = find_first_nonblank_line(sample);
    bxstr_t *line = adjust_eols(p);
    if (line == NULL) {
        perror(PROJECT);
        return RC_ABORT;
    }

    if (bxs_is_empty(line)) {
        yyerror(bison_args, "SAMPLE block must not be empty");
        return RC_ERROR;
    }
    size_t error_pos = 0;
    if (!bxs_valid_in_sample(line, &error_pos)) {
        yyerror(bison_args, "invalid character in SAMPLE block at position %d", (int) error_pos);
        return RC_ERROR;
    }

    curdes.sample = line;
    ++(bison_args->num_mandatory);
    return RC_SUCCESS;
}



int action_add_regex_rule(pass_to_bison *bison_args, char *type, reprule_t **rule_list, size_t *rule_list_len,
        bxstr_t *search, bxstr_t *replace, char mode)
{
    size_t n = *rule_list_len;

    if (is_debug_logging(PARSER)) {
        char *out_search = bxs_to_output(search);
        char *out_replace = bxs_to_output(replace);
        log_debug(__FILE__, PARSER, "Adding %s rule: \"%s\" with \"%s\" (%c)\n",
                strcmp(type, "rep") == 0 ? "replacement" : "reversion", out_search, out_replace, mode);
        BFREE(out_replace);
        BFREE(out_search);
    }

    *rule_list = (reprule_t *) realloc(*rule_list, (n + 1) * sizeof(reprule_t));
    if (*rule_list == NULL) {
        perror(PROJECT);
        return RC_ABORT;
    }
    memset((*rule_list) + n, 0, sizeof(reprule_t));
    (*rule_list)[n].search = bxs_strdup(search);
    (*rule_list)[n].repstr = bxs_strdup(replace);
    if ((*rule_list)[n].search == NULL || (*rule_list)[n].repstr == NULL) {
        perror(PROJECT);
        return RC_ABORT;
    }
    (*rule_list)[n].line = yyget_lineno(bison_args->lexer_state);
    (*rule_list)[n].mode = mode;
    *rule_list_len = n + 1;
    return RC_SUCCESS;
}



int action_first_shape_line(pass_to_bison *bison_args, bxstr_t *line, sentry_t *shape)
{
    sentry_t rval = SENTRY_INITIALIZER;

    log_debug(__FILE__, PARSER, "Initializing a shape entry with first line\n");

    size_t error_pos = 0;
    if (!bxs_valid_in_shape(line, &error_pos)) {
        yyerror(bison_args, "invalid character in shape line at position %d", (int) error_pos);
        return RC_ERROR;
    }

    rval.width = line->num_columns;
    rval.height = 1;

    rval.chars = (char **) malloc(sizeof(char *));
    if (rval.chars == NULL) {
        perror(PROJECT ": shape_lines21");
        return RC_ABORT;
    }
    rval.chars[0] = (char *) strdup(line->ascii);
    if (rval.chars[0] == NULL) {
        perror(PROJECT ": shape_lines22");
        return RC_ABORT;
    }

    rval.mbcs = (bxstr_t **) malloc(sizeof(bxstr_t *));
    if (rval.mbcs == NULL) {
        perror(PROJECT ": shape_lines23");
        return RC_ABORT;
    }
    rval.mbcs[0] = bxs_strdup(line);
    if (rval.mbcs[0] == NULL) {
        perror(PROJECT ": shape_lines24");
        return RC_ABORT;
    }

    memcpy(shape, &rval, sizeof(sentry_t));
    return RC_SUCCESS;
}



int action_add_shape_line(pass_to_bison *bison_args, sentry_t *shape, bxstr_t *line)
{
    log_debug(__FILE__, PARSER, "Extending a shape entry\n");

    size_t slen = line->num_columns;
    if (slen != shape->width) {
        yyerror(bison_args, "all elements of a shape spec must be of equal length");
        return RC_ERROR;
    }

    size_t error_pos = 0;
    if (!bxs_valid_in_shape(line, &error_pos)) {
        yyerror(bison_args, "invalid character in shape line at position %d", (int) error_pos);
        return RC_ERROR;
    }

    shape->height++;

    char **tmp = (char **) realloc(shape->chars, shape->height * sizeof(char *));
    if (tmp == NULL) {
        perror(PROJECT ": shape_lines11");
        return RC_ABORT;
    }
    shape->chars = tmp;
    shape->chars[shape->height - 1] = (char *) strdup(line->ascii);
    if (shape->chars[shape->height - 1] == NULL) {
        perror(PROJECT ": shape_lines12");
        return RC_ABORT;
    }

    bxstr_t **mtmp = (bxstr_t **) realloc(shape->mbcs, shape->height * sizeof(bxstr_t *));
    if (mtmp == NULL) {
        perror(PROJECT ": shape_lines13");
        return RC_ABORT;
    }
    shape->mbcs = mtmp;
    shape->mbcs[shape->height - 1] = bxs_strdup(line);
    if (shape->mbcs[shape->height - 1] == NULL) {
        perror(PROJECT ": shape_lines14");
        return RC_ABORT;
    }

    return RC_SUCCESS;
}


/* vim: set cindent sw=4: */
