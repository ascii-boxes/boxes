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
 * List design information about a single design or about all designs.
 */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "boxes.h"
#include "parsing.h"
#include "query.h"
#include "tools.h"
#include "list.h"



typedef struct {
    char  *tag;
    size_t count;
} tagstats_t;



static void analyze_names(design_t *design, size_t *r_num_aliases, size_t *r_num_chars)
{
    size_t siz = strlen(design->name);
    size_t num_aliases = 0;
    while(design->aliases[num_aliases] != NULL) {
        siz += strlen(design->aliases[num_aliases]);
        if (num_aliases > 0) {
            siz += 2;   // ", "
        }
        ++num_aliases;
    }
    if (num_aliases > 0) {
        siz += 7;   // " alias "
    }

    *r_num_aliases = num_aliases;
    *r_num_chars = siz;
}



/**
 * Create a string listing all the names of a design, for example `"ada-cmt alias lua-cmt, sql-cmt"`.
 * @param design pointer to the design
 * @return the string of names, for which memory was allocated
 */
static char *names(design_t *design)
{
    size_t siz;
    size_t num_aliases;
    analyze_names(design, &num_aliases, &siz);

    char *result = (char *) malloc (siz + 1);
    if (result == NULL) {
        perror(PROJECT);
        return NULL;
    }
    strcpy(result, design->name);

    if (num_aliases > 0) {
        char *p = result + strlen(design->name);
        strcpy(p, " alias ");
        p += 7;   // " alias "

        for (size_t aidx = 0; design->aliases[aidx] != NULL; ++aidx) {
            strcpy(p, design->aliases[aidx]);
            p += strlen(design->aliases[aidx]);

            if (aidx < num_aliases - 1) {
                strcpy(p, ", ");
                p += 2;   // ", "
            }
        }
    }
    return result;
}



static int style_sort(const void *p1, const void *p2)
{
    return strcasecmp((const char *) ((*((design_t **) p1))->name),
                      (const char *) ((*((design_t **) p2))->name));
}



design_t **sort_designs_by_name()
{
    design_t **result = (design_t **) malloc(num_designs * sizeof(design_t *));
    if (result != NULL) {
        for (int i = 0; i < num_designs; ++i) {
            result[i] = &(designs[i]);
        }
        qsort(result, num_designs, sizeof(design_t *), style_sort);
    }
    else {
        perror(PROJECT);
    }
    return result;
}



static void count_tag(char *tag, tagstats_t **tagstats, size_t *num_tags)
{
    if (*tagstats != NULL) {
        for (size_t i = 0; i < (*num_tags); ++i) {
            if (strcasecmp((*tagstats)[i].tag, tag) == 0) {
                ++((*tagstats)[i].count);
                return;
            }
        }
    }

    ++(*num_tags);
    *tagstats = realloc(*tagstats, (*num_tags) * sizeof(tagstats_t));
    (*tagstats)[(*num_tags) - 1].tag = tag;
    (*tagstats)[(*num_tags) - 1].count = 1;
}



static int tagstats_sort(const void *p1, const void *p2)
{
    return strcasecmp(((tagstats_t *) p1)->tag, ((tagstats_t *) p2)->tag);
}



static void print_tags(tagstats_t *tagstats, size_t num_tags)
{
    if (tagstats != NULL) {
        qsort(tagstats, num_tags, sizeof(tagstats_t), tagstats_sort);
        for (size_t tidx = 0; tidx < num_tags; ++tidx) {
            if (tidx > 0) {
                fprintf(opt.outfile, " | ");
            }
            fprintf(opt.outfile, "%s (%d)", tagstats[tidx].tag, (int) tagstats[tidx].count);
        }
        fprintf(opt.outfile, "%s", opt.eol);
    }
}



static char *escape(const char *org, const int pLength)
{
    char *result = (char *) calloc(1, 2 * strlen(org) + 1);
    int orgIdx, resultIdx;
    for (orgIdx = 0, resultIdx = 0; orgIdx < pLength; ++orgIdx, ++resultIdx) {
        if (org[orgIdx] == '\\' || org[orgIdx] == '"') {
            result[resultIdx++] = '\\';
        }
        result[resultIdx] = org[orgIdx];
    }
    result[resultIdx] = '\0';
    return result;
}



static void print_design_details(design_t *d)
{
    fprintf(opt.outfile, "Complete Design Information for \"%s\":%s", d->name, opt.eol);
    fprintf(opt.outfile, "-----------------------------------");
    for (int i = strlen(d->name); i > 0; --i) {
        fprintf(opt.outfile, "-");
    }
    fprintf(opt.outfile, "%s", opt.eol);

    fprintf(opt.outfile, "Alias Names:            ");
    size_t aidx = 0;
    while(d->aliases[aidx] != NULL) {
        fprintf(opt.outfile, "%s%s", aidx > 0 ? ", " : "", d->aliases[aidx]);
        ++aidx;
    }
    if (aidx == 0) {
        fprintf(opt.outfile, "none");
    }
    fprintf(opt.outfile, "%s", opt.eol);

    fprintf(opt.outfile, "Author:                 %s%s", d->author ? d->author : "(unknown author)", opt.eol);
    fprintf(opt.outfile, "Original Designer:      %s%s", d->designer ? d->designer : "(unknown artist)", opt.eol);
    fprintf(opt.outfile, "Creation Date:          %s%s", d->created ? d->created : "(unknown)", opt.eol);

    fprintf(opt.outfile, "Current Revision:       %s%s%s%s",
            d->revision ? d->revision : "",
            d->revision && d->revdate ? " as of " : "",
            d->revdate ? d->revdate : (d->revision ? "" : "(unknown)"), opt.eol);

    fprintf(opt.outfile, "Configuration File:     %s%s", d->defined_in, opt.eol);

    fprintf(opt.outfile, "Indentation Mode:       ");
    switch (d->indentmode) {
        case 'b':
            fprintf(opt.outfile, "box (indent box)%s", opt.eol);
            break;
        case 't':
            fprintf(opt.outfile, "text (retain indentation inside of box)%s", opt.eol);
            break;
        default:
            fprintf(opt.outfile, "none (discard indentation)%s", opt.eol);
            break;
    }

    fprintf(opt.outfile, "Replacement Rules:      ");
    if (d->anz_reprules > 0) {
        for (int i = 0; i < (int) d->anz_reprules; ++i) {
            fprintf(opt.outfile, "%d. (%s) \"%s\" WITH \"%s\"%s", i + 1,
                    d->reprules[i].mode == 'g' ? "glob" : "once",
                    d->reprules[i].search, d->reprules[i].repstr, opt.eol);
            if (i < (int) d->anz_reprules - 1) {
                fprintf(opt.outfile, "                        ");
            }
        }
    }
    else {
        fprintf(opt.outfile, "none%s", opt.eol);
    }
    fprintf(opt.outfile, "Reversion Rules:        ");
    if (d->anz_revrules > 0) {
        for (int i = 0; i < (int) d->anz_revrules; ++i) {
            fprintf(opt.outfile, "%d. (%s) \"%s\" TO \"%s\"%s", i + 1,
                    d->revrules[i].mode == 'g' ? "glob" : "once",
                    d->revrules[i].search, d->revrules[i].repstr, opt.eol);
            if (i < (int) d->anz_revrules - 1) {
                fprintf(opt.outfile, "                        ");
            }
        }
    }
    else {
        fprintf(opt.outfile, "none%s", opt.eol);
    }

    fprintf(opt.outfile, "Minimum Box Dimensions: %d x %d  (width x height)%s",
            (int) d->minwidth, (int) d->minheight, opt.eol);

    fprintf(opt.outfile, "Default Padding:        ");
    if (d->padding[BTOP] || d->padding[BRIG] || d->padding[BBOT] || d->padding[BLEF]) {
        if (d->padding[BLEF]) {
            fprintf(opt.outfile, "left %d", d->padding[BLEF]);
            if (d->padding[BTOP] || d->padding[BRIG] || d->padding[BBOT]) {
                fprintf(opt.outfile, ", ");
            }
        }
        if (d->padding[BTOP]) {
            fprintf(opt.outfile, "top %d", d->padding[BTOP]);
            if (d->padding[BRIG] || d->padding[BBOT]) {
                fprintf(opt.outfile, ", ");
            }
        }
        if (d->padding[BRIG]) {
            fprintf(opt.outfile, "right %d", d->padding[BRIG]);
            if (d->padding[BBOT]) {
                fprintf(opt.outfile, ", ");
            }
        }
        if (d->padding[BBOT]) {
            fprintf(opt.outfile, "bottom %d", d->padding[BBOT]);
        }
        fprintf(opt.outfile, "%s", opt.eol);
    }
    else {
        fprintf(opt.outfile, "none%s", opt.eol);
    }

    fprintf(opt.outfile, "Default Killblank:      %s%s",
            empty_side(opt.design->shape, BTOP) && empty_side(opt.design->shape, BBOT) ? "no" : "yes", opt.eol);

    fprintf(opt.outfile, "Tags:                   ");
    size_t tidx = 0;
    while(d->tags[tidx] != NULL) {
        fprintf(opt.outfile, "%s%s", tidx > 0 ? ", " : "", d->tags[tidx]);
        ++tidx;
    }
    if (tidx == 0) {
        fprintf(opt.outfile, "none");
    }
    fprintf(opt.outfile, "%s", opt.eol);

    fprintf(opt.outfile, "Elastic Shapes:         ");
    int sstart = 0;
    for (int i = 0; i < NUM_SHAPES; ++i) {
        if (isempty(d->shape + i)) {
            continue;
        }
        if (d->shape[i].elastic) {
            fprintf(opt.outfile, "%s%s", sstart ? ", " : "", shape_name[i]);
            sstart = 1;
        }
    }
    fprintf(opt.outfile, "%s", opt.eol);

    /*
     *  Display all shapes
     */
    if (query_is_undoc()) {
        fprintf(opt.outfile, "Sample:%s%s%s", opt.eol, d->sample, opt.eol);
    }
    else {
        int first_shape = 1;
        for (int i = 0; i < NUM_SHAPES; ++i) {
            if (isdeepempty(d->shape + i)) {
                continue;
            }
            for (size_t w = 0; w < d->shape[i].height; ++w) {
                char *escaped_line = escape(d->shape[i].chars[w], d->shape[i].width);
                fprintf(opt.outfile, "%-24s%3s%c \"%s\"%c%s",
                        (first_shape == 1 && w == 0 ? "Defined Shapes:" : ""),
                        (w == 0 ? shape_name[i] : ""), (w == 0 ? ':' : ' '),
                        escaped_line,
                        (w < d->shape[i].height - 1 ? ',' : ' '),
                        opt.eol
                );
                BFREE (escaped_line);
            }
            first_shape = 0;
        }
    }
}



int list_designs()
{
    if (opt.design_choice_by_user) {
        print_design_details(opt.design);
    }

    else {
        tagstats_t *tagstats = NULL;
        size_t num_tags = 0;
        design_t **list = sort_designs_by_name();     /* temp list for sorting */
        if (list == NULL) {
            return 1;
        }
        print_design_list_header();

        for (int i = 0; i < num_designs; ++i) {
            char *all_names = names(list[i]);
            if (list[i]->author && list[i]->designer && strcmp(list[i]->author, list[i]->designer) != 0) {
                fprintf(opt.outfile, "%s%s%s, coded by %s:%s%s%s%s%s", all_names, opt.eol,
                        list[i]->designer, list[i]->author, opt.eol, opt.eol,
                        list[i]->sample, opt.eol, opt.eol);
            }
            else if (list[i]->designer) {
                fprintf(opt.outfile, "%s%s%s:%s%s%s%s%s", all_names, opt.eol,
                        list[i]->designer, opt.eol, opt.eol,
                        list[i]->sample, opt.eol, opt.eol);
            }
            else if (list[i]->author) {
                fprintf(opt.outfile, "%s%sunknown artist, coded by %s:%s%s%s%s%s", all_names, opt.eol,
                        list[i]->author, opt.eol, opt.eol,
                        list[i]->sample, opt.eol, opt.eol);
            }
            else {
                fprintf(opt.outfile, "%s:%s%s%s%s%s", all_names, opt.eol, opt.eol,
                        list[i]->sample, opt.eol, opt.eol);
            }
            BFREE(all_names);

            for (size_t tidx = 0; list[i]->tags[tidx] != NULL; ++tidx) {
                count_tag(list[i]->tags[tidx], &tagstats, &num_tags);
            }
        }
        BFREE(list);

        print_tags(tagstats, num_tags);
        BFREE(tagstats);
    }

    return 0;
}


/*EOF*/                                                  /* vim: set sw=4: */
