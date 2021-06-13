%code requires {
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
 * Yacc parser for boxes configuration files
 */

#include "config.h"
#include "boxes.h"


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
    char *config_file;

    int num_mandatory;

    int time_for_se_check;

    /** number of user-specified shapes */
    int num_shapespec;

    /** used to limit "skipping" msgs */
    int skipping;

    /** true if we're skipping designs, but no error */
    int speeding;

    /** names of config files specified via "parent" */
    char **parent_configs;

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

#include "shape.h"
#include "tools.h"
#include "parsing.h"
#include "parser.h"
#include "lex.yy.h"
#include "parsecode.h"


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
    char *s;
    char c;
    shape_t shape;
    sentry_t sentry;
    int num;
}

%token YPARENT YSHAPES YELASTIC YPADDING YSAMPLE YENDSAMPLE YBOX YEND YUNREC
%token YREPLACE YREVERSE YTO YWITH YCHGDEL YTAGS
%token <s> KEYWORD
%token <s> WORD
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
                fprintf (stderr, "%s: no valid data in config file -- %s\n", PROJECT, bison_args->config_file);
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
        #ifdef PARSER_DEBUG
            fprintf (stderr, " Parser: Discarding token [skipping=%s, speeding=%s]\n",
                bison_args->skipping ? "true" : "false", bison_args->speeding ? "true" : "false");
        #endif
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


alias: WORD
    {
        invoke_action(action_add_alias(bison_args, $1));
    }

alias_list: alias | alias_list ',' alias;

design_id: WORD | WORD ',' alias_list;

design: YBOX design_id
    {
        invoke_action(action_start_parsing_design(bison_args, $<s>2));
    }
layout YEND WORD
    {
        invoke_action(action_add_design(bison_args, $<s>2, $6));
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
        char *filename = $2;
        if (filename[0] != filename[strlen(filename) - 1]
            || (filename[0] >= 'a' && filename[0] <= 'z')
            || (filename[0] >= 'A' && filename[0] <= 'Z')
            || (filename[0] >= '0' && filename[0] <= '9'))
        {
            yyerror(bison_args, "string expected", filename);
            YYERROR;
        }
        else {
            #ifdef PARSER_DEBUG
                fprintf (stderr, " Parser: Discarding entry [%s = %s].\n", "parent", filename);
            #endif
        }
    }

| YCHGDEL YDELIMSPEC
    {
        /* string delimiters were changed - this is a lexer thing. ignore here. */
    }

| YTAGS '(' tag_list ')' | YTAGS tag_entry

| WORD STRING
    {
        #ifdef PARSER_DEBUG
            fprintf (stderr, " Parser: Discarding entry [%s = %s].\n", $1, $2);
        #endif
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
        int a = curdes.anz_reprules;

        #ifdef PARSER_DEBUG
            fprintf (stderr, "Adding replacement rule: \"%s\" with \"%s\" (%c)\n",
                    $3, $5, $2);
        #endif

        curdes.reprules = (reprule_t *) realloc (curdes.reprules, (a+1) * sizeof(reprule_t));
        if (curdes.reprules == NULL) {
            perror (PROJECT);
            YYABORT;
        }
        memset (&(curdes.reprules[a]), 0, sizeof(reprule_t));
        curdes.reprules[a].search = (char *) strdup ($3);
        curdes.reprules[a].repstr = (char *) strdup ($5);
        if (curdes.reprules[a].search == NULL || curdes.reprules[a].repstr == NULL) {
            perror (PROJECT);
            YYABORT;
        }
        curdes.reprules[a].line = yyget_lineno(scanner);
        curdes.reprules[a].mode = $2;
        curdes.anz_reprules = a + 1;
    }

| YREVERSE rflag STRING YTO STRING
    {
        int a = curdes.anz_revrules;

        #ifdef PARSER_DEBUG
            fprintf (stderr, "Adding reversion rule: \"%s\" to \"%s\" (%c)\n", $3, $5, $2);
        #endif

        curdes.revrules = (reprule_t *) realloc (curdes.revrules, (a+1) * sizeof(reprule_t));
        if (curdes.revrules == NULL) {
            perror (PROJECT);
            YYABORT;
        }
        memset (&(curdes.revrules[a]), 0, sizeof(reprule_t));
        curdes.revrules[a].search = (char *) strdup ($3);
        curdes.revrules[a].repstr = (char *) strdup ($5);
        if (curdes.revrules[a].search == NULL || curdes.revrules[a].repstr == NULL) {
            perror (PROJECT);
            YYABORT;
        }
        curdes.revrules[a].line = yyget_lineno(scanner);
        curdes.revrules[a].mode = $2;
        curdes.anz_revrules = a + 1;
    }

| YPADDING '{' wlist '}'
    {
        #ifdef PARSER_DEBUG
            fprintf (stderr, "Padding set to (l%d o%d r%d u%d)\n",
                curdes.padding[BLEF], curdes.padding[BTOP], curdes.padding[BRIG], curdes.padding[BBOT]);
        #endif
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
        #ifdef PARSER_DEBUG
            fprintf (stderr, "Marked \'%s\' shape as elastic\n", shape_name[(int) $1]);
        #endif
        curdes.shape[$1].elastic = 1;
    }
;


slist: slist slist_entry | slist_entry;


slist_entry: SHAPE shape_def
    {
        #ifdef PARSER_DEBUG
            fprintf (stderr, "Adding shape spec for \'%s\' (width %d "
                    "height %d)\n", shape_name[$1], (int) $2.width, (int) $2.height);
        #endif

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
        size_t slen = strlen ($3);
        char **tmp;

        #ifdef PARSER_DEBUG
            fprintf (stderr, "Extending a shape entry\n");
        #endif

        if (slen != rval.width) {
            yyerror(bison_args, "all elements of a shape spec must be of equal length");
            YYERROR;
        }

        rval.height++;
        tmp = (char **) realloc (rval.chars, rval.height*sizeof(char*));
        if (tmp == NULL) {
            perror (PROJECT": shape_lines11");
            YYABORT;
        }
        rval.chars = tmp;
        rval.chars[rval.height-1] = (char *) strdup ($3);
        if (rval.chars[rval.height-1] == NULL) {
            perror (PROJECT": shape_lines12");
            YYABORT;
        }
        $$ = rval;
    }

| STRING
    {
        sentry_t rval = SENTRY_INITIALIZER;

        #ifdef PARSER_DEBUG
            fprintf (stderr, "Initializing a shape entry with first line\n");
        #endif

        rval.width = strlen ($1);
        rval.height = 1;
        rval.chars = (char **) malloc (sizeof(char*));
        if (rval.chars == NULL) {
            perror (PROJECT": shape_lines21");
            YYABORT;
        }
        rval.chars[0] = (char *) strdup ($1);
        if (rval.chars[0] == NULL) {
            perror (PROJECT": shape_lines22");
            YYABORT;
        }
        $$ = rval;
    }
;


wlist: wlist wlist_entry | wlist_entry;

wlist_entry: WORD YNUMBER
    {
        invoke_action(action_padding_entry(bison_args, $1, $2));
    }
;


%%


/*EOF*/                                          /* vim: set sw=4 cindent: */
