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
 * Invocation of the configuration file parser
 */

#include "config.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "boxes.h"
#include "parsing.h"
#include "parser.h"
#include "lex.yy.h"
#include "tools.h"


/** file handle of the config file currently being parsed */
static FILE *current_config_handle = NULL;

/** the arguments passed to our current invocation of the parser */
static pass_to_bison *current_bison_args = NULL;

/** the name of the initially specified config file */
static const char *first_config_file = NULL;

/** all parent configs encountered across all parsed config files */
static char **parent_configs = NULL;

/** total number of parent configs (the size of the `parent_configs` array) */
static size_t num_parent_configs = 0;



/**
 *  Set yyin to the config file to be used.
 *
 *  @param bison_args the bison args that we set up in the calling function (contains config file path)
 *  @return 0 if successful (yyin is set)  
 *          != 0 on error (yyin is unmodified)
 */
static int open_yy_config_file(pass_to_bison *bison_args)
{
    current_config_handle = fopen(bison_args->config_file, "r");
    if (current_config_handle == NULL) {
        fprintf(stderr, "%s: Couldn't open config file '%s' for input\n", PROJECT, bison_args->config_file);
        return 1;
    }
    yyset_in(current_config_handle, bison_args->lexer_state);
    return 0;
}



void print_design_list_header()
{
    char buf[42];
    sprintf(buf, "%d", num_designs);

    fprintf(opt.outfile, "%d Available Style%s", num_designs, num_designs == 1 ? "" : "s");
    if (num_parent_configs > 0) {
        fprintf(opt.outfile, ":%s", opt.eol);
        fprintf(opt.outfile, "-----------------%s", num_designs == 1 ? "" : "-");
        for (int i = strlen(buf); i > 0; --i) {
            fprintf(opt.outfile, "-");
        }
        fprintf(opt.outfile, "%s%s", opt.eol, opt.eol);
        fprintf(opt.outfile, "Configuration Files:%s", opt.eol);
        fprintf(opt.outfile, "    - %s%s", first_config_file, opt.eol);
        for (size_t i = 0; i < num_parent_configs; i++) {
            fprintf(opt.outfile, "    - %s (parent)%s", parent_configs[i], opt.eol);
        }
    }
    else {
        fprintf(opt.outfile, " in \"%s\":%s", first_config_file, opt.eol);
        fprintf(opt.outfile, "-----------------------%s", num_designs == 1 ? "" : "-");
        for (int i = strlen(first_config_file) + strlen(buf); i > 0; --i) {
            fprintf(opt.outfile, "-");
        }
    }
    fprintf(opt.outfile, "%s%s", opt.eol, opt.eol);
}



/**
 * Print configuration file parser errors.
 * @param bison_args pointer to the parser arguments
 * @param fmt a format string for `vfprintf()`, followed by the arguments
 * @return 0
 */
int yyerror(pass_to_bison *bison_args, const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);

    pass_to_bison *bargs = bison_args ? bison_args : current_bison_args;
    fprintf(stderr, "%s: %s: line %d: ", PROJECT, bargs->config_file, yyget_lineno(bargs->lexer_state));
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);

    va_end (ap);

    return 0;
}



static pass_to_bison new_bison_args(const char *config_file)
{
    pass_to_bison bison_args;
    bison_args.designs = NULL;
    bison_args.num_designs = 0;
    bison_args.design_idx = 0;
    bison_args.config_file = (char *) config_file;
    bison_args.num_mandatory = 0;
    bison_args.time_for_se_check = 0;
    bison_args.num_shapespec = 0;
    bison_args.skipping = 0;
    bison_args.speeding = 0;
    bison_args.parent_configs = NULL;
    bison_args.num_parent_configs = 0;
    bison_args.lexer_state = NULL;
    return bison_args;
}



static pass_to_flex new_flex_extra_data()
{
	pass_to_flex flex_extra_data;
	flex_extra_data.yyerrcnt = 0;
	flex_extra_data.sdel = '\"';
    flex_extra_data.sesc = '\\';
    return flex_extra_data;
}



static pass_to_bison parse_config_file(const char *config_file, design_t *child_configs, size_t num_child_configs)
{
    #ifdef DEBUG
        fprintf (stderr, "Parsing Config File %s ...\n", config_file);
    #endif

    pass_to_bison bison_args = new_bison_args(config_file);
    bison_args.child_configs = child_configs;
    bison_args.num_child_configs = num_child_configs;
	pass_to_flex flex_extra_data = new_flex_extra_data();
    current_bison_args = &bison_args;

    yylex_init (&(bison_args.lexer_state));
	yylex_init_extra(&flex_extra_data, &(bison_args.lexer_state));
    int rc = open_yy_config_file(&bison_args);
    if (rc) {
        return new_bison_args(config_file);
    }
    inflate_inbuf(bison_args.lexer_state, config_file);
    rc = yyparse(&bison_args);
    yylex_destroy(bison_args.lexer_state);

    if (current_config_handle != NULL) {
        fclose(current_config_handle);
        current_config_handle = NULL;
    }

    if (rc) {
        #ifdef DEBUG
            fprintf (stderr, "yyparse() returned %d\n", rc);
        #endif
        return new_bison_args(config_file);
    }
    return bison_args;
}



static int designs_contain(design_t *designs, size_t num_designs, design_t adesign)
{
    int result = adesign.name == NULL;    /* broken records count as "present", so we don't copy them */
    if (designs != NULL && num_designs > 0 && adesign.name != NULL) {
        for (size_t d = 0; d < num_designs; d++) {
            if (strcasecmp(designs[d].name, adesign.name) == 0) {
                result = 1;
                break;
            }
        }
    }
    return result;
}



static int record_parent_config_files(pass_to_bison *bison_args)
{
    if (bison_args->num_parent_configs > 0) {
        for (int parent_idx = bison_args->num_parent_configs - 1; parent_idx >= 0; parent_idx--) {
            char *parent_file = bison_args->parent_configs[parent_idx];
            int is_new = !array_contains(parent_configs, num_parent_configs, parent_file);
            if (is_new) {
                parent_configs = (char **) realloc(parent_configs, (num_parent_configs + 1) * sizeof(char *));
                if (parent_configs == NULL) {
                    return 1;
                }
                parent_configs[num_parent_configs] = parent_file;
                ++num_parent_configs;
            }
        }
    }
    BFREE(bison_args->parent_configs);
    return 0;
}



static int copy_designs(pass_to_bison *bison_args, design_t **r_result, size_t *r_num_designs)
{
    if (bison_args->num_designs > 0) {
        if (*r_result == NULL) {
            *r_result = bison_args->designs;
            *r_num_designs = bison_args->num_designs;
        }
        else {
            for (size_t d = 0; d < bison_args->num_designs; d++) {
                if (!designs_contain(*r_result, *r_num_designs, bison_args->designs[d])) {
                    *r_result = (design_t *) realloc(*r_result, (*r_num_designs + 1) * sizeof(design_t));
                    if (*r_result == NULL) {
                        perror(PROJECT);
                        return 1;
                    }
                    memcpy(*r_result + *r_num_designs, bison_args->designs + d, sizeof(design_t));
                    (*r_num_designs)++;
                }
            }
            BFREE(bison_args->designs);
        }
    }
    return 0;
}



design_t *parse_config_files(const char *p_first_config_file, size_t *r_num_designs)
{
    size_t parents_parsed = -1;      /** how many parent config files have already been parsed */

    design_t *result = NULL;
    *r_num_designs = 0;

    first_config_file = p_first_config_file;
    const char *config_file = p_first_config_file;
    do {
        pass_to_bison bison_args = parse_config_file(config_file, result, *r_num_designs);
        ++parents_parsed;
        #ifdef DEBUG
            fprintf (stderr, "bison_args returned: "
                ".num_parent_configs=%d, .parent_configs=%p, .num_designs=%d, .designs=%p\n",
                (int) bison_args.num_parent_configs, bison_args.parent_configs,
                (int) bison_args.num_designs, bison_args.designs);
        #endif

        if (record_parent_config_files(&bison_args) != 0) {
            perror(PROJECT);
            return NULL;
        }
        if (copy_designs(&bison_args, &result, r_num_designs) != 0) {
            return NULL;
        }

        #ifdef DEBUG
            fprintf (stderr, "design count raised to: %d\n", (int) *r_num_designs);
        #endif
        if (parents_parsed < num_parent_configs) {
            config_file = parent_configs[parents_parsed];
        }
    } while (parents_parsed < num_parent_configs);

    if (*r_num_designs == 0) {
        if (opt.design_choice_by_user) {
            fprintf (stderr, "%s: unknown box design -- %s\n", PROJECT, (char *) opt.design);
        }
        else {
            fprintf (stderr, "%s: no valid designs found\n", PROJECT);
        }
        return NULL;
    }

    fflush(stderr);      /* ensure order of error msgs and normal output for test cases */
    return result;
}


/*EOF*/                                                  /* vim: set sw=4: */
