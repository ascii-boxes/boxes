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
 * Functions related to querying the design list by tag.
 */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "boxes.h"
#include "list.h"
#include "tools.h"
#include "query.h"



#define QUERY_ALL "(all)"
#define QUERY_UNDOC "(undoc)"



static int validate_tag(char *tag)
{
    if (strcmp(tag, QUERY_ALL) == 0 || strcmp(tag, QUERY_UNDOC) == 0) {
        return 1;
    }
    return tag_is_valid(tag);
}



char **parse_query(char *optarg)
{
    /* CAUTION: This function cannot use `opt`, because it is involved in its construction. */

    char **query = NULL;
    char *dup = strdup(optarg);   /* required because strtok() modifies its input */

    int contains_positive_element = 0;
    size_t num_expr = 0;
    for (char *q = strtok(dup, ","); q != NULL; q = strtok(NULL, ","))
    {
        char *trimmed = trimdup(q, q + strlen(q) - 1);
        if (strlen(trimmed) == 0) {
            BFREE(trimmed);
            continue;
        }

        if (trimmed[0] != '-') {
            contains_positive_element = 1;
        }

        char *raw_tag = (trimmed[0] == '+' || trimmed[0] == '-') ? (trimmed + 1) : trimmed;
        if (!validate_tag(raw_tag)) {
            fprintf(stderr, "%s: not a tag -- %s\n", PROJECT, raw_tag);
            BFREE(trimmed);
            BFREE(query);
            return NULL;
        }
        if (query != NULL) {
            for (size_t i = 0; query[i] != NULL; ++i) {
                char *restag = (query[i][0] == '+' || query[i][0] == '-') ? (query[i] + 1) : query[i];
                if (strcasecmp(restag, raw_tag) == 0) {
                    fprintf(stderr, "%s: duplicate query expression -- %s\n", PROJECT, trimmed);
                    BFREE(trimmed);
                    BFREE(query);
                    return NULL;
                }
            }
        }

        ++num_expr;
        query = (char **) realloc(query, (num_expr + 1) * sizeof(char *));
        if (query == NULL) {
            perror(PROJECT);
            break;
        }
        query[num_expr - 1] = trimmed;
        query[num_expr] = NULL;
    }
    BFREE(dup);

    if (num_expr == 0) {
        fprintf(stderr, "%s: empty tag query -- %s\n", PROJECT, optarg);
        return NULL;
    }

    if (!contains_positive_element) {
        ++num_expr;
        query = (char **) realloc(query, (num_expr + 1) * sizeof(char *));
        if (query == NULL) {
            perror(PROJECT);
        }
        else {
            query[num_expr - 1] = QUERY_ALL;
            query[num_expr] = NULL;
        }
    }

    return query;
}



int query_is_undoc()
{
    return opt.query != NULL && strcmp(opt.query[0], QUERY_UNDOC) == 0 && opt.query[1] == NULL;
}



static int filter_by_tag(char **tags)
{
    #ifdef DEBUG
        fprintf(stderr, "filter_by_tag(");
        for (size_t tidx = 0; tags[tidx] != NULL; ++tidx) {
            fprintf(stderr, "%s%s", tidx > 0 ? ", " : "", tags[tidx]);
        }
    #endif

    int result = array_contains0(opt.query, QUERY_ALL);
    if (opt.query != NULL) {
        for (size_t qidx = 0; opt.query[qidx] != NULL; ++qidx) {
            if (opt.query[qidx][0] == '+') {
                result = array_contains0(tags, opt.query[qidx] + 1);
                if (!result) {
                    break;
                }
            }
            else if (opt.query[qidx][0] == '-') {
                if (array_contains0(tags, opt.query[qidx] + 1)) {
                    result = 0;
                    break;
                }
            }
            else if (array_contains0(tags, opt.query[qidx])) {
                result = 1;
            }
        }
    }

    #ifdef DEBUG
        fprintf(stderr, ") -> %d\n", result);
    #endif
    return result;
}



int query_by_tag()
{
    design_t **list = sort_designs_by_name();                 /* temp list for sorting */
    if (list == NULL) {
        return 1;
    }
    for (int i = 0; i < num_designs; ++i) {
        if (filter_by_tag(list[i]->tags)) {
            fprintf(opt.outfile, "%s%s", list[i]->name, opt.eol);
            for (size_t aidx = 0; list[i]->aliases[aidx] != NULL; ++aidx) {
                fprintf(opt.outfile, "%s (alias)%s", list[i]->aliases[aidx], opt.eol);
            }
        }
    }
    BFREE(list);
    return 0;
}


/*EOF*/                                                  /* vim: set sw=4: */
