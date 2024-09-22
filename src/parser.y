%code requires {
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
 * Yacc parser for boxes configuration files
 */

#include "config.h"
#include "boxes.h"
#include "bxstring.h"


/** all the arguments which we pass to the bison parser */
typedef struct {
    /** bison will store the parsed designs here, also allocating memory as required */
    design_t *designs;

    /** the size of `*designs` */
    size_t num_designs;

    /** index into `*designs` */
    int design_idx;

    /** Box designs already parsed from child config files, if any. Else NULL */
    design_t *child_configs;

    /** the size of `*child_configs` */
    size_t num_child_configs;

    /** the path to the config file we are parsing */
    bxstr_t *config_file;

    int num_mandatory;

    int time_for_se_check;

    /** number of user-specified shapes */
    int num_shapespec;

    /** used to limit "skipping" msgs */
    int skipping;

    /** true if we're skipping designs, but no error */
    int speeding;

    /** names of config files specified via "parent" */
    bxstr_t **parent_configs;

    /** number of parent config files (size of parent_configs array) */
    size_t num_parent_configs;

    /** the flex scanner state, which is explicitly passed to reentrant bison */
    void *lexer_state;
} pass_to_bison;

}

%code {

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unictype.h>

#include "logging.h"
#include "shape.h"
#include "tools.h"
#include "parsing.h"
#include "parser.h"
#include "lex.yy.h"
#include "parsecode.h"
#include "unicode.h"


/** required for bison-flex bridge */
#define scanner bison_args->lexer_state

/** invoke a parsecode action and react to its return code */
#define invoke_action(action) {   \
    int rc = (action);            \
    if (rc == RC_ERROR) {         \
        YYERROR;                  \
    } else if (rc == RC_ABORT) {  \
        YYABORT;                  \
    } else if (rc == RC_ACCEPT) { \
        YYACCEPT;                 \
    }                             \
}

}


/*\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\
|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
|  |                                                                                            |  |
|  |                              B i s o n  D e f i n i t i o n s                              |  |
|  |                                                                                            |  |
|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|
|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|_*/

%define api.pure true
%lex-param {void *scanner}
%parse-param {pass_to_bison *bison_args}

%union {
    bxstr_t *s;
    char *ascii;
    char c;
    shape_t shape;
    sentry_t sentry;
    int num;
}

%token YPARENT YSHAPES YELASTIC YPADDING YSAMPLE YENDSAMPLE YBOX YEND YUNREC
%token YREPLACE YREVERSE YTO YWITH YCHGDEL YTAGS
%token <ascii> KEYWORD
%token <s> BXWORD
%token <ascii> ASCII_ID
%token <s> STRING
%token <s> FILENAME
%token <shape> SHAPE
%token <num> YNUMBER
%token <c> YRXPFLAG
%token <s> YDELIMSPEC

%type <sentry> shape_def
%type <sentry> shape_lines
%type <c> rflag

%start first_rule


%%

/*\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\
|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
|  |                                                                                            |  |
|  |                             G r a m m a r   &   A c t i o n s                              |  |
|  |                                                                                            |  |
|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|
|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|_*/


first_rule:
    {
        invoke_action(action_init_parser(bison_args));
    }

config_file
    {
        /*
         *  Clean up parser data structures
         */
        design_t *tmp;

        if (bison_args->design_idx == 0) {
            BFREE (bison_args->designs);
            bison_args->num_designs = 0;
            if (!opt.design_choice_by_user && bison_args->num_parent_configs == 0) {
                fprintf(stderr, "%s: no valid data in config file -- %s\n", PROJECT,
                        bxs_to_output(bison_args->config_file));
                YYABORT;
            }
            YYACCEPT;
        }

        --(bison_args->design_idx);
        bison_args->num_designs = bison_args->design_idx + 1;
        tmp = (design_t *) realloc (bison_args->designs, (bison_args->num_designs) * sizeof(design_t));
        if (!tmp) {
            perror (PROJECT);
            YYABORT;
        }
        bison_args->designs = tmp;
    }

parent_def: YPARENT FILENAME
    {
        invoke_action(action_parent_config(bison_args, $2));
    }

config_file: config_file design_or_error | design_or_error | config_file parent_def | parent_def ;


design_or_error: design | error
    {
        /* reset alias list, as those are collected even when speedmode is on */
        log_debug(__FILE__, PARSER, " Parser: Discarding token [skipping=%s, speeding=%s]\n",
                bison_args->skipping ? "true" : "false", bison_args->speeding ? "true" : "false");
        if (curdes.aliases[0] != NULL) {
            BFREE(curdes.aliases);
            curdes.aliases = (char **) calloc(1, sizeof(char *));
        }
        if (!bison_args->speeding && !bison_args->skipping) {
            recover(bison_args);
            yyerror(bison_args, "skipping to next design");
            bison_args->skipping = 1;
        }
    }


alias: ASCII_ID
    {
        invoke_action(action_add_alias(bison_args, $1));
    }

alias_list: alias | alias_list ',' alias;

design_id: ASCII_ID | ASCII_ID ',' alias_list

| BXWORD
    {
        yyerror(bison_args, "box design name must consist of printable standard ASCII characters.");
        YYERROR;
    }
;

design: YBOX design_id
    {
        invoke_action(action_start_parsing_design(bison_args, $<ascii>2));
    }
layout YEND ASCII_ID
    {
        invoke_action(action_add_design(bison_args, $<ascii>2, $6));
    }
;


layout: layout entry | layout block | entry | block ;


tag_entry: STRING
    {
        tag_record(bison_args, $1);    /* discard return code (we print warnings, but tolerate the problem) */
    }

tag_list: tag_entry | tag_list ',' tag_entry;


entry: KEYWORD STRING
    {
        invoke_action(action_record_keyword(bison_args, $1, $2));
    }

| YPARENT FILENAME
    {
        /*
         * Called when PARENT appears as a key inside a box design. That's a user mistake, but not an error.
         */
        bxstr_t *filename = $2;
        if (filename->memory[0] != filename->memory[filename->num_chars - 1] || uc_is_alnum(filename->memory[0])) {
            yyerror(bison_args, "string expected");
            YYERROR;
        }
        else if (is_debug_logging(PARSER)) {
            char *out_filename = bxs_to_output(filename);
            log_debug(__FILE__, PARSER, " Parser: Discarding entry [%s = %s].\n", "parent", out_filename);
            BFREE(out_filename);
        }
    }

| YCHGDEL YDELIMSPEC
    {
        /* string delimiters were changed - this is a lexer thing. ignore here. */
    }

| YTAGS '(' tag_list ')' | YTAGS tag_entry

| BXWORD STRING | ASCII_ID STRING
    {
        if (is_debug_logging(PARSER)) {
            char *out_string = bxs_to_output($2);
            log_debug(__FILE__, PARSER, " Parser: Discarding entry [%s = %s].\n", $1, out_string);
            BFREE(out_string);
        }
    }
;


block: YSAMPLE STRING YENDSAMPLE
    {
        invoke_action(action_sample_block(bison_args, $2));
    }

| YSHAPES  '{' slist  '}'
    {
        invoke_action(action_finalize_shapes(bison_args));
    }

| YELASTIC '(' elist ')'
    {
        ++(bison_args->num_mandatory);
        if (++(bison_args->time_for_se_check) > 1) {
            if (perform_se_check(bison_args) != 0) {
                YYERROR;
            }
        }
    }

| YREPLACE rflag STRING YWITH STRING
    {
        invoke_action(action_add_regex_rule(bison_args, "rep", &curdes.reprules, &curdes.num_reprules, $3, $5, $2));
    }

| YREVERSE rflag STRING YTO STRING
    {
        invoke_action(action_add_regex_rule(bison_args, "rev", &curdes.revrules, &curdes.num_revrules, $3, $5, $2));
    }

| YPADDING '{' wlist '}'
    {
        log_debug(__FILE__, PARSER, "Padding set to (l%d o%d r%d u%d)\n",
                curdes.padding[BLEF], curdes.padding[BTOP], curdes.padding[BRIG], curdes.padding[BBOT]);
    }
;


rflag: YRXPFLAG
    {
        $$ = $1;
    }
| %empty
    {
        $$ = 'g';
    }
;


elist: elist ',' elist_entry | elist_entry;


elist_entry: SHAPE
    {
        log_debug(__FILE__, PARSER, "Marked \'%s\' shape as elastic\n", shape_name[(int) $1]);
        curdes.shape[$1].elastic = 1;
    }
;


slist: slist slist_entry | slist_entry;


slist_entry: SHAPE shape_def
    {
        log_debug(__FILE__, PARSER, "Adding shape spec for \'%s\' (width %d height %d)\n",
                shape_name[$1], (int) $2.width, (int) $2.height);

        if (isempty (curdes.shape + $1)) {
            curdes.shape[$1] = $2;
            if (!isdeepempty(&($2))) {
                ++(bison_args->num_shapespec);
            }
        }
        else {
            yyerror(bison_args, "duplicate specification for %s shape", shape_name[$1]);
            YYERROR;
        }
    }
;


shape_def: '(' shape_lines ')'
    {
        if ($2.width == 0 || $2.height == 0) {
            yyerror(bison_args, "minimum shape dimension is 1x1 - clearing");
            freeshape (&($2));
        }
        $$ = $2;
    }

| '(' ')'
    {
        $$ = SENTRY_INITIALIZER;
    }
;


shape_lines: shape_lines ',' STRING
    {
        sentry_t rval = $1;
        invoke_action(action_add_shape_line(bison_args, &rval, $3));
        $$ = rval;
    }

| STRING
    {
        sentry_t rval;
        invoke_action(action_first_shape_line(bison_args, $1, &rval));
        $$ = rval;
    }
;


wlist: wlist wlist_entry | wlist_entry;

wlist_entry: ASCII_ID YNUMBER
    {
        invoke_action(action_padding_entry(bison_args, $1, $2));
    }
;


%%


/* vim: set sw=4 cindent: */
