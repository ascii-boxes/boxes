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
 * C code used exclusively by the parser.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "discovery.h"
#include "tools.h"
#include "regulex.h"
#include "unicode.h"
#include "query.h"
#include "parsecode.h"
#include "parsing.h"
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

    #ifdef PARSER_DEBUG
        fprintf (stderr, " Parser: check_sizes()\n");
    #endif

    for (i=0; i<NUM_SIDES; ++i) {
        if (i == 0 || i == 2) {
            /*
             *  horizontal
             */
            for (j=0; j<SHAPES_PER_SIDE-1; ++j) {
                if (curdes.shape[sides[i][j]].height == 0) {
                    continue;
                }
                for (k=j+1; k<SHAPES_PER_SIDE; ++k) {
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
            for (j=0; j<SHAPES_PER_SIDE-1; ++j) {
                if (curdes.shape[sides[i][j]].width == 0) {
                    continue;
                }
                for (k=j+1; k<SHAPES_PER_SIDE; ++k) {
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

    return 0;                            /* all clear */
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

    #ifdef PARSER_DEBUG
        fprintf (stderr, " Parser: corner_check()\n");
    #endif

    for (c=0; c<NUM_CORNERS; ++c) {
        if (curdes.shape[corners[c]].elastic) {
            yyerror(bison_args, "Corners may not be elastic (%s)", shape_name[corners[c]]);
            return 1;
        }
    }

    return 0;                            /* all clear */
}



static shape_t non_existent_elastics(pass_to_bison *bison_args)
{
    shape_t i;

    #ifdef PARSER_DEBUG
        fprintf (stderr, " Parser: non_existent_elastics()\n");
    #endif

    for (i=0; i<NUM_SHAPES; ++i) {
        if (curdes.shape[i].elastic && isempty(curdes.shape+i)) {
            return i;
        }
    }

    return (shape_t) NUM_SHAPES;         /* all elastic shapes exist */
}



static int insufficient_elasticity(pass_to_bison *bison_args)
{
    int i, j, ef;

    #ifdef PARSER_DEBUG
        fprintf (stderr, " Parser: insufficient_elasticity()\n");
    #endif

    for (i=0; i<NUM_SIDES; ++i) {
        for (j=1,ef=0; j<4; ++j) {
            if (curdes.shape[sides[i][j]].elastic) {
                ++ef;
            }
        }
        if (ef != 1 && ef != 2) {
            return 1;                    /* error */
        }
    }

    return 0;                            /* all clear */
}



static int adjoining_elastics(pass_to_bison *bison_args)
{
    int i, j, ef;

    #ifdef PARSER_DEBUG
        fprintf (stderr, " Parser: adjoining_elastics()\n");
    #endif

    for (i=0; i<NUM_SIDES; ++i) {
        ef = 0;
        for (j=1; j<4; ++j) {
            if (isempty(curdes.shape+sides[i][j])) {
                continue;
            }
            if (curdes.shape[sides[i][j]].elastic) {
                if (ef) {
                    return 1;            /* error detected */
                } else {
                    ef = 1;
                }
            }
            else {
                ef = 0;
            }
        }
    }

    return 0;                            /* all clear */
}



int perform_se_check(pass_to_bison *bison_args)
{
    shape_t s_rc;

    s_rc = non_existent_elastics(bison_args);
    if (s_rc != NUM_SHAPES) {
        yyerror(bison_args, "Shape %s has been specified as elastic but doesn't exist",
                shape_name[s_rc]);
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
    memset (design, 0, sizeof(design_t));
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
     BFREE (curdes.name);
     BFREE (curdes.author);
     BFREE (curdes.aliases);
     BFREE (curdes.designer);
     BFREE (curdes.created);
     BFREE (curdes.revision);
     BFREE (curdes.revdate);
     BFREE (curdes.sample);
     BFREE (curdes.tags);
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
        result = opt.r || opt.l || (opt.query != NULL && !query_is_undoc());
    }
    #ifdef DEBUG
        fprintf(stderr, " Parser: full_parse_required() -> %s\n", result ? "true" : "false");
    #endif
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



static int tag_add(pass_to_bison *bison_args, char *tag)
{
    int rc = RC_SUCCESS;
    if (tag_is_valid(tag)) {
        if (!array_contains0(curdes.tags, tag)) {
            size_t num_tags = array_count0(curdes.tags);
            curdes.tags = (char **) realloc(curdes.tags, (num_tags + 2) * sizeof(char *));
            curdes.tags[num_tags] = tag;
            curdes.tags[num_tags + 1] = NULL;
        }
        else {
            yyerror(bison_args, "duplicate tag -- %s", tag);
            rc = RC_ERROR;
        }
    }
    else {
        yyerror(bison_args, "invalid tag -- %s", tag);
        rc = RC_ERROR;
    }
    return rc;
}



static char *tag_next_comma(char *s)
{
    char *comma = strchr(s, ',');
    if (comma == NULL) {
        comma = s + strlen(s);
    }
    return comma;
}



static int tag_split_add(pass_to_bison *bison_args, char *tag)
{
    int rc = RC_SUCCESS;
    char *s = tag;
    char *c = NULL;
    do {
        c = tag_next_comma(s);
        char *single_tag = trimdup(s, c - 1);
        int rc_add = tag_add(bison_args, single_tag);
        if (rc_add != 0) {
            rc = rc_add;
        }
        s = c + 1;
    } while (*c != '\0');
    return rc;
}



/**
 * Add tag to list of current design's tag after validity checking.
 * @param bison_args the parser state
 * @param tag a single tag, or a comma-separated list of tags
 * @return error code, 0 on success, anything else on failure
 */
int tag_record(pass_to_bison *bison_args, char *tag)
{
    int rc = RC_SUCCESS;
    if (strchr(tag, ',') != NULL) {
        rc = tag_split_add(bison_args, tag);
    }
    else {
        char *trimmed = trimdup(tag, tag + strlen(tag) - 1);
        rc = tag_add(bison_args, trimmed);
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
    int     i,j;
    shape_t fshape;                  /* found shape */
    int     fside;                   /* first side */
    int     sc;                      /* side counter */
    int     side;                    /* effective side */
    int     rc;                      /* received return code */

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
    fshape = findshape (curdes.shape, NUM_SHAPES);
    if (fshape == NUM_SHAPES) {
        yyerror(bison_args, "internal error");
        return RC_ABORT;
    }
    fside = on_side (fshape, 0);
    if (fside == NUM_SIDES) {
        yyerror(bison_args, "internal error");
        return RC_ABORT;
    }

    for (sc=0,side=fside; sc<NUM_SIDES; ++sc,side=(side+1)%NUM_SIDES) {
        shape_t   nshape;               /* next shape */
        sentry_t *c;                    /* corner to be processed */
        c = curdes.shape + sides[side][SHAPES_PER_SIDE-1];

        if (isempty(c)) {
            nshape = findshape (c, SHAPES_PER_SIDE);
            if (side == BLEF || side == BRIG) {
                if (nshape == SHAPES_PER_SIDE) {
                    c->height = 1;
                } else {
                    c->height = c[nshape].height;
                }
                c->width = curdes.shape[fshape].width;
            }
            else {
                if (nshape == SHAPES_PER_SIDE) {
                    c->width = 1;
                } else {
                    c->width = c[nshape].width;
                }
                c->height = curdes.shape[fshape].height;
            }
            c->elastic = 0;
            rc = genshape (c->width, c->height, &(c->chars));
            if (rc) {
                return RC_ABORT;
            }
        }

        fshape = sides[side][SHAPES_PER_SIDE-1];
    }

    /*
     *  For all sides whose side shapes have not been defined, generate
     *  an elastic middle side shape.
     */
    for (side=0; side<NUM_SIDES; ++side) {
        int found = 0;
        for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
            if (isempty (curdes.shape + sides[side][i])) {
                continue;
            } else {
                found = 1;
            }
        }
        if (!found) {
            sentry_t *c = curdes.shape + sides[side][SHAPES_PER_SIDE/2];
            if (side == BLEF || side == BRIG) {
                c->width = curdes.shape[sides[side][0]].width;
                c->height = 1;
            }
            else {
                c->width = 1;
                c->height = curdes.shape[sides[side][0]].height;
            }
            c->elastic = 1;
            rc = genshape (c->width, c->height, &(c->chars));
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
    for (i=0; i<NUM_SIDES; ++i) {
        size_t c = 0;
        if (i % 2) {                 /* vertical sides */
            for (j=0; j<SHAPES_PER_SIDE; ++j) {
                if (!isempty(curdes.shape + sides[i][j])) {
                    c += curdes.shape[sides[i][j]].height;
                }
            }
            if (c > curdes.minheight) {
                curdes.minheight = c;
            }
        }
        else {                       /* horizontal sides */
            for (j=0; j<SHAPES_PER_SIDE; ++j) {
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
    for (i=0; i<NUM_SHAPES; ++i) {
        if (isempty(curdes.shape + i)) {
            continue;
        }
        if (curdes.shape[i].height > curdes.maxshapeheight) {
            curdes.maxshapeheight = curdes.shape[i].height;
        }
    }
    #ifdef PARSER_DEBUG
        fprintf (stderr, " Parser: Minimum box dimensions: width %d height %d\n",
                (int) curdes.minwidth, (int) curdes.minheight);
        fprintf (stderr, " Parser: Maximum shape height: %d\n",
                (int) curdes.maxshapeheight);
    #endif
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
        #ifdef PARSER_DEBUG
            fprintf (stderr, " Parser: Skipping to next design (lexer doesn't know!)\n");
        #endif
        return RC_ERROR;         /* trigger the parser's `error` rule, which will skip to the next design */
    }
    return RC_SUCCESS;
}



/**
 * Rule action called when a parent config file specification is encountered.
 * @param bison_args the parser state
 * @param filepath the path to the parent config file
 * @return 0: success;
 *         1: YYERROR must be invoked
 */
int action_parent_config(pass_to_bison *bison_args, char *filepath)
{
    #ifdef PARSER_DEBUG
        fprintf (stderr, " Parser: parent config file specified: [%s]\n", filepath);
    #endif
    if (filepath == NULL || filepath[0] == '\0') {
        bison_args->skipping = 1;
        yyerror(bison_args, "parent reference is empty");
        return RC_ERROR;
    }
    else if (strcasecmp(filepath, ":global:") == 0) {    /* special token */
        filepath = discover_config_file(1);
        if (filepath == NULL) {
            bison_args->skipping = 1;   /* prevent redundant "skipping to next design" message */
            yyerror(bison_args, "parent reference to global config which cannot be found");
            return RC_ERROR;
        }
    }
    else {
        FILE *f = fopen(filepath, "r");
        if (f == NULL) {
            bison_args->skipping = 1;
            yyerror(bison_args, "parent config file not found: %s", filepath);
            return RC_ERROR;
        }
        else {
            fclose(f);
        }
    }
    #ifdef PARSER_DEBUG
        fprintf (stderr, " Parser: parent config file path resolved: [%s]\n", filepath);
    #endif
    
    int is_new = !array_contains(bison_args->parent_configs, bison_args->num_parent_configs, filepath);
    if (!is_new) {
        #ifdef PARSER_DEBUG
            fprintf (stderr, " Parser: duplicate parent / cycle: [%s]\n", filepath);
        #endif
    }
    else {
        bison_args->parent_configs = realloc(bison_args->parent_configs,
                (bison_args->num_parent_configs + 1) * sizeof(char *));
        bison_args->parent_configs[bison_args->num_parent_configs] = filepath;
        ++(bison_args->num_parent_configs);
    }
    return RC_SUCCESS;
}



/**
 * Rule action called when a design has been parsed completely and can be checked and added.
 * @param bison_args the parser state
 * @param design_primary_name the primary name of the design as specified at BOX statement
 * @param name_at_end the primary name of the design as specified at END statement
 * @return 0: success;
 *         1: YYERROR must be invoked
 *         2: YYABORT must be invoked
 *         3: YYACCEPT must be invoked
 */
int action_add_design(pass_to_bison *bison_args, char *design_primary_name, char *name_at_end)
{
    design_t *tmp;
    char *p;

    #ifdef PARSER_DEBUG
        fprintf (stderr, "--------- ADDING DESIGN \"%s\".\n", design_primary_name);
    #endif

    if (strcasecmp (design_primary_name, name_at_end)) {
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

    p = design_primary_name;
    while (*p) {
        if (*p < 32 || *p > 126) {
            yyerror(bison_args, "box design name must consist of printable standard ASCII characters.");
            return RC_ERROR;
        }
        ++p;
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
        perror (PROJECT);
        return RC_ABORT;
    }
    bison_args->designs = tmp;
    init_design(bison_args, &(curdes));

    return RC_SUCCESS;
}



/**
 * Rule action called when a keyword entry is encountered in a box design.
 * @param bison_args the parser state
 * @param keyword the keyword
 * @param value the associated value
 * @return 0: success;
 *         1: YYERROR must be invoked
 *         2: YYABORT must be invoked
 */
int action_record_keyword(pass_to_bison *bison_args, char *keyword, char *value)
{
    #ifdef PARSER_DEBUG
        fprintf (stderr, " Parser: entry rule fulfilled [%s = %s]\n", keyword, value);
    #endif
    if (strcasecmp (keyword, "author") == 0) {
        curdes.author = (char *) strdup (value);
        if (curdes.author == NULL) {
            perror (PROJECT);
            return RC_ABORT;
        }
    }
    else if (strcasecmp (keyword, "designer") == 0) {
        curdes.designer = (char *) strdup (value);
        if (curdes.designer == NULL) {
            perror (PROJECT);
            return RC_ABORT;
        }
    }
    else if (strcasecmp (keyword, "revision") == 0) {
        curdes.revision = (char *) strdup (value);
        if (curdes.revision == NULL) {
            perror (PROJECT);
            return RC_ABORT;
        }
    }
    else if (strcasecmp (keyword, "created") == 0) {
        curdes.created = (char *) strdup (value);
        if (curdes.created == NULL) {
            perror (PROJECT);
            return RC_ABORT;
        }
    }
    else if (strcasecmp (keyword, "revdate") == 0) {
        curdes.revdate = (char *) strdup (value);
        if (curdes.revdate == NULL) {
            perror (PROJECT);
            return RC_ABORT;
        }
    }
    else if (strcasecmp (keyword, "tags") == 0) {
        tag_record(bison_args, value);    /* discard return code (we print warnings, but tolerate the problem) */
    }
    else if (strcasecmp (keyword, "indent") == 0) {
        if (strcasecmp (value, "text") == 0 ||
            strcasecmp (value, "box") == 0 ||
            strcasecmp (value, "none") == 0) {
            curdes.indentmode = value[0];
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
    bison_args->designs = (design_t *) calloc (1, sizeof(design_t));
    if (bison_args->designs == NULL) {
        perror (PROJECT);
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
        #ifdef PARSER_DEBUG
            fprintf (stderr, " Parser: alias already used by child config, dropping: %s\n", alias_name);
        #endif
    }
    else {
        size_t num_aliases = array_count0(curdes.aliases);
        curdes.aliases = (char **) realloc(curdes.aliases, (num_aliases + 2) * sizeof(char *));
        curdes.aliases[num_aliases] = strdup(alias_name);
        curdes.aliases[num_aliases + 1] = NULL;
    }
    return RC_SUCCESS;
}



static char *adjust_eols(char *sample)
{
    if (eol_pattern == NULL) {
        eol_pattern = compile_pattern("(?(?=\r)(\r\n?)|(\n))");
    }
    uint32_t *u32_sample = u32_strconv_from_input(sample);
    uint32_t *replaced = regex_replace(eol_pattern, opt.eol, u32_sample, strlen(sample), 1);
    char *result = u32_strconv_to_output(replaced);
    BFREE(replaced);
    BFREE(u32_sample);
    return result;
}



int action_sample_block(pass_to_bison *bison_args, char *sample)
{
    #ifdef PARSER_DEBUG
        fprintf(stderr, " Parser: SAMPLE block rule satisfied\n");
    #endif

    if (curdes.sample) {
        yyerror(bison_args, "duplicate SAMPLE block");
        return RC_ERROR;
    }

    char *p = sample;
    while ((*p == '\r' || *p == '\n') && *p != '\0') {
        p++;
    }
    char *line = adjust_eols(p);
    if (line == NULL) {
        perror(PROJECT);
        return RC_ABORT;
    }

    curdes.sample = line;
    ++(bison_args->num_mandatory);
    return RC_SUCCESS;
}


/*EOF*/                                          /* vim: set cindent sw=4: */
