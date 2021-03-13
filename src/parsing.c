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
 * Configuration file parsing
 */

#include "config.h"
#include <stdarg.h>
#include <string.h>

#include "boxes.h"
#include "parsing.h"
#include "parser.h"
#include "lex.yy.h"


/* file name of the config file currently being parsed */
char *current_config_file_name = NULL;

/** file handle of the config file currently being parsed */
static FILE *current_config_handle = NULL;

/** names of config files specified via "parent" */
char **parent_configs = NULL;

/** number of parent config files (size of parent_configs array) */
size_t num_parent_configs = 0;

/** the arguments passed to our current invocation of the parser */
static pass_to_bison *current_bison_args = NULL;




/**
 *  Set yyin and current_config_file_name to the config file to be used.
 *
 *  @param bison_args the bison args that we set up in the calling function
 *  @param config_file_path the new file name to set
 *  @return 0 if successful (yyin and current_config_file_name are set)  
 *          != 0 on error (yyin is unmodified)
 */
static int open_yy_config_file(pass_to_bison *bison_args, const char *config_file_path)
{
    current_config_handle = fopen(config_file_path, "r");
    if (current_config_handle == NULL) {
        fprintf(stderr, "%s: Couldn't open config file '%s' for input\n", PROJECT, config_file_path);
        return 1;
    }
    current_config_file_name = (char *) config_file_path;
    yyset_in(current_config_handle, bison_args->lexer_state);
    // TODO to reset parser, ‘YY_FLUSH_BUFFER’ and BEGIN INITIAL after each change to yyin.
    // --> should be ok, because we delete the whole buffer at the beginning, but BEGIN INITIAL may make sense
    return 0;
}


void print_design_list_header()
{
    char buf[42];
    sprintf(buf, "%d", anz_designs);

    fprintf(opt.outfile, "%d Available Style%s in \"%s\":\n",
            anz_designs, anz_designs == 1 ? "" : "s", current_config_file_name);
    fprintf(opt.outfile, "-----------------------%s",
            anz_designs == 1 ? "" : "-");
    for (int i = strlen(current_config_file_name) + strlen(buf); i > 0; --i) {
        fprintf(opt.outfile, "-");
    }
    fprintf(opt.outfile, "\n\n");
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
    fprintf(stderr, "%s: %s: line %d: ", PROJECT,
            current_config_file_name ? current_config_file_name : "(null)",
            yyget_lineno(bargs->lexer_state));
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);

    va_end (ap);

    return 0;
}



static design_t *parse_config_file(const char *config_file, size_t *r_num_designs)
{
    #ifdef DEBUG
        fprintf (stderr, "Parsing Config File %s ...\n", config_file);
    #endif

    pass_to_bison bison_args;
    bison_args.designs = NULL;
    bison_args.num_designs = 0;
    bison_args.design_idx = 0;
    bison_args.config_file = (char *) config_file;
    bison_args.lexer_state = NULL;
    current_bison_args = &bison_args;

    yylex_init (&(bison_args.lexer_state));
    int rc = open_yy_config_file(&bison_args, config_file);
    if (rc) {
        return NULL;
    }
    inflate_inbuf(bison_args.lexer_state, config_file);
    rc = yyparse(&bison_args);
    yylex_destroy(bison_args.lexer_state);

    if (current_config_handle != NULL) {
        fclose(current_config_handle);
        current_config_handle = NULL;
    }

    if (rc) {
        return NULL;
    }
    *r_num_designs = bison_args.num_designs;
    return bison_args.designs;
}



design_t *parse_config_files(const char *first_config_file, size_t *r_num_designs)
{
    // TODO much code

    return parse_config_file(first_config_file, r_num_designs);
}


/*EOF*/                                                  /* vim: set sw=4: */
