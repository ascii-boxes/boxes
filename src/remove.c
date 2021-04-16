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
 * Box removal, i.e. the deletion of boxes
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistr.h>

#include "shape.h"
#include "boxes.h"
#include "tools.h"
#include "unicode.h"
#include "remove.h"



static int best_match(const line_t *line,
                      char **ws, char **we, char **es, char **ee)
/*
 *  Find positions of west and east box parts in line.
 *
 *    line      line to examine
 *    ws etc.   result parameters (west start, west end, east start, east end)
 *
 *  RETURNS:    > 0   a match was found (ws etc are set to indicate positions)
 *             == 0   no match was found
 *              < 0   internal error (out of memory)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t numw = 0;             /* number of shape lines on west side */
    size_t nume = 0;             /* number of shape lines on east side */
    size_t j;                    /* counts number of lines of all shapes tested */
    size_t k;                    /* line counter within shape */
    int w;                       /* shape counter */
    sentry_t *cs;                /* current shape */
    char *s;                     /* duplicate of current shape part */
    char *p;                     /* position found by strstr */
    size_t cq;                   /* current quality */
    char *q;                     /* space check rover */
    line_t chkline;              /* for calls to empty_line() */
    size_t quality;

    *ws = *we = *es = *ee = NULL;

    numw = opt.design->shape[WNW].height;
    numw += opt.design->shape[W].height;
    numw += opt.design->shape[WSW].height;

    nume = opt.design->shape[ENE].height;
    nume += opt.design->shape[E].height;
    nume += opt.design->shape[ESE].height;

    #ifdef DEBUG
    fprintf (stderr, "Number of WEST side shape lines: %d\n", (int) numw);
    fprintf (stderr, "Number of EAST side shape lines: %d\n", (int) nume);
    #endif

    /*
     *  Find match for WEST side
     */
    if (!empty_side(opt.design->shape, BLEF)) {
        quality = 0;
        cs = opt.design->shape + WNW;
        for (j = 0, k = 0, w = 3; j < numw; ++j, ++k) {
            if (k == cs->height) {
                k = 0;
                cs = opt.design->shape + west_side[--w];
            }

            chkline.text = cs->chars[k];
            chkline.len = cs->width;
            if (empty_line(&chkline) && !(quality == 0 && j == numw - 1)) {
                continue;
            }

            s = (char *) strdup(cs->chars[k]);
            if (s == NULL) {
                perror(PROJECT);
                return -1;
            }
            cq = cs->width;

            do {
                p = strstr(line->text, s);
                if (p) {
                    q = p - 1;
                    while (q >= line->text) {
                        if (*q-- != ' ') {
                            p = NULL;
                            break;
                        }
                    }
                    if (p) {
                        break;
                    }
                }
                if (!p && cq) {
                    if (*s == ' ') {
                        memmove(s, s + 1, cq--);
                    } else if (s[cq - 1] == ' ') {
                        s[--cq] = '\0';
                    } else {
                        cq = 0;
                        break;
                    }
                }
            } while (cq && !p);

            if (cq == 0) {
                BFREE (s);
                continue;
            }

            /*
            *  If the current match is the best yet, adjust result values
            */
            if (cq > quality) {
                quality = cq;
                *ws = p;
                *we = p + cq;
            }

            BFREE (s);
        }
    }

    /*
     *  Find match for EAST side
     */
    if (!empty_side(opt.design->shape, BRIG)) {
        quality = 0;
        cs = opt.design->shape + ENE;
        for (j = 0, k = 0, w = 1; j < nume; ++j, ++k) {
            if (k == cs->height) {
                k = 0;
                cs = opt.design->shape + east_side[++w];
            }
            #ifdef DEBUG
                fprintf(stderr, "\nj %d, k %d, w %d, cs->chars[k] = \"%s\"\n",
                        (int) j, (int) k, w, cs->chars[k] ? cs->chars[k] : "(null)");
            #endif

            chkline.text = cs->chars[k];
            chkline.len = cs->width;
            if (empty_line(&chkline)) {
                continue;
            }

            s = (char *) strdup(cs->chars[k]);
            if (s == NULL) {
                perror(PROJECT);
                return -1;
            }
            cq = cs->width;

            do {
                p = my_strnrstr(line->text, s, cq, 0);
                if (p) {
                    q = p + cq;
                    while (*q) {
                        if (*q++ != ' ') {
                            p = NULL;
                            break;
                        }
                    }
                    if (p) {
                        break;
                    }
                }
                if (!p && cq) {
                    if (*s == ' ') {
                        memmove(s, s + 1, cq--);
                    } else if (s[cq - 1] == ' ') {
                        s[--cq] = '\0';
                    } else {
                        cq = 0;
                        break;
                    }
                }
            } while (cq && !p);

            if (cq == 0) {
                BFREE (s);
                continue;
            }

            /*
            *  If the current match is the best yet, adjust result values
            */
            if (cq > quality) {
                quality = cq;
                *es = p;
                *ee = p + cq;
            }

            BFREE (s);
        }
    }

    return *ws || *es ? 1 : 0;
}



static int hmm(const int aside, const size_t follow,
               const char *p, const char *ecs, const int cnt)
/*
 *  (horizontal middle match)
 *
 *      aside   box part to check (BTOP or BBOT)
 *      follow  index of line number in shape spec to check
 *      p       current check position
 *      ecs     pointer to first char of east corner shape
 *      cnt     current shape to check (0 == leftmost middle shape)
 *
 *  Recursive helper function for detect_horiz(), uses backtracking
 *
 *  RETURNS:  == 0  success
 *            != 0  error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int cmp;
    sentry_t *cs;
    shape_t sh;
    int rc;

    #ifdef DEBUG
        fprintf(stderr, "hmm (%s, %d, \'%c\', \'%c\', %d)\n",
                aside == BTOP ? "BTOP" : "BBOT", (int) follow, p[0], *ecs, cnt);
    #endif

    if (p > ecs) {                         /* last shape tried was too long */
        return 2;
    }

    sh = leftmost(aside, cnt);
    if (sh == NUM_SHAPES) {
        return 1;
    }

    cs = opt.design->shape + sh;

    cmp = strncmp(p, cs->chars[follow], cs->width);

    if (cmp == 0) {
        if (p + cs->width == ecs) {
            if (leftmost(aside, cnt + 1) == NUM_SHAPES) {
                return 0;                /* good! all clear, it matched */
            } else {
                return 3;
            }                /* didn't use all shapes to do it */
        }
        if (cs->elastic) {
            rc = hmm(aside, follow, p + cs->width, ecs, cnt);
            #ifdef DEBUG
            fprintf (stderr, "hmm returned %d\n", rc);
            #endif
            if (rc) {
                rc = hmm(aside, follow, p + cs->width, ecs, cnt + 1);
                #ifdef DEBUG
                fprintf (stderr, "hmm returned %d\n", rc);
                #endif
            }
        }
        else {
            rc = hmm(aside, follow, p + cs->width, ecs, cnt + 1);
            #ifdef DEBUG
            fprintf (stderr, "hmm returned %d\n", rc);
            #endif
        }
        if (rc == 0) {
            return 0;                    /* we're on the way back */
        } else {
            return 4;
        }                    /* can't continue on this path */
    }
    else {
        return 5;                        /* no match */
    }
}



static int detect_horiz(const int aside, size_t *hstart, size_t *hend)
/*
 *  Detect which part of the input belongs to the top/bottom of the box
 *
 *      aside   part of box to detect (BTOP or BBOT)
 *      hstart  index of first line of detected box part (result)
 *      hend    index of first line following detected box part (result)
 *
 *  We assume the horizontal parts of the box to be in one piece, i.e. no
 *  blank lines inserted. Lines may be missing, though. Lines may not be
 *  duplicated. They may be shifted left and right by inserting whitespace,
 *  but whitespace which is part of the box must not have been deleted
 *  (unless it's because an entire box side is empty). Box part lines may
 *  even differ in length as long as each line is in itself a valid
 *  horizontal box line.
 *
 *  RETURNS:  == 0   success (hstart & hend are set)
 *            != 0   error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t follow;                 /* possible box line */
    sentry_t *cs;                  /* current shape */
    line_t *line;                  /* currently processed input line */
    size_t lcnt;                   /* line counter */
    char *p = NULL;                /* middle line part scanner */
    char *q;                       /* space check rover */
    char *wcs = NULL;              /* west corner shape position */
    char *ecs = NULL;              /* east corner shape position */
    int mmok = 0;                  /* true if middle match was ok */
    size_t mheight;                /* regular height of box part */
    int result_init = 0;           /* true if hstart etc. was init. */
    int nowside;                   /* true if west side is empty */
    int goeast;                    /* no. of finds to ignore on right */
    int gowest;                    /* set to request search start incr. */

    *hstart = *hend = 0;
    nowside = empty_side(opt.design->shape, BLEF);

    mheight = opt.design->shape[sides[aside][0]].height;
    if (aside == BTOP) {
        follow = 0;
        line = input.lines;
    }
    else {
        follow = mheight - 1;
        line = input.lines + input.num_lines - 1;
    }

    for (lcnt = 0; lcnt < mheight && lcnt < input.num_lines
            && line >= input.lines; ++lcnt) {
        goeast = gowest = 0;

        #ifdef DEBUG
            fprintf(stderr, "----- Processing line index %2d -----------------------------------------------\n",
                    (int) (aside == BTOP ? lcnt : input.num_lines - lcnt - 1));
        #endif

        do {
            /*
             *  Look for west corner shape
             */
            if (!goeast) {
                if (nowside) {
                    wcs = NULL;
                    if (gowest) {
                        gowest = 0;
                        if (*p == ' ' || *p == '\t') {
                            ++p;
                        } else {
                            break;
                        }
                    }
                    else {
                        p = line->text;
                    }
                }
                else {
                    cs = opt.design->shape + sides[aside][aside == BTOP ? 0 : SHAPES_PER_SIDE - 1];
                    if (gowest) {
                        gowest = 0;
                        wcs = strstr(wcs + 1, cs->chars[follow]);
                    }
                    else {
                        wcs = strstr(line->text, cs->chars[follow]);
                    }
                    if (wcs) {
                        for (q = wcs - 1; q >= line->text; --q) {
                            if (*q != ' ' && *q != '\t') {
                                break;
                            }
                        }
                        if (q >= line->text) {
                            wcs = NULL;
                        }
                    }
                    if (!wcs) {
                        break;
                    }

                    p = wcs + cs->width;
                }
            }
            /* Now, wcs is either NULL (if west side is empty)         */
            /* or not NULL (if west side is not empty). In any case, p */
            /* points to where we start searching for the east corner. */
            #ifdef DEBUG
                if (wcs) {
                    fprintf(stderr, "West corner shape matched at position %d.\n", (int) (wcs - line->text));
                } else {
                    fprintf(stderr, "West box side is empty.\n");
                }
            #endif
            /*
             *  Look for east corner shape
             */
            cs = opt.design->shape + sides[aside][aside == BTOP ? SHAPES_PER_SIDE - 1 : 0];
            ecs = my_strnrstr(p, cs->chars[follow], cs->width, goeast);
            if (ecs) {
                for (q = ecs + cs->width; *q; ++q) {
                    if (*q != ' ' && *q != '\t') {
                        break;
                    }
                }
                if (*q) {
                    ecs = NULL;
                }
            }
            if (!ecs) {
                if (goeast == 0) {
                    break;
                } else {
                    goeast = 0;
                    gowest = 1;
                    continue;
                }
            }
            #ifdef DEBUG
                fprintf(stderr, "East corner shape matched at position %d.\n", (int) (ecs - line->text));
            #endif

            /*
             *  Check if text between corner shapes is valid
             */
            mmok = !hmm(aside, follow, p, ecs, 0);
            if (!mmok) {
                ++goeast;
            }
            #ifdef DEBUG
                fprintf(stderr, "Text between corner shapes is %s.\n", mmok ? "VALID" : "NOT valid");
            #endif
        } while (!mmok);

        /*
         *  Proceed to next line
         */
        if (mmok) {                      /* match found */
            if (!result_init) {
                result_init = 1;
                if (aside == BTOP) {
                    *hstart = lcnt;
                } else {
                    *hend = (input.num_lines - lcnt - 1) + 1;
                }
            }
            if (aside == BTOP) {
                *hend = lcnt + 1;
            } else {
                *hstart = input.num_lines - lcnt - 1;
            }
        }
        else {
            if (result_init) {
                break;
            }
        }

        wcs = NULL;
        ecs = NULL;
        p = NULL;
        mmok = 0;

        if (aside == BTOP) {
            ++follow;
            ++line;
        }
        else {
            --follow;
            --line;
        }
    }

    return result_init ? 0 : 1;
}



static design_t *detect_design()
/*
 *  Autodetect design used by box in input.
 *
 *  This requires knowledge about ALL designs, so the entire config file had
 *  to be parsed at some earlier time.
 *
 *  RETURNS:  != NULL   success, pointer to detected design
 *            == NULL   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    design_t *d = designs;         /* ptr to currently tested design */
    long hits;                     /* hit points of the current design */
    long maxhits = 0;              /* maximum no. of hits so far */
    design_t *res = NULL;          /* ptr to design with the most hits */
    int dcnt;                      /* design loop counter */
    shape_t scnt;                  /* shape loop counter */
    size_t j, k;
    char *p;
    char *s;
    line_t shpln;                  /* a line which is part of a shape */
    size_t a;
    int empty[NUM_SIDES];

    for (dcnt = 0; dcnt < num_designs; ++dcnt, ++d) {
        #ifdef DEBUG
        fprintf (stderr, "CONSIDERING DESIGN ---- \"%s\" ---------------\n",
                d->name);
        #endif
        hits = 0;

        for (j = 0; j < NUM_SIDES; ++j) {
            empty[j] = empty_side(d->shape, j);
        }
        #ifdef DEBUG
        fprintf (stderr, "Empty sides: TOP %d, LEFT %d, BOTTOM %d, RIGHT %d\n",
                empty[BTOP], empty[BLEF], empty[BBOT], empty[BRIG]);
        #endif

        for (scnt = 0; scnt < NUM_SHAPES; ++scnt) {

            switch (scnt) {
                case NW:
                case SW:
                    /*
                     *  Try and find west corner shapes. Every non-empty shape
                     *  line is searched for on every input line. A hit is
                     *  generated whenever a match is found.
                     */
                    if (empty[BLEF] || (empty[BTOP] && scnt == NW)
                            || (empty[BBOT] && scnt == SW)) {
                        break;
                    }
                    for (j = 0; j < d->shape[scnt].height; ++j) {
                        shpln.text = d->shape[scnt].chars[j];
                        shpln.len = d->shape[scnt].width;
                        if (empty_line(&shpln)) {
                            continue;
                        }
                        for (s = shpln.text; *s == ' ' || *s == '\t'; ++s) {
                        }
                        for (k = 0; k < d->shape[scnt].height; ++k) {
                            a = k;
                            if (scnt == SW) {
                                a += input.num_lines - d->shape[scnt].height;
                            }
                            if (a >= input.num_lines) {
                                break;
                            }
                            for (p = input.lines[a].text; *p == ' ' || *p == '\t'; ++p) {
                            }
                            if (strncmp(p, s, shpln.len - (s - shpln.text)) == 0) {
                                ++hits;
                            }
                        }
                    }
#ifdef DEBUG
                    fprintf (stderr, "After %s corner check:\t%ld hits.\n", shape_name[scnt], hits);
#endif
                    break;

                case NE:
                case SE:
                    /*
                     *  Try and find east corner shapes. Every non-empty shape
                     *  line is searched for on every input line. A hit is
                     *  generated whenever a match is found.
                     */
                    if (empty[BRIG] || (empty[BTOP] && scnt == NE)
                            || (empty[BBOT] && scnt == SE)) {
                        break;
                    }
                    for (j = 0; j < d->shape[scnt].height; ++j) {
                        shpln.text = d->shape[scnt].chars[j];
                        shpln.len = d->shape[scnt].width;
                        if (empty_line(&shpln)) {
                            continue;
                        }
                        for (s = shpln.text + shpln.len - 1;
                                (*s == ' ' || *s == '\t') && shpln.len;
                                --s, --(shpln.len)) {
                        }
                        for (k = 0; k < d->shape[scnt].height; ++k) {
                            a = k;
                            if (scnt == SE) {
                                a += input.num_lines - d->shape[scnt].height;
                            }
                            if (a >= input.num_lines) {
                                break;
                            }
                            for (p = input.lines[a].text + input.lines[a].len - 1;
                                    p >= input.lines[a].text && (*p == ' ' || *p == '\t');
                                    --p) {
                            }
                            p = p - shpln.len + 1;
                            if (p < input.lines[a].text) {
                                continue;
                            }
                            if (strncmp(p, shpln.text, shpln.len) == 0) {
                                ++hits;
                            }
                        }
                    }
#ifdef DEBUG
                    fprintf (stderr, "After %s corner check:\t%ld hits.\n", shape_name[scnt], hits);
#endif
                    break;

                default:
                    if (isempty(d->shape + scnt)) {
                        continue;
                    }

                    if ((scnt >= NNW && scnt <= NNE)
                            || (scnt >= SSE && scnt <= SSW)) {
                        /*
                         *  Try and find horizontal shapes between the box
                         *  corners. Every non-empty shape line is searched for
                         *  on every input line. Elastic shapes must occur
                         *  twice in an uninterrupted row to generate a hit.
                         */
                        if ((scnt >= NNW && scnt <= NNE && empty[BTOP])
                                || (scnt >= SSE && scnt <= SSW && empty[BBOT])) {
                            ++hits;
                            break;       /* horizontal box part is empty */
                        }
                        for (j = 0; j < d->shape[scnt].height; ++j) {
                            shpln.text = d->shape[scnt].chars[j];
                            shpln.len = d->shape[scnt].width;
                            if (empty_line(&shpln)) {
                                continue;
                            }
                            for (k = 0; k < d->shape[scnt].height; ++k) {
                                a = k;
                                if (scnt >= SSE && scnt <= SSW) {
                                    a += input.num_lines - d->shape[scnt].height;
                                }
                                if (a >= input.num_lines) {
                                    break;
                                }
                                for (p = input.lines[a].text;
                                        *p == ' ' || *p == '\t'; ++p) {
                                }
                                p += d->shape[NW].width;
                                if (p - input.lines[a].text
                                        >= (long) input.lines[a].len) {
                                    continue;
                                }
                                p = strstr(p, shpln.text);
                                if (p) {
                                    if (d->shape[scnt].elastic) {
                                        p += shpln.len;
                                        if (p - input.lines[a].text
                                                >= (long) input.lines[a].len) {
                                            continue;
                                        }
                                        if (!strncmp(p, shpln.text, shpln.len)) {
                                            ++hits;
                                        }
                                    }
                                    else {
                                        ++hits;
                                    }
                                }
                            }
                        }
                    }

                    else if ((scnt >= ENE && scnt <= ESE)
                            || (scnt >= WSW && scnt <= WNW)) {
                        /* handle later */
                        break;
                    }
                    else {
                        fprintf(stderr, "%s: internal error\n", PROJECT);
                        return NULL;
                    }
                #ifdef DEBUG
                fprintf (stderr, "After %s shape check:\t%ld hits.\n",
                        shape_name[scnt], hits);
                #endif
            }
        }

        /*
         *  Now iterate over all input lines except for potential top and
         *  bottom box parts. Check if east and west line ends match a
         *  non-empty shape line. If so, generate a hit.
         */
        if (((empty[BTOP] ? 0 : d->shape[NW].height)
                + (empty[BBOT] ? 0 : d->shape[SW].height)) < input.num_lines) {
            for (k = empty[BTOP] ? 0 : d->shape[NW].height;
                    k < input.num_lines - (empty[BBOT] ? 0 : d->shape[SW].height);
                    ++k) {
                for (p = input.lines[k].text; *p == ' ' || *p == '\t'; ++p) {
                }
                for (scnt = WSW; scnt <= WNW; ++scnt) {
                    a = 0;
                    if (isempty(d->shape + scnt)) {
                        continue;
                    }
                    for (j = 0; j < d->shape[scnt].height; ++j) {
                        shpln.text = d->shape[scnt].chars[j];
                        shpln.len = d->shape[scnt].width;
                        if (empty_line(&shpln)) {
                            continue;
                        }
                        for (s = shpln.text; *s == ' ' || *s == '\t'; ++s) {
                        }
                        if (strncmp(p, s, shpln.len - (s - shpln.text)) == 0) {
                            ++hits;
                            a = 1;
                            break;
                        }
                    }
                    if (a) {
                        break;
                    }
                }

                for (scnt = ENE; scnt <= ESE; ++scnt) {
                    a = 0;
                    if (isempty(d->shape + scnt)) {
                        continue;
                    }
                    for (j = 0; j < d->shape[scnt].height; ++j) {
                        shpln.text = d->shape[scnt].chars[j];
                        shpln.len = d->shape[scnt].width;
                        if (empty_line(&shpln)) {
                            continue;
                        }
                        for (p = input.lines[k].text + input.lines[k].len - 1;
                                p >= input.lines[a].text && (*p == ' ' || *p == '\t');
                                --p) {
                        }
                        for (s = shpln.text + shpln.len - 1;
                                (*s == ' ' || *s == '\t') && shpln.len;
                                --s, --(shpln.len)) {
                        }
                        p = p - shpln.len + 1;
                        if (strncmp(p, shpln.text, shpln.len) == 0) {
                            ++hits;
                            a = 1;
                            break;
                        }
                    }
                    if (a) {
                        break;
                    }
                }
            }
        }
#ifdef DEBUG
        fprintf (stderr, "After side checks:\t%ld hits.\n", hits);
#endif
        if (hits > maxhits) {
            maxhits = hits;
            res = d;
        }
    }

#ifdef DEBUG
    if (res) {
        fprintf (stderr, "CHOOSING \"%s\" design (%ld hits).\n", res->name, maxhits);
    } else {
        fprintf (stderr, "NO DESIGN FOUND WITH EVEN ONE HIT!\n");
    }
#endif

    return res;
}



static void add_spaces_to_line(line_t* line, const size_t n)
{
    if (n == 0) {
        return;
    }
    line->mbtext_org = (uint32_t *) realloc(line->mbtext_org, (line->num_chars + n + 1) * sizeof(uint32_t));
    line->mbtext = line->mbtext_org;
    u32_set(line->mbtext + line->num_chars, char_space, n);
    set_char_at(line->mbtext, line->num_chars + n, char_nul);
    line->num_chars += n;
    analyze_line_ascii(&input, line);
}



int remove_box()
/*
 *  Remove box from input.
 *
 *  RETURNS:  == 0  success
 *            != 0  error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t textstart = 0;         /* index of 1st line of box body */
    size_t textend = 0;           /* index of 1st line of south side */
    size_t boxstart = 0;          /* index of 1st line of box */
    size_t boxend = 0;            /* index of 1st line trailing the box */
    int m;                        /* true if a match was found */
    size_t j;                     /* loop counter */
    int did_something = 0;        /* true if there was something to remove */

    /*
     *  If the user didn't specify a design to remove, autodetect it.
     *  Since this requires knowledge of all available designs, the entire
     *  config file had to be parsed (earlier).
     */
    if (opt.design_choice_by_user == 0) {
        design_t *tmp = detect_design();
        if (tmp) {
            opt.design = tmp;
            #ifdef DEBUG
                fprintf(stderr, "Design autodetection: Removing box of design \"%s\".\n", opt.design->name);
            #endif
        }
        else {
            fprintf(stderr, "%s: Box design autodetection failed. Use -d option.\n", PROJECT);
            return 1;
        }
    }

    /*
     *  Make all lines the same length by adding trailing spaces (needed
     *  for recognition).
     *  Also append a number of spaces to ALL input lines. A greater number
     *  takes more space and time, but enables the correct removal of boxes
     *  whose east sides consist of lots of spaces (the given value). So we
     *  add a number of spaces equal to the east side width.
     */
    const size_t normalized_len = input.maxline + opt.design->shape[NE].width;
    for (j = 0; j < input.num_lines; ++j) {
        add_spaces_to_line(input.lines + j, normalized_len - input.lines[j].len);
    }
    #ifdef DEBUG
        fprintf(stderr, "Normalized all lines to %d columns (maxline + east width).\n", (int) input.maxline);
        print_input_lines(" (remove_box)");
    #endif

    /*
     *  Phase 1: Try to find out how many lines belong to the top of the box
     */
    boxstart = 0;
    textstart = 0;
    if (empty_side(opt.design->shape, BTOP)) {
        #ifdef DEBUG
            fprintf(stderr, "----> Top box side is empty: boxstart == textstart == 0.\n");
        #endif
    }
    else {
        detect_horiz(BTOP, &boxstart, &textstart);
        #ifdef DEBUG
            fprintf(stderr, "----> First line of box is %d, ", (int) boxstart);
            fprintf(stderr, "first line of box body (text) is %d.\n", (int) textstart);
        #endif
    }


    /*
     *  Phase 2: Find out how many lines belong to the bottom of the box
     */
    if (empty_side(opt.design->shape, BBOT)) {
        textend = input.num_lines;
        boxend = input.num_lines;
        #ifdef DEBUG
            fprintf(stderr, "----> Bottom box side is empty: boxend == textend == %d.\n", (int) input.num_lines);
        #endif
    }
    else {
        textend = 0;
        boxend = 0;
        detect_horiz(BBOT, &textend, &boxend);
        if (textend == 0 && boxend == 0) {
            textend = input.num_lines;
            boxend = input.num_lines;
        }
        #ifdef DEBUG
            fprintf(stderr, "----> Last line of box body (text) is %d, ", (int) (textend - 1));
            fprintf(stderr, "last line of box is %d.\n", (int) (boxend - 1));
        #endif
    }

    /*
     *  Phase 3: Iterate over body lines, removing box sides where applicable
     */
    for (j = textstart; j < textend; ++j) {
        char *ws, *we, *es, *ee;         /* west start & end, east start&end */
        char *p;

        #ifdef DEBUG
            fprintf(stderr, "Calling best_match() for line %d:\n", (int) j);
        #endif
        m = best_match(input.lines + j, &ws, &we, &es, &ee);
        if (m < 0) {
            fprintf(stderr, "%s: internal error\n", PROJECT);
            return 1;                    /* internal error */
        }
        else if (m == 0) {
            #ifdef DEBUG
                fprintf(stderr, "\033[00;33;01mline %2d: no side match\033[00m\n", (int) j);
            #endif
        }
        else {
            #ifdef DEBUG
            fprintf(stderr, "\033[00;33;01mline %2d: west: %d (\'%c\') to %d (\'%c\') [len %d];  "
                            "east: %d (\'%c\') to %d (\'%c\') [len %d]\033[00m\n", (int) j,
                    (int) (ws       ?  ws - input.lines[j].text : 0),     ws ? ws[0]  : '?',
                    (int) (we       ?  we - input.lines[j].text - 1 : 0), we ? we[-1] : '?',
                    (int) (ws && we ? (we - input.lines[j].text - (ws - input.lines[j].text)) : 0),
                    (int) (es       ?  es - input.lines[j].text : 0),     es ? es[0]  : '?',
                    (int) (ee       ?  ee - input.lines[j].text - 1 : 0), ee ? ee[-1] : '?',
                    (int) (es && ee ? (ee - input.lines[j].text - (es - input.lines[j].text)) : 0));
            #endif
            if (ws && we) {
                did_something = 1;
                for (p = ws; p < we; ++p) {
                    size_t idx = p - input.lines[j].text;
                    *p = ' ';
                    set_char_at(input.lines[j].mbtext, input.lines[j].posmap[idx], char_space);
                }
            }
            if (es && ee) {
                for (p = es; p < ee; ++p) {
                    size_t idx = p - input.lines[j].text;
                    *p = ' ';
                    set_char_at(input.lines[j].mbtext, input.lines[j].posmap[idx], char_space);
                }
            }
        }
    }

    /*
     *  Remove as many spaces from the left side of the line as the west side
     *  of the box was wide. Don't do it if we never removed anything from the
     *  west side. Don't harm the line's text if there aren't enough spaces.
     */
    if (did_something) {
        for (j = textstart; j < textend; ++j) {
            size_t c;
            size_t widz = opt.design->shape[NW].width + opt.design->padding[BLEF];
            for (c = 0; c < widz; ++c) {
                if (input.lines[j].text[c] != ' ') {
                    break;
                }
            }
            #if defined(DEBUG)
                fprintf(stderr, "memmove(\"%s\", \"%s\", %d);\n",
                        input.lines[j].text, input.lines[j].text + c, (int) (input.lines[j].len - c + 1));
            #endif
            memmove(input.lines[j].text, input.lines[j].text + c,
                    input.lines[j].len - c + 1);         /* +1 for zero byte */
            input.lines[j].len -= c;

            #if defined(DEBUG)
                fprintf(stderr, "u32_move(\"%s\", \"%s\", %d); // posmap[c]=%d\n",
                        u32_strconv_to_output(input.lines[j].mbtext),
                        u32_strconv_to_output(input.lines[j].mbtext + input.lines[j].posmap[c]),
                        (int) (input.lines[j].num_chars - c + 1), (int) input.lines[j].posmap[c]);
            #endif
            u32_move(input.lines[j].mbtext, input.lines[j].mbtext + input.lines[j].posmap[c],
                     input.lines[j].num_chars - c + 1);  /* +1 for zero byte */
            input.lines[j].num_chars -= c;
        }
    }
    #ifdef DEBUG
        if (!did_something) {
            fprintf(stderr, "There is nothing to remove (did_something == 0).\n");
        }
    #endif

    /*
     *  Phase 4: Remove box top and body lines from input
     */
    if (opt.killblank) {
        while (empty_line(input.lines + textstart) && textstart < textend) {
            #ifdef DEBUG
                fprintf(stderr, "Killing leading blank line in box body.\n");
            #endif
            ++textstart;
        }
        while (empty_line(input.lines + textend - 1) && textend > textstart) {
            #ifdef DEBUG
                fprintf(stderr, "Killing trailing blank line in box body.\n");
            #endif
            --textend;
        }
    }

    if (textstart > boxstart) {
        for (j = boxstart; j < textstart; ++j) BFREE (input.lines[j].text);
        memmove(input.lines + boxstart, input.lines + textstart,
                (input.num_lines - textstart) * sizeof(line_t));
        input.num_lines -= textstart - boxstart;
        textend -= textstart - boxstart;
        boxend -= textstart - boxstart;
    }
    if (boxend > textend) {
        for (j = textend; j < boxend; ++j) BFREE (input.lines[j].text);
        if (boxend < input.num_lines) {
            memmove(input.lines + textend, input.lines + boxend,
                    (input.num_lines - boxend) * sizeof(line_t));
        }
        input.num_lines -= boxend - textend;
    }
    input.maxline = 0;
    for (j = 0; j < input.num_lines; ++j) {
        if (input.lines[j].len - input.lines[j].invis > input.maxline) {
            input.maxline = input.lines[j].len - input.lines[j].invis;
        }
    }
    memset(input.lines + input.num_lines, 0,
           (BMAX (textstart - boxstart, (size_t) 0) + BMAX (boxend - textend, (size_t) 0)) * sizeof(line_t));

    #ifdef DEBUG
        print_input_lines(" (remove_box) after box removal");
        fprintf(stderr, "Number of lines shrunk by %d.\n",
                (int) (BMAX (textstart - boxstart, (size_t) 0) + BMAX (boxend - textend, (size_t) 0)));
    #endif

    return 0;                            /* all clear */
}



void output_input(const int trim_only)
/*
 *  Output contents of input line list "as is" to standard output, except
 *  for removal of trailing spaces (trimming).
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t indent;
    int ntabs, nspcs;

    #ifdef DEBUG
        fprintf(stderr, "output_input() - enter (trim_only=%d)\n", trim_only);
    #endif
    for (size_t j = 0; j < input.num_lines; ++j) {
        if (input.lines[j].text == NULL) {
            continue;
        }
        btrim(input.lines[j].text, &(input.lines[j].len));
        btrim32(input.lines[j].mbtext, &(input.lines[j].num_chars));
        if (trim_only) {
            continue;
        }

        char *indentspc = NULL;
        if (opt.tabexp == 'u') {
            indent = strspn(input.lines[j].text, " ");
            ntabs = indent / opt.tabstop;
            nspcs = indent % opt.tabstop;
            indentspc = (char *) malloc(ntabs + nspcs + 1);
            if (indentspc == NULL) {
                perror(PROJECT);
                return;
            }
            memset(indentspc, (int) '\t', ntabs);
            memset(indentspc + ntabs, (int) ' ', nspcs);
            indentspc[ntabs + nspcs] = '\0';
        }
        else if (opt.tabexp == 'k') {
            indentspc = tabbify_indent(j, NULL, input.indent);
            indent = input.indent;
        }
        else {
            indentspc = (char *) strdup("");
            indent = 0;
        }

        fprintf(opt.outfile, "%s%s%s", indentspc, u32_strconv_to_output(advance32(input.lines[j].mbtext, indent)),
                (input.final_newline || j < input.num_lines - 1 ? opt.eol : ""));
        BFREE (indentspc);
    }
}



/*EOF*/                                                 /* vim: set sw=4: */
