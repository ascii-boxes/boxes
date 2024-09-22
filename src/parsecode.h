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
 * C code used exclusively by the parser. Include this only in parser.y.
 */

#ifndef PARSECODE_H
#define PARSECODE_H 1

#include "parser.h"


/* return codes of the action_xxx() functions */
#define RC_SUCCESS 0
#define RC_ERROR 1
#define RC_ABORT 2
#define RC_ACCEPT 3

/** the current design being parsed */
#define curdes (bison_args->designs[bison_args->design_idx])



/**
 * Initialize parser data structures.
 * @param bison_args the parser state
 * @return 0: success;
 *         2: YYABORT must be invoked
 */
int action_init_parser(pass_to_bison *bison_args);


/**
 * Rule action called when a shape list was fully parsed.
 * @param bison_args the parser state
 * @return 0: success;
 *         1: YYERROR must be invoked;
 *         2: YYABORT must be invoked
 */
int action_finalize_shapes(pass_to_bison *bison_args);


/**
 * Rule action called when a new box design is starting to be parsed.
 * @param bison_args the parser state
 * @param design_name the primary name of the design
 * @return 0: success;
 *         1: YYERROR must be invoked;
 *         2: YYABORT must be invoked
 */
int action_start_parsing_design(pass_to_bison *bison_args, char *design_name);


/**
 * Rule action called when a parent config file specification is encountered.
 * @param bison_args the parser state
 * @param filepath the path to the parent config file
 * @return 0: success;
 *         1: YYERROR must be invoked
 */
int action_parent_config(pass_to_bison *bison_args, bxstr_t *filepath);


/**
 * Rule action called when a design has been parsed completely and can be checked and added.
 * @param bison_args the parser state
 * @param design_primary_name the primary name of the design as specified at BOX statement
 * @param name_at_end the primary name of the design as specified at END statement
 * @return 0: success;
 *         1: YYERROR must be invoked;
 *         2: YYABORT must be invoked;
 *         3: YYACCEPT must be invoked
 */
int action_add_design(pass_to_bison *bison_args, char *design_primary_name, char *name_at_end);


/**
 * Rule action called when a keyword entry is encountered in a box design.
 * @param bison_args the parser state
 * @param keyword the keyword
 * @param value the associated value
 * @return 0: success;
 *         1: YYERROR must be invoked
 *         2: YYABORT must be invoked
 */
int action_record_keyword(pass_to_bison *bison_args, char *keyword, bxstr_t *value);


/**
 * Rule action called when an area spec within a padding definition is parsed.
 * @param bison_args the parser state
 * @param area the padding area specified
 * @param value the associated padding value
 * @return currently always 0 (success), but could return any `RC_*`
 */
int action_padding_entry(pass_to_bison *bison_args, char *area, int value);


/**
 * Rule action called to add an alias name to the current design definition.
 * @param bison_args the parser state
 * @param alias_name the alias name
 * @return 0: success;
 *         1: YYERROR must be invoked
 */
int action_add_alias(pass_to_bison *bison_args, char *alias_name);


/**
 * Rule action to add a `replace` or `reverse` rule to the current design definition.
 * @param bison_args the parser state
 * @param type the rule type (either `rep` or `rev`)
 * @param rule_list pointer to the list to add to
 * @param rule_list_len pointer to the number of elements in `rule_list`
 * @param search the search pattern of the rule
 * @param replace the replacement text of the rule
 * @param mode the replacement mode (`g` or `o` for `global` or `once`)
 * @return 0: success;
 *         1: YYERROR must be invoked
 *         2: YYABORT must be invoked
 */
int action_add_regex_rule(pass_to_bison *bison_args, char *type, reprule_t **rule_list, size_t *rule_list_len,
    bxstr_t *search, bxstr_t *replace, char mode);


/**
 * (shape-elastic check)
 */
int perform_se_check(pass_to_bison *bison_args);


/**
 * Reset parser to neutral state, so a new design can be parsed.
 * @param bison_args the parser state
 */
void recover(pass_to_bison *bison_args);


/**
 * Add tag to list of current design's tags after validity checking.
 * @param bison_args the parser state
 * @param tag a single tag, or a comma-separated list of tags
 * @return error code, 0 on success, anything else on failure
 */
int tag_record(pass_to_bison *bison_args, bxstr_t *tag);


/**
 * Add the sample block to the current design.
 * @param bison_args the parser state
 * @param sample the sample block content (non-empty when this is invoked)
 * @return 0: success;
 *         1: YYERROR must be invoked;
 *         2: YYABORT must be invoked
 */
int action_sample_block(pass_to_bison *bison_args, bxstr_t *sample);


/**
 * Start a new shape by adding its first line.
 * @param bison_args the parser state
 * @param line the first line of the new shape
 * @param shape the address where the new shape shall be stored
 * @return 0: success;
 *         1: YYERROR must be invoked;
 *         2: YYABORT must be invoked
 */
int action_first_shape_line(pass_to_bison *bison_args, bxstr_t *line, sentry_t *shape);


/**
 * Extend a shape by adding another line.
 * @param bison_args the parser state
 * @param shape the address of the shape to be extended
 * @param line the new line
 * @return 0: success;
 *         1: YYERROR must be invoked;
 *         2: YYABORT must be invoked
 */
int action_add_shape_line(pass_to_bison *bison_args, sentry_t *shape, bxstr_t *line);


#endif

/* vim: set cindent sw=4: */
