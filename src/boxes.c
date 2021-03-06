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

#include "config.h"
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <unictype.h>
#include <unistdio.h>
#include <unistr.h>
#include <unitypes.h>
#include <uniwidth.h>

#include "shape.h"
#include "boxes.h"
#include "tools.h"
#include "discovery.h"
#include "generate.h"
#include "regulex.h"
#include "remove.h"
#include "unicode.h"


extern char *optarg;                     /* for getopt() */
extern int optind, opterr, optopt;       /* for getopt() */



/*       _\|/_
         (o o)
 +----oOO-{_}-OOo------------------------------------------------------------+
 |                    G l o b a l   V a r i a b l e s                        |
 +--------------------------------------------------------------------------*/

extern int yyparse();
extern FILE *yyin;                   /* lex input file */

char *yyfilename = NULL;             /* file name of config file used */

design_t *designs = NULL;            /* available box designs */

int design_idx = 0;                  /* anz_designs-1 */
int anz_designs = 0;                 /* no of designs after parsing */

opt_t opt;                           /* command line options */

input_t input = INPUT_INITIALIZER;   /* input lines */



/*       _\|/_
         (o o)
 +----oOO-{_}-OOo------------------------------------------------------------+
 |                           F u n c t i o n s                               |
 +--------------------------------------------------------------------------*/


static int open_yy_config_file(const char *config_file_name)
/*
 *  Set yyin and yyfilename to the config file to be used.
 *
 *  RETURNS:    == 0    success  (yyin and yyfilename are set)
 *              != 0    error    (yyin is unmodified)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    FILE *new_yyin = fopen(config_file_name, "r");
    if (new_yyin == NULL) {
        fprintf(stderr, "%s: Couldn't open config file '%s' for input\n", PROJECT, config_file_name);
        return 1;
    }
    yyfilename = (char *) config_file_name;
    yyin = new_yyin;
    return 0;
}



static void usage(FILE *st)
/*
 *  Print usage information on stream st.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char *config_file = discover_config_file();

    fprintf(st, "Usage:  %s [options] [infile [outfile]]\n", PROJECT);
    fprintf(st, "        -a fmt   alignment/positioning of text inside box [default: hlvt]\n");
    fprintf(st, "        -c str   use single shape box design where str is the W shape\n");
    fprintf(st, "        -d name  box design [default: first one in file]\n");
    fprintf(st, "        -f file  configuration file [default: %s]\n", config_file != NULL ? config_file : "none");
    fprintf(st, "        -h       print usage information\n");
    fprintf(st, "        -i mode  indentation mode [default: box]\n");
    fprintf(st, "        -k bool  leading/trailing blank line retention on removal\n");
    fprintf(st, "        -l       list available box designs w/ samples\n");
    fprintf(st, "        -m       mend box, i.e. remove it and redraw it afterwards\n");
    fprintf(st, "        -n enc   Character encoding of input and output [default: %s]\n", locale_charset());
    fprintf(st, "        -p fmt   padding [default: none]\n");
    /* fprintf(st, "        -q       modify command for needs of the web UI (undocumented)\n"); */
    fprintf(st, "        -r       remove box\n");
    fprintf(st, "        -s wxh   box size (width w and/or height h)\n");
    fprintf(st, "        -t str   tab stop distance and expansion [default: %de]\n", DEF_TABSTOP);
    fprintf(st, "        -v       print version information\n");

    BFREE(config_file);
}



static void usage_short(FILE *st)
/*
 *  Print abbreviated usage information on stream st.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    fprintf(st, "Usage: %s [options] [infile [outfile]]\n", PROJECT);
    fprintf(st, "Try `%s -h' for more information.\n", PROJECT);
}



static void usage_long(FILE *st)
/*
 *  Print usage information on stream st, including a header text.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    fprintf(st, "%s - draws any kind of box around your text (or removes it)\n", PROJECT);
    fprintf(st, "        Website: https://boxes.thomasjensen.com/\n");
    usage(st);
}



static int process_commandline(int argc, char *argv[])
/*
 *  Process command line options.
 *
 *    argc, argv   command line as passed to main()
 *
 *  RETURNS:   == 0    success, continue
 *             == 42   success, but terminate anyway (e.g. help/version)
 *             != 0/42 error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int oc;                           /* option character */
    int idummy;
    char *pdummy;
    char c;
    int errfl = 0;                    /* true on error */
    size_t optlen;

    /*
     *  Set default values
     */
    memset(&opt, 0, sizeof(opt_t));
    opt.tabstop = DEF_TABSTOP;
    opt.f = NULL;
    opt.tabexp = 'e';
    opt.killblank = -1;
    opt.encoding = NULL;
    for (idummy = 0; idummy < ANZ_SIDES; ++idummy) {
        opt.padding[idummy] = -1;
    }

    /*
     *  Intercept '--help' and '-?' cases first, as they are not supported by getopt()
     */
    if (argc >= 2 && argv[1] != NULL
            && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-?") == 0)) {
        usage_long(stdout);
        return 42;
    }

    /*
     *  Parse Command Line
     */
    do {
        oc = getopt(argc, argv, "a:c:d:f:hi:k:lmn:p:qrs:t:v");

        switch (oc) {

            case 'a':
                /*
                 *  Alignment/positioning of text inside box
                 */
                errfl = 0;
                pdummy = optarg;
                while (*pdummy) {
                    if (pdummy[1] == '\0' && !strchr("lLcCrR", *pdummy)) {
                        errfl = 1;
                        break;
                    }
                    switch (*pdummy) {
                        case 'h': case 'H':
                            switch (pdummy[1]) {
                                case 'c': case 'C': opt.halign = 'c'; break;
                                case 'l': case 'L': opt.halign = 'l'; break;
                                case 'r': case 'R': opt.halign = 'r'; break;
                                default:            errfl = 1;        break;
                            }
                            ++pdummy;
                            break;
                        case 'v': case 'V':
                            switch (pdummy[1]) {
                                case 'c': case 'C': opt.valign = 'c'; break;
                                case 't': case 'T': opt.valign = 't'; break;
                                case 'b': case 'B': opt.valign = 'b'; break;
                                default:            errfl = 1;        break;
                            }
                            ++pdummy;
                            break;
                        case 'j': case 'J':
                            switch (pdummy[1]) {
                                case 'l': case 'L': opt.justify = 'l'; break;
                                case 'c': case 'C': opt.justify = 'c'; break;
                                case 'r': case 'R': opt.justify = 'r'; break;
                                default:            errfl = 1;         break;
                            }
                            ++pdummy;
                            break;
                        case 'l': case 'L':
                            opt.justify = 'l';
                            opt.halign = 'l';
                            opt.valign = 'c';
                            break;
                        case 'r': case 'R':
                            opt.justify = 'r';
                            opt.halign = 'r';
                            opt.valign = 'c';
                            break;
                        case 'c': case 'C':
                            opt.justify = 'c';
                            opt.halign = 'c';
                            opt.valign = 'c';
                            break;
                        default:
                            errfl = 1;
                            break;
                    }
                    if (errfl) {
                        break;
                    } else {
                        ++pdummy;
                    }
                }
                if (errfl) {
                    fprintf(stderr, "%s: Illegal text format -- %s\n", PROJECT, optarg);
                    return 1;
                }
                break;

            case 'c':
                /*
                 *  Command line design definition
                 */
                opt.cld = (char *) strdup(optarg);
                if (opt.cld == NULL) {
                    perror(PROJECT);
                    return 1;
                }
                else {
                    line_t templine = {0};
                    templine.len = strlen(opt.cld);
                    templine.text = opt.cld;
                    if (empty_line(&templine)) {
                        fprintf(stderr, "%s: boxes may not consist entirely of whitespace\n", PROJECT);
                        return 1;
                    }
                }
                opt.design_choice_by_user = 1;
                break;

            case 'd':
                /*
                 *  Box design selection
                 */
                BFREE (opt.design);
                opt.design = (design_t *) ((char *) strdup(optarg));
                if (opt.design == NULL) {
                    perror(PROJECT);
                    return 1;
                }
                opt.design_choice_by_user = 1;
                break;

            case 'f':
                /*
                 *  Input File
                 */
                opt.f = optarg;
                break;

            case 'h':
                /*
                 *  Display usage information and terminate
                 */
                usage_long(stdout);
                return 42;

            case 'i':
                /*
                 *  Indentation mode
                 */
                optlen = strlen(optarg);
                if (optlen <= 3 && !strncasecmp("box", optarg, optlen)) {
                    opt.indentmode = 'b';
                } else if (optlen <= 4 && !strncasecmp("text", optarg, optlen)) {
                    opt.indentmode = 't';
                } else if (optlen <= 4 && !strncasecmp("none", optarg, optlen)) {
                    opt.indentmode = 'n';
                } else {
                    fprintf(stderr, "%s: invalid indentation mode\n", PROJECT);
                    return 1;
                }
                break;

                /*
                 *  Kill blank lines or not [default: design-dependent]
                 */
            case 'k':
                if (opt.killblank == -1) {
                    if (strisyes(optarg)) {
                        opt.killblank = 1;
                    } else if (strisno(optarg)) {
                        opt.killblank = 0;
                    } else {
                        fprintf(stderr, "%s: -k: invalid parameter\n", PROJECT);
                        return 1;
                    }
                }
                break;

            case 'l':
                /*
                 *  List available box styles
                 */
                opt.l = 1;
                break;

            case 'm':
                /*
                 *  Mend box: remove, then redraw
                 */
                opt.mend = 2;
                opt.r = 1;
                opt.killblank = 0;
                break;

            case 'n':
                /*
                 *  Character encoding
                 */
                opt.encoding = (char *) strdup(optarg);
                if (opt.encoding == NULL) {
                    perror(PROJECT);
                    return 1;
                }
                break;

            case 'p':
                /*
                 *  Padding. format is ([ahvtrbl]n)+
                 */
                errfl = 0;
                pdummy = optarg;
                while (*pdummy) {
                    if (pdummy[1] == '\0') {
                        errfl = 1;
                        break;
                    }
                    c = *pdummy;
                    errno = 0;
                    idummy = (int) strtol(pdummy + 1, &pdummy, 10);
                    if (errno || idummy < 0) {
                        errfl = 1;
                        break;
                    }
                    switch (c) {
                        case 'a': case 'A':
                            opt.padding[BTOP] = idummy;
                            opt.padding[BBOT] = idummy;
                            opt.padding[BLEF] = idummy;
                            opt.padding[BRIG] = idummy;
                            break;
                        case 'h': case 'H':
                            opt.padding[BLEF] = idummy;
                            opt.padding[BRIG] = idummy;
                            break;
                        case 'v': case 'V':
                            opt.padding[BTOP] = idummy;
                            opt.padding[BBOT] = idummy;
                            break;
                        case 't': case 'T':
                            opt.padding[BTOP] = idummy;
                            break;
                        case 'l': case 'L':
                            opt.padding[BLEF] = idummy;
                            break;
                        case 'b': case 'B':
                            opt.padding[BBOT] = idummy;
                            break;
                        case 'r': case 'R':
                            opt.padding[BRIG] = idummy;
                            break;
                        default:
                            errfl = 1;
                            break;
                    }
                    if (errfl) {
                        break;
                    }
                }
                if (errfl) {
                    fprintf(stderr, "%s: invalid padding specification - %s\n", PROJECT, optarg);
                    return 1;
                }
                break;

            case 'q':
                /*
                 *  Activate special behavior for web UI (undocumented)
                 */
                opt.q = 1;
                break;

            case 'r':
                /*
                 *  Remove box from input
                 */
                opt.r = 1;
                break;

            case 's':
                /*
                 *  Specify desired box target size
                 */
                pdummy = strchr(optarg, 'x');
                if (!pdummy) {
                    pdummy = strchr(optarg, 'X');
                }
                if (pdummy) {
                    *pdummy = '\0';
                }
                errno = 0;
                if (optarg != pdummy) {
                    opt.reqwidth = strtol(optarg, NULL, 10);
                }
                if (pdummy) {
                    opt.reqheight = strtol(pdummy + 1, NULL, 10);
                    *pdummy = 'x';
                }
                if (errno || (opt.reqwidth == 0 && opt.reqheight == 0)
                        || opt.reqwidth < 0 || opt.reqheight < 0) {
                    fprintf(stderr, "%s: invalid box size specification -- %s\n",
                            PROJECT, optarg);
                    return 1;
                }
                break;

            case 't':
                /*
                 *  Tab handling. Format is n[eku]
                 */
                idummy = (int) strtol(optarg, &pdummy, 10);
                if (idummy < 1 || idummy > MAX_TABSTOP) {
                    fprintf(stderr, "%s: invalid tab stop distance -- %d\n",
                            PROJECT, idummy);
                    return 1;
                }
                opt.tabstop = idummy;

                errfl = 0;
                if (*pdummy != '\0') {
                    if (pdummy[1] != '\0') {
                        errfl = 1;
                    }
                    else {
                        switch (*pdummy) {
                            case 'e': case 'E':
                                opt.tabexp = 'e';
                                break;
                            case 'k': case 'K':
                                opt.tabexp = 'k';
                                break;
                            case 'u': case 'U':
                                opt.tabexp = 'u';
                                break;
                            default:
                                errfl = 1;
                                break;
                        }
                    }
                }
                if (errfl) {
                    fprintf(stderr, "%s: invalid tab handling specification - "
                            "%s\n", PROJECT, optarg);
                    return 1;
                }
                break;

            case 'v':
                /*
                 *  Print version number
                 */
                printf("%s version %s\n", PROJECT, VERSION);
                return 42;

            case ':':
            case '?':
                /*
                 *  Missing argument or illegal option - do nothing else
                 */
                usage_short(stderr);
                return 1;

            case EOF:
                /*
                 *  End of list, do nothing more
                 */
                break;

            default:                     /* This case must never be */
                fprintf(stderr, "%s: internal error\n", PROJECT);
                return 1;
        }
    } while (oc != EOF);

    /*
     *  Input and Output Files
     *
     *  After any command line options, an input file and an output file may
     *  be specified (in that order). "-" may be substituted for standard
     *  input or output. A third file name would be invalid.
     */
    if (argv[optind] == NULL) {          /* neither infile nor outfile given */
        opt.infile = stdin;
        opt.outfile = stdout;
    }

    else if (argv[optind + 1] && argv[optind + 2]) {       /* illegal third file */
        fprintf(stderr, "%s: illegal parameter -- %s\n", PROJECT, argv[optind + 2]);
        usage_short(stderr);
        return 1;
    }

    else {
        if (strcmp(argv[optind], "-") == 0) {
            opt.infile = stdin;          /* use stdin for input */
        }
        else {
            opt.infile = fopen(argv[optind], "r");
            if (opt.infile == NULL) {
                fprintf(stderr, "%s: Can\'t open input file -- %s\n", PROJECT, argv[optind]);
                return 9;                /* can't read infile */
            }
        }

        if (argv[optind + 1] == NULL) {
            opt.outfile = stdout;        /* no outfile given */
        }
        else if (strcmp(argv[optind + 1], "-") == 0) {
            opt.outfile = stdout;        /* use stdout for output */
        }
        else {
            opt.outfile = fopen(argv[optind + 1], "w");
            if (opt.outfile == NULL) {
                perror(PROJECT);
                if (opt.infile != stdin) {
                    fclose(opt.infile);
                }
                return 10;
            }
        }
    }

    #if defined(DEBUG) || 0
        fprintf (stderr, "Command line option settings (excerpt):\n");
        fprintf (stderr, "- Explicit config file: %s\n", opt.f ? opt.f : "no");
        fprintf (stderr, "- Padding: l:%d t:%d r:%d b:%d\n", opt.padding[BLEF],
                opt.padding[BTOP], opt.padding[BRIG], opt.padding[BBOT]);
        fprintf (stderr, "- Requested box size: %ldx%ld\n", opt.reqwidth, opt.reqheight);
        fprintf (stderr, "- Tabstop distance: %d\n", opt.tabstop);
        fprintf (stderr, "- Tab handling: \'%c\'\n", opt.tabexp);
        fprintf (stderr, "- Alignment: horiz %c, vert %c\n", opt.halign?opt.halign:'?', opt.valign?opt.valign:'?');
        fprintf (stderr, "- Indentmode: \'%c\'\n", opt.indentmode? opt.indentmode: '?');
        fprintf (stderr, "- Line justification: \'%c\'\n", opt.justify? opt.justify: '?');
        fprintf (stderr, "- Kill blank lines: %d\n", opt.killblank);
        fprintf (stderr, "- Remove box: %d\n", opt.r);
        fprintf (stderr, "- Special handling for Web UI: %d\n", opt.q);
        fprintf (stderr, "- Mend box: %d\n", opt.mend);
        fprintf (stderr, "- Design Definition W shape: %s\n", opt.cld? opt.cld: "n/a");
    #endif

    return 0;
}



static int build_design(design_t **adesigns, const char *cld)
/*
 *  Build a box design.
 *
 *      adesigns    Pointer to global designs list
 *      cld         the W shape as specified on the command line
 *
 *  Builds the global design list containing only one design which was
 *  built from the -c command line definition.
 *
 *  RETURNS:  != 0   on error (out of memory)
 *            == 0   on success
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    design_t *dp;                        /* pointer to design to be created */
    sentry_t *c;                         /* pointer to current shape */
    int i;
    int rc;

    *adesigns = (design_t *) calloc(1, sizeof(design_t));
    if (*adesigns == NULL) {
        perror(PROJECT);
        return 1;
    }
    dp = *adesigns;                      /* for readability */

    dp->name = "<Command Line Definition>";
    dp->created = "now";
    dp->revision = "1.0";
    dp->sample = "n/a";
    dp->indentmode = DEF_INDENTMODE;
    dp->padding[BLEF] = 1;

    dp->tags = (char *) calloc(10, sizeof(char));
    strcpy(dp->tags, "transient");

    dp->shape[W].height = 1;
    dp->shape[W].width = strlen(cld);
    dp->shape[W].elastic = 1;
    rc = genshape(dp->shape[W].width, dp->shape[W].height, &(dp->shape[W].chars));
    if (rc) {
        return rc;
    }
    strcpy(dp->shape[W].chars[0], cld);

    for (i = 0; i < ANZ_SHAPES; ++i) {
        c = dp->shape + i;

        if (i == NNW || i == NNE || i == WNW || i == ENE || i == W
                || i == WSW || i == ESE || i == SSW || i == SSE) {
                    continue;
        }

        switch (i) {
            case NW:
            case SW:
                c->width = dp->shape[W].width;
                c->height = 1;
                c->elastic = 0;
                break;

            case NE:
            case SE:
                c->width = 1;
                c->height = 1;
                c->elastic = 0;
                break;

            case N:
            case S:
            case E:
                c->width = 1;
                c->height = 1;
                c->elastic = 1;
                break;

            default:
                fprintf(stderr, "%s: internal error\n", PROJECT);
                return 1;                /* never happens ;-) */
        }

        rc = genshape(c->width, c->height, &(c->chars));
        if (rc) {
            return rc;
        }
    }

    dp->maxshapeheight = 1;
    dp->minwidth = dp->shape[W].width + 2;
    dp->minheight = 3;

    return 0;
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



static int style_sort(const void *p1, const void *p2)
{
    return strcasecmp((const char *) ((*((design_t **) p1))->name),
                      (const char *) ((*((design_t **) p2))->name));
}



static int list_styles()
/*
 *  Generate sorted listing of available box styles.
 *  Uses design name from BOX spec and sample picture plus author.
 *
 *  RETURNS:  != 0   on error (out of memory)
 *            == 0   on success
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i;

    if (opt.design_choice_by_user) {

        design_t *d = opt.design;
        int sstart = 0;
        size_t w = 0;
        char space[LINE_MAX_BYTES + 1];

        memset(&space, ' ', LINE_MAX_BYTES);
        space[LINE_MAX_BYTES] = '\0';

        fprintf(opt.outfile, "Complete Design Information for \"%s\":\n",
                d->name);
        fprintf(opt.outfile, "-----------------------------------");
        for (i = strlen(d->name); i > 0; --i) {
            fprintf(opt.outfile, "-");
        }
        fprintf(opt.outfile, "\n");

        fprintf(opt.outfile, "Author:                 %s\n",
                d->author ? d->author : "(unknown author)");
        fprintf(opt.outfile, "Original Designer:      %s\n",
                d->designer ? d->designer : "(unknown artist)");
        fprintf(opt.outfile, "Creation Date:          %s\n",
                d->created ? d->created : "(unknown)");

        fprintf(opt.outfile, "Current Revision:       %s%s%s\n",
                d->revision ? d->revision : "",
                d->revision && d->revdate ? " as of " : "",
                d->revdate ? d->revdate : (d->revision ? "" : "(unknown)"));

        fprintf(opt.outfile, "Indentation Mode:       ");
        switch (d->indentmode) {
            case 'b':
                fprintf(opt.outfile, "box (indent box)\n");
                break;
            case 't':
                fprintf(opt.outfile, "text (retain indentation inside of box)\n");
                break;
            default:
                fprintf(opt.outfile, "none (discard indentation)\n");
                break;
        }

        fprintf(opt.outfile, "Replacement Rules:      ");
        if (d->anz_reprules > 0) {
            for (i = 0; i < (int) d->anz_reprules; ++i) {
                fprintf(opt.outfile, "%d. (%s) \"%s\" WITH \"%s\"\n", i + 1,
                        d->reprules[i].mode == 'g' ? "glob" : "once",
                        d->reprules[i].search, d->reprules[i].repstr);
                if (i < (int) d->anz_reprules - 1) {
                    fprintf(opt.outfile, "                        ");
                }
            }
        }
        else {
            fprintf(opt.outfile, "none\n");
        }
        fprintf(opt.outfile, "Reversion Rules:        ");
        if (d->anz_revrules > 0) {
            for (i = 0; i < (int) d->anz_revrules; ++i) {
                fprintf(opt.outfile, "%d. (%s) \"%s\" TO \"%s\"\n", i + 1,
                        d->revrules[i].mode == 'g' ? "glob" : "once",
                        d->revrules[i].search, d->revrules[i].repstr);
                if (i < (int) d->anz_revrules - 1) {
                    fprintf(opt.outfile, "                        ");
                }
            }
        }
        else {
            fprintf(opt.outfile, "none\n");
        }

        fprintf(opt.outfile, "Minimum Box Dimensions: %d x %d  (width x height)\n",
                (int) d->minwidth, (int) d->minheight);

        fprintf(opt.outfile, "Default Padding:        ");
        if (d->padding[BTOP] || d->padding[BRIG]
                || d->padding[BBOT] || d->padding[BLEF]) {
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
            fprintf(opt.outfile, "\n");
        }
        else {
            fprintf(opt.outfile, "none\n");
        }

        fprintf(opt.outfile, "Default Killblank:      %s\n",
                empty_side(opt.design->shape, BTOP) &&
                        empty_side(opt.design->shape, BBOT) ? "no" : "yes");

        fprintf(opt.outfile, "Tags:                   %s\n",
                d->tags ? d->tags : "(none)");

        fprintf(opt.outfile, "Elastic Shapes:         ");
        sstart = 0;
        for (i = 0; i < ANZ_SHAPES; ++i) {
            if (isempty(d->shape + i)) {
                continue;
            }
            if (d->shape[i].elastic) {
                fprintf(opt.outfile, "%s%s", sstart ? ", " : "", shape_name[i]);
                sstart = 1;
            }
        }
        fprintf(opt.outfile, "\n");

        /*
         *  Display all shapes
         */
        if (opt.q) {
            fprintf(opt.outfile, "Sample:\n%s\n", d->sample);
        }
        else {
            int first_shape = 1;
            for (i = 0; i < ANZ_SHAPES; ++i) {
                if (isdeepempty(d->shape + i)) {
                    continue;
                }
                for (w = 0; w < d->shape[i].height; ++w) {
                    char *escaped_line = escape(d->shape[i].chars[w], d->shape[i].width);
                    fprintf(opt.outfile, "%-24s%3s%c \"%s\"%c\n",
                            (first_shape == 1 && w == 0 ? "Defined Shapes:" : ""),
                            (w == 0 ? shape_name[i] : ""), (w == 0 ? ':' : ' '),
                            escaped_line,
                            (w < d->shape[i].height - 1 ? ',' : ' ')
                    );
                    BFREE (escaped_line);
                }
                first_shape = 0;
            }
        }
    }


    else {
        design_t **list;                 /* temp list for sorting */
        char buf[42];

        list = (design_t **) calloc(design_idx + 1, sizeof(design_t *));
        if (list == NULL) {
            perror(PROJECT);
            return 1;
        }

        for (i = 0; i < anz_designs; ++i) {
            list[i] = &(designs[i]);
        }
        qsort(list, design_idx + 1, sizeof(design_t *), style_sort);

        sprintf(buf, "%d", anz_designs);

        if (!opt.q) {
            fprintf(opt.outfile, "%d Available Style%s in \"%s\":\n",
                    anz_designs, anz_designs == 1 ? "" : "s", yyfilename);
            fprintf(opt.outfile, "-----------------------%s",
                    anz_designs == 1 ? "" : "-");
            for (i = strlen(yyfilename) + strlen(buf); i > 0; --i) {
                fprintf(opt.outfile, "-");
            }
            fprintf(opt.outfile, "\n\n");
        }
        for (i = 0; i < anz_designs; ++i) {
            if (opt.q) {
                fprintf(opt.outfile, "%s\n", list[i]->name);
            }
            else {
                if (list[i]->author && list[i]->designer && strcmp(list[i]->author, list[i]->designer) != 0) {
                    fprintf(opt.outfile, "%s\n%s, coded by %s:\n%s\n\n", list[i]->name,
                            list[i]->designer, list[i]->author, list[i]->sample);
                }
                else if (list[i]->designer) {
                    fprintf(opt.outfile, "%s\n%s:\n%s\n\n", list[i]->name,
                            list[i]->designer, list[i]->sample);
                }
                else if (list[i]->author) {
                    fprintf(opt.outfile, "%s\nunknown artist, coded by %s:\n%s\n\n", list[i]->name,
                            list[i]->author, list[i]->sample);
                }
                else {
                    fprintf(opt.outfile, "%s:\n%s\n\n", list[i]->name, list[i]->sample);
                }
            }
        }
        BFREE (list);
    }

    return 0;
}



static int get_indent(const line_t *lines, const size_t lines_size)
/*
 *  Determine indentation of given lines in spaces.
 *
 *      lines      the lines to examine
 *      lines_size number of lines to examine
 *
 *  Lines are assumed to be free of trailing whitespace.
 *
 *  RETURNS:    >= 0   indentation in spaces
 *               < 0   error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int res = LINE_MAX_BYTES;  /* result */
    int nonblank = 0;          /* true if one non-blank line found */

    if (lines == NULL) {
        fprintf(stderr, "%s: internal error\n", PROJECT);
        return -1;
    }
    if (lines_size == 0) {
        return 0;
    }

    for (size_t j = 0; j < lines_size; ++j) {
        if (lines[j].len > 0) {
            nonblank = 1;
            size_t ispc = strspn(lines[j].text, " ");
            if ((int) ispc < res) {
                res = ispc;
            }
        }
    }

    if (nonblank) {
        return res;            /* success */
    } else {
        return 0;              /* success, but only blank lines */
    }
}



static int apply_substitutions(const int mode)
/*
 *  Apply regular expression substitutions to input text.
 *
 *    mode == 0   use replacement rules (box is being *drawn*)
 *         == 1   use reversion rules (box is being *removed*)
 *
 *  Attn: This modifies the actual input array!
 *
 *  RETURNS:  == 0   success
 *            != 0   error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t anz_rules;
    reprule_t *rules;
    size_t j, k;

    if (opt.design == NULL) {
        return 1;
    }

    if (mode == 0) {
        anz_rules = opt.design->anz_reprules;
        rules = opt.design->reprules;
    }
    else if (mode == 1) {
        anz_rules = opt.design->anz_revrules;
        rules = opt.design->revrules;
    }
    else {
        fprintf(stderr, "%s: internal error\n", PROJECT);
        return 2;
    }

    /*
     *  Compile regular expressions
     */
    #ifdef REGEXP_DEBUG
        fprintf(stderr, "Compiling %d %s rule patterns\n", (int) anz_rules, mode ? "reversion" : "replacement");
    #endif
    errno = 0;
    opt.design->current_rule = rules;
    for (j = 0; j < anz_rules; ++j, ++(opt.design->current_rule)) {
        rules[j].prog = compile_pattern(rules[j].search);
        if (rules[j].prog == NULL) {
            return 5;
        }
    }
    opt.design->current_rule = NULL;
    if (errno) {
        return 3;
    }

    /*
     *  Apply regular expression substitutions to input lines
     */
    for (k = 0; k < input.anz_lines; ++k) {
        opt.design->current_rule = rules;
        for (j = 0; j < anz_rules; ++j, ++(opt.design->current_rule)) {
            #ifdef REGEXP_DEBUG
            fprintf (stderr, "regex_replace(0x%p, \"%s\", \"%s\", %d, \'%c\') == ",
                    rules[j].prog, rules[j].repstr, u32_strconv_to_output(input.lines[k].mbtext),
                    (int) input.lines[k].num_chars, rules[j].mode);
            #endif
            uint32_t *newtext = regex_replace(rules[j].prog, rules[j].repstr,
                                              input.lines[k].mbtext, input.lines[k].num_chars, rules[j].mode == 'g');
            #ifdef REGEXP_DEBUG
                fprintf (stderr, "\"%s\"\n", newtext ? u32_strconv_to_output(newtext) : "NULL");
            #endif
            if (newtext == NULL) {
                return 1;
            }

            BFREE(input.lines[k].mbtext_org);  /* original address allocated for mbtext */
            input.lines[k].mbtext = newtext;
            input.lines[k].mbtext_org = newtext;

            analyze_line_ascii(input.lines + k);

            #ifdef REGEXP_DEBUG
                fprintf (stderr, "input.lines[%d] == {%d, \"%s\"}\n", (int) k,
                    (int) input.lines[k].num_chars, u32_strconv_to_output(input.lines[k].mbtext));
            #endif
        }
        opt.design->current_rule = NULL;
    }

    /*
     *  If text indentation was part of the lines processed, indentation
     *  may now be different -> recalculate input.indent.
     */
    if (opt.design->indentmode == 't') {
        int rc;
        rc = get_indent(input.lines, input.anz_lines);
        if (rc >= 0) {
            input.indent = (size_t) rc;
        } else {
            return 4;
        }
    }

    return 0;
}



static int has_linebreak(const uint32_t *s, const int len)
/*
 *  Determine if the given line of raw text is ended by a line break.
 *
 *  s: the string to check
 *  len: length of s in characters
 *
 *  RETURNS:  != 0   line break found
 *            == 0   line break not found
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int result = 0;
    if (s != NULL && len > 0) {
        ucs4_t the_last = s[len - 1];
        result = u32_cmp(&char_cr, &the_last, 1) == 0 || u32_cmp(&char_newline, &the_last, 1) == 0;
        #if defined(DEBUG)
            fprintf(stderr, "has_linebreak: (%#010x) %d\n", (int) the_last, result);
        #endif
    }
    return result;
}



static int read_all_input(const int use_stdin)
/*
 *  Read entire input (possibly from stdin) and store it in 'input' array.
 *
 *  Tabs are expanded.
 *  Might allocate slightly more memory than it needs. Trade-off for speed.
 *
 *  use_stdin: flag indicating whether to read from stdin (use_stdin != 0)
 *             or use the data currently present in input (use_stdin == 0).
 *
 *  RETURNS:  != 0   on error (out of memory)
 *            == 0   on success
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char buf[LINE_MAX_BYTES + 3];    /* input buffer incl. newline + zero terminator */
    size_t len_chars;
    size_t input_size = 0;           /* number of elements allocated */
    uint32_t *mbtemp = NULL;         /* temp string for preparing the multi-byte input */
    size_t i;
    int rc;

    input.indent = LINE_MAX_BYTES;
    input.maxline = 0;

    if (use_stdin) {
        input.anz_lines = 0;

        /*
         *  Start reading
         */
        while (fgets(buf, LINE_MAX_BYTES + 2, opt.infile)) {
            if (input.anz_lines % 100 == 0) {
                input_size += 100;
                line_t *tmp = (line_t *) realloc(input.lines, input_size * sizeof(line_t));
                if (tmp == NULL) {
                    perror(PROJECT);
                    BFREE (input.lines);
                    return 1;
                }
                input.lines = tmp;
            }

            mbtemp = u32_strconv_from_input(buf);
            len_chars = u32_strlen(mbtemp);
            input.final_newline = has_linebreak(mbtemp, len_chars);
            input.lines[input.anz_lines].posmap = NULL;

            if (opt.r) {
                if (is_char_at(mbtemp, len_chars - 1, char_newline)) {
                    set_char_at(mbtemp, len_chars - 1, char_nul);
                    --len_chars;
                }
                if (is_char_at(mbtemp, len_chars - 1, char_cr)) {
                    set_char_at(mbtemp, len_chars - 1, char_nul);
                    --len_chars;
                }
            }
            else {
                btrim32(mbtemp, &len_chars);
            }

            /*
             * Expand tabs
             */
            if (len_chars > 0) {
                uint32_t *temp = NULL;
                len_chars = expand_tabs_into(mbtemp, opt.tabstop, &temp,
                                             &(input.lines[input.anz_lines].tabpos),
                                             &(input.lines[input.anz_lines].tabpos_len));
                if (len_chars == 0) {
                    perror(PROJECT);
                    BFREE (input.lines);
                    return 1;
                }
                input.lines[input.anz_lines].mbtext = temp;
                BFREE(mbtemp);
                temp = NULL;
            }
            else {
                input.lines[input.anz_lines].mbtext = mbtemp;
            }
            input.lines[input.anz_lines].mbtext_org = input.lines[input.anz_lines].mbtext;
            input.lines[input.anz_lines].num_chars = len_chars;

            /*
             * Build ASCII equivalent of the multi-byte string, update line stats
             */
            input.lines[input.anz_lines].text = NULL;  /* we haven't used it yet! */
            analyze_line_ascii(input.lines + input.anz_lines);
            input.lines[input.anz_lines].num_leading_blanks = 0;

            ++input.anz_lines;
        }

        if (ferror(stdin)) {
            perror(PROJECT);
            BFREE (input.lines);
            return 1;
        }
    }

    else {
        /* recalculate input statistics for redrawing the mended box */
        for (i = 0; i < input.anz_lines; ++i) {
            analyze_line_ascii(input.lines + i);
        }
    }

    /*
     *  Exit if there was no input at all
     */
    if (input.lines == NULL || input.lines[0].text == NULL) {
        return 0;
    }

    /*
     *  Compute indentation
     */
    rc = get_indent(input.lines, input.anz_lines);
    if (rc >= 0) {
        input.indent = (size_t) rc;
    } else {
        return 1;
    }

    /*
     *  Remove indentation, unless we want to preserve it (when removing
     *  a box or if the user wants to retain it inside the box)
     */
    if (opt.design->indentmode != 't' && opt.r == 0) {
        for (i = 0; i < input.anz_lines; ++i) {
            #ifdef DEBUG
                fprintf(stderr, "%2d: mbtext = \"%s\" (%d chars)\n", (int) i,
                    u32_strconv_to_output(input.lines[i].mbtext), (int) input.lines[i].num_chars);
            #endif
            if (input.lines[i].num_chars >= input.indent) {
                memmove(input.lines[i].text, input.lines[i].text + input.indent,
                        input.lines[i].len - input.indent + 1);
                input.lines[i].len -= input.indent;

                input.lines[i].mbtext = advance32(input.lines[i].mbtext, input.indent);
                input.lines[i].num_chars -= input.indent;
            }
            #ifdef DEBUG
                fprintf(stderr, "%2d: mbtext = \"%s\" (%d chars)\n", (int) i,
                    u32_strconv_to_output(input.lines[i].mbtext), (int) input.lines[i].num_chars);
            #endif
        }
        input.maxline -= input.indent;
    }

    /*
     *  Apply regular expression substitutions
     */
    if (opt.r == 0) {
        if (apply_substitutions(0) != 0) {
            return 1;
        }
    }

#ifdef DEBUG
    fprintf (stderr, "Encoding: %s\n", encoding);
    print_input_lines(NULL);
#endif

    return 0;
}



/*       _\|/_
         (o o)
 +----oOO-{_}-OOo------------------------------------------------------------+
 |                       P r o g r a m   S t a r t                           |
 +--------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int rc;                           /* general return code */
    size_t pad;
    int i;
    int saved_designwidth;            /* opt.design->minwith backup, used for mending */
    int saved_designheight;           /* opt.design->minheight backup, used for mending */

    #ifdef DEBUG
        fprintf (stderr, "BOXES STARTING ...\n");
    #endif

    /*
     *  Process command line options
     */
    #ifdef DEBUG
        fprintf (stderr, "Processing Command Line ...\n");
    #endif
    rc = process_commandline(argc, argv);
    if (rc == 42) {
        exit(EXIT_SUCCESS);
    }
    if (rc) {
        exit(EXIT_FAILURE);
    }

    /*
     * Store system character encoding
     */
    setlocale(LC_ALL, "");    /* switch from default "C" encoding to system encoding */
    encoding = check_encoding(opt.encoding, locale_charset());
    #ifdef DEBUG
        fprintf (stderr, "Character Encoding = %s\n", encoding);
    #endif

    /*
     *  Parse config file, then reset design pointer
     */
    char *config_file = discover_config_file();
    if (config_file == NULL) {
        exit(EXIT_FAILURE);
    }
    if (opt.cld == NULL) {
        yyin = stdin;
        rc = open_yy_config_file(config_file);
        if (rc) {
            exit(EXIT_FAILURE);
        }
    }
    #ifdef DEBUG
        fprintf (stderr, "Parsing Config File %s ...\n", config_file);
    #endif
    if (opt.cld == NULL) {
        rc = yyparse();
        if (rc) {
            exit(EXIT_FAILURE);
        }
    }
    else {
        rc = build_design(&designs, opt.cld);
        if (rc) {
            exit(EXIT_FAILURE);
        }
        anz_designs = 1;
    }
    BFREE (opt.design);
    opt.design = designs;

    /*
     *  If "-l" option was given, list styles and exit.
     */
    if (opt.l) {
        rc = list_styles();
        exit(rc);
    }

    /*
     *  Adjust box size and indentmode to command line specification
     *  Increase box width/height by width/height of empty sides in order
     *  to match appearance of box with the user's expectations (if -s).
     */
    if (opt.reqheight > (long) opt.design->minheight) {
        opt.design->minheight = opt.reqheight;
    }
    if (opt.reqwidth > (long) opt.design->minwidth) {
        opt.design->minwidth = opt.reqwidth;
    }
    if (opt.reqwidth) {
        if (empty_side(opt.design->shape, BRIG)) {
            opt.design->minwidth += opt.design->shape[SE].width;
        }
        if (empty_side(opt.design->shape, BLEF)) {
            opt.design->minwidth += opt.design->shape[NW].width;
        }
    }
    if (opt.reqheight) {
        if (empty_side(opt.design->shape, BTOP)) {
            opt.design->minheight += opt.design->shape[NW].height;
        }
        if (empty_side(opt.design->shape, BBOT)) {
            opt.design->minheight += opt.design->shape[SE].height;
        }
    }
    if (opt.indentmode) {
        opt.design->indentmode = opt.indentmode;
    }
    saved_designwidth = opt.design->minwidth;
    saved_designheight = opt.design->minheight;

    do {
        if (opt.mend == 1) {  /* Mending a box works in two phases: */
            opt.r = 0;        /* opt.mend == 2: remove box          */
        }
        --opt.mend;           /* opt.mend == 1: add it back         */
        opt.design->minwidth = saved_designwidth;
        opt.design->minheight = saved_designheight;

        /*
         *  Read input lines
         */
        #ifdef DEBUG
            fprintf (stderr, "Reading all input ...\n");
        #endif
        rc = read_all_input(opt.mend);
        if (rc) {
            exit(EXIT_FAILURE);
        }
        if (input.anz_lines == 0) {
            exit(EXIT_SUCCESS);
        }

        /*
         *  Adjust box size to fit requested padding value
         *  Command line-specified box size takes precedence over padding.
         */
        for (i = 0; i < ANZ_SIDES; ++i) {
            if (opt.padding[i] > -1) {
                opt.design->padding[i] = opt.padding[i];
            }
        }
        pad = opt.design->padding[BTOP] + opt.design->padding[BBOT];
        if (pad > 0) {
            pad += input.anz_lines;
            pad += opt.design->shape[NW].height + opt.design->shape[SW].height;
            if (pad > opt.design->minheight) {
                if (opt.reqheight) {
                    for (i = 0; i < (int) (pad - opt.design->minheight); ++i) {
                        if (opt.design->padding[i % 2 ? BBOT : BTOP]) {
                            opt.design->padding[i % 2 ? BBOT : BTOP] -= 1;
                        } else if (opt.design->padding[i % 2 ? BTOP : BBOT]) {
                            opt.design->padding[i % 2 ? BTOP : BBOT] -= 1;
                        } else {
                            break;
                        }
                    }
                }
                else {
                    opt.design->minheight = pad;
                }
            }
        }
        pad = opt.design->padding[BLEF] + opt.design->padding[BRIG];
        if (pad > 0) {
            pad += input.maxline;
            pad += opt.design->shape[NW].width + opt.design->shape[NE].width;
            if (pad > opt.design->minwidth) {
                if (opt.reqwidth) {
                    for (i = 0; i < (int) (pad - opt.design->minwidth); ++i) {
                        if (opt.design->padding[i % 2 ? BRIG : BLEF]) {
                            opt.design->padding[i % 2 ? BRIG : BLEF] -= 1;
                        } else if (opt.design->padding[i % 2 ? BLEF : BRIG]) {
                            opt.design->padding[i % 2 ? BLEF : BRIG] -= 1;
                        } else {
                            break;
                        }
                    }
                }
                else {
                    opt.design->minwidth = pad;
                }
            }
        }

        if (opt.r) {
            /*
             *  Remove box
             */
            #ifdef DEBUG
                fprintf (stderr, "Removing Box ...\n");
            #endif
            if (opt.killblank == -1) {
                if (empty_side(opt.design->shape, BTOP) && empty_side(opt.design->shape, BBOT)) {
                    opt.killblank = 0;
                } else {
                    opt.killblank = 1;
                }
            }
            rc = remove_box();
            if (rc) {
                exit(EXIT_FAILURE);
            }
            rc = apply_substitutions(1);
            if (rc) {
                exit(EXIT_FAILURE);
            }
            output_input(opt.mend > 0);
        }

        else {
            /*
             *  Generate box
             */
            sentry_t *thebox;

            #ifdef DEBUG
                fprintf (stderr, "Generating Box ...\n");
            #endif
            thebox = (sentry_t *) calloc(ANZ_SIDES, sizeof(sentry_t));
            if (thebox == NULL) {
                perror(PROJECT);
                exit(EXIT_FAILURE);
            }
            rc = generate_box(thebox);
            if (rc) {
                exit(EXIT_FAILURE);
            }
            output_box(thebox);
        }
    } while (opt.mend > 0);

    return EXIT_SUCCESS;
}

/*EOF*/                                                  /* vim: set sw=4: */
