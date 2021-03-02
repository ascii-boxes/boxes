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

#ifndef PARSING_H
#define PARSING_H 1

#include "parser.h"


/** file name of config file used */
extern char *config_file_name;

/** names of config files specified via "parent" */
extern char **parent_configs;

/** number of parent config files (size of parent_configs array) */
extern size_t num_parent_configs;



/**
 * Print the header for the design list output, which includes the file name.
 * TODO what if more file names used?
 */
void print_design_list_header();


/**
 * Print configuration file parser errors.
 * @param bison_args pointer to the parser arguments, not used
 * @param fmt a format string for `vfprintf()`, followed by the arguments
 * @return 0
 */
int yyerror(pass_to_bison *bison_args, const char *fmt, ...);


/**
 * Parse the given config file and all parents.
 * @param first_config_file the path to the config file (relative or absolute)
 * @param r_num_designs a return argument that takes the number of design definitions returned from the function.
 *              Will be set to 0 on error
 * @return the consolidated list of designs parsed, or `NULL` on error (then an error message was alread printed)
 */
design_t *parse_config_files(const char *first_config_file, size_t *r_num_designs);


#endif

/*EOF*/                                       /* vim: set cindent sw=4: */
