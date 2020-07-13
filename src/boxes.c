/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (C) 1999  Thomas Jensen and the boxes contributors
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "shape.h"
#include "boxes.h"
#include "tools.h"
#include "regexp.h"
#include "generate.h"
#include "remove.h"

#ifdef __MINGW32__
    #include <windows.h>
#endif

extern char *optarg;                     /* for getopt() */
extern int optind, opterr, optopt;       /* for getopt() */

#ifdef __MINGW32__
    #define BOXES_CONFIG "boxes.cfg"     /* filename of config file in $HOME */
#else
    #define BOXES_CONFIG ".boxes"
#endif

/*
 * Some source comments (AD 1999):
 *
 * - The decision to number box shapes in clockwise order was a
 *   major design mistake. Treatment of box parts of the same
 *   alignment (N-S and E-W) is usually combined in one function,
 *   which must now deal with the numbering being reversed all the
 *   time.
 * - All shapes defined in a box design must be used in any box of
 *   that design at least once. In other words, there must not be
 *   a shape which is defined in the config file but cannot be
 *   found in an actual box of that design. This sort of limits
 *   how small your boxes can get. However, in practice it is not
 *   a problem, because boxes which must be small usually consist
 *   of small shapes which can be packed pretty tightly anyway.
 */


/*       _\|/_
         (o o)
 +----oOO-{_}-OOo------------------------------------------------------------+
 |                    G l o b a l   V a r i a b l e s                        |
 +--------------------------------------------------------------------------*/

extern int yyparse();
extern FILE *yyin;                       /* lex input file */

char *yyfilename = NULL;                 /* file name of config file used */

design_t *designs = NULL;                /* available box designs */

int design_idx = 0;                      /* anz_designs-1 */
int anz_designs = 0;                     /* no of designs after parsing */

opt_t opt;                               /* command line options */

input_t input = INPUT_INITIALIZER;       /* input lines */


/*       _\|/_
         (o o)
 +----oOO-{_}-OOo------------------------------------------------------------+
 |                           F u n c t i o n s                               |
 +--------------------------------------------------------------------------*/


static void usage (FILE *st)
/*
 *  Print usage information on stream st.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    fprintf (st, "Usage:  %s [options] [infile [outfile]]\n", PROJECT);
    fprintf (st, "        -a fmt   alignment/positioning of text inside box [default: hlvt]\n");
    fprintf (st, "        -c str   use single shape box design where str is the W shape\n");
    fprintf (st, "        -d name  box design [default: first one in file]\n");
    fprintf (st, "        -f file  configuration file\n");
    fprintf (st, "        -h       print usage information\n");
    fprintf (st, "        -i mode  indentation mode [default: box]\n");
    fprintf (st, "        -k bool  leading/trailing blank line retention on removal\n");
    fprintf (st, "        -l       list available box designs w/ samples\n");
    fprintf (st, "        -m       mend box, i.e. remove it and redraw it afterwards\n");
    fprintf (st, "        -p fmt   padding [default: none]\n");
    /* fprintf (st, "        -q       modify command for needs of the web UI (undocumented)\n"); */
    fprintf (st, "        -r       remove box\n");
    fprintf (st, "        -s wxh   box size (width w and/or height h)\n");
    fprintf (st, "        -t str   tab stop distance and expansion [default: %de]\n", DEF_TABSTOP);
    fprintf (st, "        -v       print version information\n");
}



static void usage_short (FILE *st)
/*
 *  Print abbreviated usage information on stream st.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    fprintf (st, "Usage: %s [options] [infile [outfile]]\n", PROJECT);
    fprintf (st, "Try `%s -h' for more information.\n", PROJECT);
}



static void usage_long (FILE *st)
/*
 *  Print usage information on stream st, including a header text.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    fprintf (st, "%s - draws any kind of box around your text (or removes it)\n", PROJECT);
    fprintf (st, "        Website: https://boxes.thomasjensen.com/\n");
    usage (st);
}



static int is_dir (const char *path)
/*
 *  Return true if file specified by path is a directory
 *
 *      path    file name to check
 *
 *  On Windows, this check seems unnecessary, because fopen() seems to fail
 *  when applied to a directory.
 *
 *  RETURNS:    ==  0   path is not a directory
 *               >  0   path is a directory
 *              == -1   error in stat() call
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    struct stat sinf;
    int rc;

    rc = stat (path, &sinf);
    if (rc) {
        perror (PROJECT);
        return -1;
    }

    return S_ISDIR(sinf.st_mode);
}



static int get_config_file()
/*
 *  Set yyin and yyfilename to the config file to be used.
 *
 *  If no config file was specified on the command line (yyin == stdin),
 *  try getting it from other locations:
 *      (1) contents of BOXES environment variable
 *      (2) file ~/.boxes
 *      (3) system-wide config file from GLOBALCONF macro.
 *  If neither file exists, return complainingly.
 *
 *  May change current working directory to $HOME.
 *
 *  RETURNS:    == 0    success  (yyin and yyfilename are set)
 *              != 0    error    (yyin is unmodified)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    FILE *new_yyin = NULL;
    char *s;                             /* points to data from environment */
    int   rc;
    #ifdef __MINGW32__
    char exepath[256];                   /* for constructing config file path */
    #endif

    if (yyin != stdin)
        return 0;                        /* we're already ok */

    /*
     *  Try getting it from the BOXES environment variable
     */
    s = getenv ("BOXES");
    if (s) {
        new_yyin = fopen (s, "r");
        if (new_yyin == NULL) {
            fprintf (stderr, "%s: Couldn't open config file '%s' "
                    "for input (taken from $BOXES).\n", PROJECT, s);
            return 1;
        }
        rc = is_dir (s);
        if (rc == -1) {
            fclose (new_yyin);
            return 1;
        }
        else if (rc) {
            fprintf (stderr, "%s: Alleged config file '%s' is a directory "
                    "(taken from $BOXES)\n", PROJECT, s);
            fclose (new_yyin);
            return 1;
        }
        yyfilename = (char *) strdup (s);
        if (yyfilename == NULL) {
            perror (PROJECT);
            fclose (new_yyin);
            return 1;
        }
        yyin = new_yyin;
        return 0;
    }

    /*
     *  Try getting it from ~/.boxes
     */
    s = getenv ("HOME");
    if (s) {
        rc = chdir (s);
        if (rc) {
            perror (PROJECT);
            return 1;
        }
        new_yyin = fopen (BOXES_CONFIG, "r");
        if (new_yyin) {
            rc = is_dir (BOXES_CONFIG);
            if (rc == -1) {
                fclose (new_yyin);
                return 1;
            }
            else {
                if (rc == 0) {
                    yyfilename = (char *) strdup (BOXES_CONFIG);
                    if (yyfilename == NULL) {
                        perror (PROJECT);
                        fclose (new_yyin);
                        return 1;
                    }
                    yyin = new_yyin;
                    return 0;
                }
                else {
                    fclose (new_yyin);
                    new_yyin = NULL;
                }
            }
        }
    }
    else {
        #ifndef __MINGW32__
        /* Do not print this warning on windows. */
        fprintf (stderr, "%s: warning: Environment variable HOME not set!\n",
                PROJECT);
        #endif
    }

    /*
     *  Try reading the system-wide config file
     *  On UNIX, the global config file must be given by the GLOBALCONF macro.
     *  On Win32, it is expected to reside in the same directory as the binary
     *  used, and its name is assumed to be execfilename+".cfg".
     */
    #ifdef __MINGW32__
        if (GetModuleFileName (NULL, exepath, 255) != 0) {
            char *p = strrchr (exepath, '.') + 1;
            if (p) {
                /* p is always != NULL, because we get the full path */
                *p = '\0';
                if (strlen(exepath) < 253) {
                    strcat (exepath, "cfg");       /* c:\blah\boxes.cfg */
                }
                else {
                    fprintf (stderr, "%s: path too long. Using C:\\boxes.cfg.\n", PROJECT);
                    strcpy (exepath, "C:\\boxes.cfg");
                }
            }
        }
        else {
            strcpy (exepath, "C:\\boxes.cfg");
        }
        new_yyin = fopen (exepath, "r");
    #else
        new_yyin = fopen (GLOBALCONF, "r");
    #endif
    if (new_yyin) {
        #ifdef __MINGW32__
            yyfilename = (char *) strdup (exepath);
        #else
            rc = is_dir (GLOBALCONF);
            if (rc == -1) {
                fclose (new_yyin);
                return 1;
            }
            else if (rc) {
                fprintf (stderr, "%s: Alleged system-wide config file '%s' "
                        "is a directory\n", PROJECT, GLOBALCONF);
                fclose (new_yyin);
                return 1;
            }
            yyfilename = (char *) strdup (GLOBALCONF);
        #endif
        if (yyfilename == NULL) {
            perror (PROJECT);
            fclose (new_yyin);
            return 1;
        }
        yyin = new_yyin;
        return 0;
    }

    /*
     *  Darn. No luck today.
     */
    fprintf (stderr, "%s: Can't find config file.\n", PROJECT);
    return 2;
}



static int process_commandline (int argc, char *argv[])
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
    int    oc;                           /* option character */
    FILE  *f;                            /* potential input file */
    int    idummy;
    char  *pdummy;
    char   c;
    int    errfl = 0;                    /* true on error */
    size_t optlen;
    int    rc;

    /*
     *  Set default values
     */
    memset (&opt, 0, sizeof(opt_t));
    opt.tabstop = DEF_TABSTOP;
    opt.tabexp = 'e';
    opt.killblank = -1;
    for (idummy=0; idummy<ANZ_SIDES; ++idummy)
        opt.padding[idummy] = -1;
    yyin = stdin;

    /*
     *  Intercept '--help' and '-?' cases first, as they are not supported by getopt()
     */
    if (argc >= 2 && argv[1] != NULL
        && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-?") == 0))
    {
        usage_long (stdout);
        return 42;
    }

    /*
     *  Parse Command Line
     */
    do {
        oc = getopt (argc, argv, "a:c:d:f:hi:k:lmp:qrs:t:v");

        switch (oc) {

            case 'a':
                /*
                 *  Alignment/positioning of text inside box
                 */
                errfl = 0;
                pdummy = optarg;
                while (*pdummy) {
                    if (pdummy[1] == '\0' && !strchr ("lLcCrR", *pdummy)) {
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
                    if (errfl)
                        break;
                    else
                        ++pdummy;
                }
                if (errfl) {
                    fprintf (stderr, "%s: Illegal text format -- %s\n",
                            PROJECT, optarg);
                    return 1;
                }
                break;

            case 'c':
                /*
                 *  Command line design definition
                 */
                opt.cld = (char *) strdup (optarg);
                if (opt.cld == NULL) {
                    perror (PROJECT);
                    return 1;
                }
                else {
                    line_t templine = {0};
                    templine.len = strlen (opt.cld);
                    templine.text = opt.cld;
                    if (empty_line(&templine)) {
                        fprintf (stderr, "%s: boxes may not consist entirely "
                                "of whitespace\n", PROJECT);
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
                opt.design = (design_t *) ((char *) strdup (optarg));
                if (opt.design == NULL) {
                    perror (PROJECT);
                    return 1;
                }
                opt.design_choice_by_user = 1;
                break;

            case 'f':
                /*
                 *  Input File
                 */
                f = fopen (optarg, "r");
                if (f == NULL) {
                    fprintf (stderr, "%s: Couldn\'t open config file \'%s\' "
                            "for input.\n", PROJECT, optarg);
                    return 1;
                }
                rc = is_dir (optarg);
                if (rc == -1) {
                    fclose (f);
                    return 1;
                }
                else if (rc) {
                    fprintf (stderr, "%s: Alleged config file '%s' is a "
                            "directory\n", PROJECT, optarg);
                    fclose (f);
                    return 1;
                }
                yyfilename = (char *) strdup (optarg);
                if (yyfilename == NULL) {
                    perror (PROJECT);
                    fclose (f);
                    return 1;
                }
                yyin = f;
                break;

            case 'h':
                /*
                 *  Display usage information and terminate
                 */
                usage_long (stdout);
                return 42;

            case 'i':
                /*
                 *  Indentation mode
                 */
                optlen = strlen (optarg);
                if (optlen <= 3 && !strncasecmp ("box", optarg, optlen))
                    opt.indentmode = 'b';
                else if (optlen <= 4 && !strncasecmp ("text", optarg, optlen))
                    opt.indentmode = 't';
                else if (optlen <= 4 && !strncasecmp ("none", optarg, optlen))
                    opt.indentmode = 'n';
                else {
                    fprintf (stderr, "%s: invalid indentation mode\n", PROJECT);
                    return 1;
                }
                break;

            /*
             *  Kill blank lines or not [default: design-dependent]
             */
            case 'k':
                if (opt.killblank == -1) {
                    if (strisyes (optarg))
                        opt.killblank = 1;
                    else if (strisno (optarg))
                        opt.killblank = 0;
                    else {
                        fprintf (stderr, "%s: -k: invalid parameter\n", PROJECT);
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
                    idummy = (int) strtol (pdummy+1, &pdummy, 10);
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
                    if (errfl)
                        break;
                }
                if (errfl) {
                    fprintf (stderr, "%s: invalid padding specification - "
                            "%s\n", PROJECT, optarg);
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
                pdummy = strchr (optarg, 'x');
                if (!pdummy) pdummy = strchr (optarg, 'X');
                if(pdummy) {
                    *pdummy = '\0';
                }
                errno = 0;
                if (optarg != pdummy) opt.reqwidth = strtol (optarg, NULL, 10);
                if (pdummy) {
                    opt.reqheight = strtol (pdummy+1, NULL, 10);
                    *pdummy = 'x';
                }
                if(errno || (opt.reqwidth == 0 && opt.reqheight == 0)
                  || opt.reqwidth < 0 || opt.reqheight < 0) {
                    fprintf (stderr, "%s: invalid box size specification -- %s\n",
                            PROJECT, optarg);
                    return 1;
                }
                break;

            case 't':
                /*
                 *  Tab handling. Format is n[eku]
                 */
                idummy = (int) strtol (optarg, &pdummy, 10);
                if (idummy < 1 || idummy > MAX_TABSTOP) {
                    fprintf (stderr, "%s: invalid tab stop distance -- %d\n",
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
                    fprintf (stderr, "%s: invalid tab handling specification - "
                            "%s\n", PROJECT, optarg);
                    return 1;
                }
                break;

            case 'v':
                /*
                 *  Print version number
                 */
                printf ("%s version %s\n", PROJECT, VERSION);
                return 42;

            case ':': case '?':
                /*
                 *  Missing argument or illegal option - do nothing else
                 */
                usage_short (stderr);
                return 1;

            case EOF:
                /*
                 *  End of list, do nothing more
                 */
                break;

            default:                     /* This case must never be */
                fprintf (stderr, "%s: internal error\n", PROJECT);
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

    else if (argv[optind+1] && argv[optind+2]) {       /* illegal third file */
        fprintf (stderr, "%s: illegal parameter -- %s\n",
                PROJECT, argv[optind+2]);
        usage_short (stderr);
        return 1;
    }

    else {
        if (strcmp (argv[optind], "-") == 0) {
            opt.infile = stdin;          /* use stdin for input */
        }
        else {
            opt.infile = fopen (argv[optind], "r");
            if (opt.infile == NULL) {
                fprintf (stderr, "%s: Can\'t open input file -- %s\n", PROJECT,
                        argv[optind]);
                return 9;                /* can't read infile */
            }
        }

        if (argv[optind+1] == NULL) {
            opt.outfile = stdout;        /* no outfile given */
        }
        else if (strcmp (argv[optind+1], "-") == 0) {
            opt.outfile = stdout;        /* use stdout for output */
        }
        else {
            opt.outfile = fopen (argv[optind+1], "w");
            if (opt.outfile == NULL) {
                perror (PROJECT);
                if (opt.infile != stdin)
                    fclose (opt.infile);
                return 10;
            }
        }
    }

    /*
     *  If no config file has been specified yet, try getting it elsewhere.
     */
    if (opt.cld == NULL) {
        rc = get_config_file();          /* sets yyin and yyfilename     */
        if (rc)                          /* may change working directory */
            return rc;
    }

    #if defined(DEBUG) || 0
        fprintf (stderr, "Command line option settings (excerpt):\n");
        fprintf (stderr, "- Padding: l:%d t:%d r:%d b:%d\n", opt.padding[BLEF],
                opt.padding[BTOP], opt.padding[BRIG], opt.padding[BBOT]);
        fprintf (stderr, "- Requested box size: %ldx%ld\n", opt.reqwidth,
                opt.reqheight);
        fprintf (stderr, "- Tabstop distance: %d\n", opt.tabstop);
        fprintf (stderr, "- Tab handling: \'%c\'\n", opt.tabexp);
        fprintf (stderr, "- Alignment: horiz %c, vert %c\n",
                opt.halign?opt.halign:'?', opt.valign?opt.valign:'?');
        fprintf (stderr, "- Indentmode: \'%c\'\n",
                opt.indentmode? opt.indentmode: '?');
        fprintf (stderr, "- Line justification: \'%c\'\n",
                opt.justify? opt.justify: '?');
        fprintf (stderr, "- Kill blank lines: %d\n", opt.killblank);
        fprintf (stderr, "- Remove box: %d\n", opt.r);
        fprintf (stderr, "- Special handling for Web UI: %d\n", opt.q);
        fprintf (stderr, "- Mend box: %d\n", opt.mend);
        fprintf (stderr, "- Design Definition W shape: %s\n",
                opt.cld? opt.cld: "n/a");
    #endif

    return 0;
}



static int build_design (design_t **adesigns, const char *cld)
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

    *adesigns = (design_t *) calloc (1, sizeof(design_t));
    if (*adesigns == NULL) {
        perror (PROJECT);
        return 1;
    }
    dp = *adesigns;                      /* for readability */

    dp->name = "<Command Line Definition>";
    dp->created = "now";
    dp->revision = "1.0";
    dp->sample = "n/a";
    dp->indentmode = DEF_INDENTMODE;
    dp->padding[BLEF] = 1;

    dp->tags = (char *) calloc (10, sizeof(char));
    strcpy(dp->tags, "transient");

    dp->shape[W].height = 1;
    dp->shape[W].width = strlen(cld);
    dp->shape[W].elastic = 1;
    rc = genshape (dp->shape[W].width, dp->shape[W].height, &(dp->shape[W].chars));
    if (rc)
        return rc;
    strcpy (dp->shape[W].chars[0], cld);

    for (i=0; i<ANZ_SHAPES; ++i) {
        c = dp->shape + i;

        if (i == NNW || i == NNE || i == WNW || i == ENE || i == W
                || i == WSW || i == ESE || i == SSW || i == SSE)
            continue;

        switch (i) {
            case NW:  case SW:
                c->width = dp->shape[W].width;
                c->height = 1;
                c->elastic = 0;
                break;

            case NE:  case SE:
                c->width = 1;
                c->height = 1;
                c->elastic = 0;
                break;

            case N:  case S:  case E:
                c->width = 1;
                c->height = 1;
                c->elastic = 1;
                break;

            default:
                fprintf (stderr, "%s: internal error\n", PROJECT);
                return 1;                /* never happens ;-) */
        }

        rc = genshape (c->width, c->height, &(c->chars));
        if (rc)
            return rc;
    }

    dp->maxshapeheight = 1;
    dp->minwidth = dp->shape[W].width + 2;
    dp->minheight = 3;

    return 0;
}


static char* escape (const char* org, const int pLength)
{
    char* result = (char *) calloc (1, 2 * strlen(org) + 1);
    int orgIdx, resultIdx;
    for (orgIdx=0, resultIdx=0; orgIdx<pLength; ++orgIdx, ++resultIdx) {
        if (org[orgIdx] == '\\' || org[orgIdx] == '"') {
            result[resultIdx++] = '\\';
        }
        result[resultIdx] = org[orgIdx];
    }
    result[resultIdx] = '\0';
    return result;
}


static int style_sort (const void *p1, const void *p2)
{
    return strcasecmp ((const char *) ((*((design_t **) p1))->name),
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
        char space[LINE_MAX+1];

        memset (&space, ' ', LINE_MAX);
        space[LINE_MAX] = '\0';

        fprintf (opt.outfile, "Complete Design Information for \"%s\":\n",
                d->name);
        fprintf (opt.outfile, "-----------------------------------");
        for (i=strlen(d->name); i>0; --i)
            fprintf (opt.outfile, "-");
        fprintf (opt.outfile, "\n");

        fprintf (opt.outfile, "Author:                 %s\n",
                d->author? d->author: "(unknown author)");
        fprintf (opt.outfile, "Original Designer:      %s\n",
                 d->designer? d->designer: "(unknown artist)");
        fprintf (opt.outfile, "Creation Date:          %s\n",
                d->created? d->created: "(unknown)");

        fprintf (opt.outfile, "Current Revision:       %s%s%s\n",
                d->revision? d->revision: "",
                d->revision && d->revdate? " as of ": "",
                d->revdate? d->revdate: (d->revision? "": "(unknown)"));

        fprintf (opt.outfile, "Indentation Mode:       ");
        switch (d->indentmode) {
            case 'b':
                fprintf (opt.outfile, "box (indent box)\n");
                break;
            case 't':
                fprintf (opt.outfile, "text (retain indentation inside of box)\n");
                break;
            default:
                fprintf (opt.outfile, "none (discard indentation)\n");
                break;
        }

        fprintf (opt.outfile, "Replacement Rules:      ");
        if (d->anz_reprules > 0) {
            for (i=0; i<(int)d->anz_reprules; ++i) {
                fprintf (opt.outfile, "%d. (%s) \"%s\" WITH \"%s\"\n", i+1,
                        d->reprules[i].mode == 'g'? "glob": "once",
                        d->reprules[i].search, d->reprules[i].repstr);
                if (i < (int) d->anz_reprules - 1)
                    fprintf (opt.outfile, "                        ");
            }
        }
        else {
            fprintf (opt.outfile, "none\n");
        }
        fprintf (opt.outfile, "Reversion Rules:        ");
        if (d->anz_revrules > 0) {
            for (i=0; i<(int)d->anz_revrules; ++i) {
                fprintf (opt.outfile, "%d. (%s) \"%s\" TO \"%s\"\n", i+1,
                        d->revrules[i].mode == 'g'? "glob": "once",
                        d->revrules[i].search, d->revrules[i].repstr);
                if (i < (int) d->anz_revrules - 1)
                    fprintf (opt.outfile, "                        ");
            }
        }
        else {
            fprintf (opt.outfile, "none\n");
        }

        fprintf (opt.outfile, "Minimum Box Dimensions: %d x %d  (width x height)\n",
                (int) d->minwidth, (int) d->minheight);

        fprintf (opt.outfile, "Default Padding:        ");
        if (d->padding[BTOP] || d->padding[BRIG]
                || d->padding[BBOT] || d->padding[BLEF]) {
            if (d->padding[BLEF]) {
                fprintf (opt.outfile, "left %d", d->padding[BLEF]);
                if (d->padding[BTOP] || d->padding[BRIG] || d->padding[BBOT])
                    fprintf (opt.outfile, ", ");
            }
            if (d->padding[BTOP]) {
                fprintf (opt.outfile, "top %d", d->padding[BTOP]);
                if (d->padding[BRIG] || d->padding[BBOT])
                    fprintf (opt.outfile, ", ");
            }
            if (d->padding[BRIG]) {
                fprintf (opt.outfile, "right %d", d->padding[BRIG]);
                if (d->padding[BBOT])
                    fprintf (opt.outfile, ", ");
            }
            if (d->padding[BBOT])
                fprintf (opt.outfile, "bottom %d", d->padding[BBOT]);
            fprintf (opt.outfile, "\n");
        }
        else {
            fprintf (opt.outfile, "none\n");
        }

        fprintf (opt.outfile, "Default Killblank:      %s\n",
                empty_side (opt.design->shape, BTOP) &&
                empty_side (opt.design->shape, BBOT)? "no": "yes");

        fprintf (opt.outfile, "Tags:                   %s\n",
                d->tags ? d->tags : "(none)");

        fprintf (opt.outfile, "Elastic Shapes:         ");
        sstart = 0;
        for (i=0; i<ANZ_SHAPES; ++i) {
            if (isempty(d->shape+i))
                continue;
            if (d->shape[i].elastic) {
                fprintf (opt.outfile, "%s%s", sstart? ", ": "", shape_name[i]);
                sstart = 1;
            }
        }
        fprintf (opt.outfile, "\n");

        /*
         *  Display all shapes
         */
        if (opt.q) {
            fprintf (opt.outfile, "Sample:\n%s\n", d->sample);
        }
        else {
            int first_shape = 1;
            for (i=0; i<ANZ_SHAPES; ++i) {
                if (isdeepempty(d->shape+i)) {
                    continue;
                }
                for (w=0; w<d->shape[i].height; ++w) {
                    char* escaped_line = escape(d->shape[i].chars[w], d->shape[i].width);
                    fprintf (opt.outfile, "%-24s%3s%c \"%s\"%c\n",
                        (first_shape==1 && w==0? "Defined Shapes:": ""),
                        (w==0? shape_name[i]: ""), (w==0? ':':' '),
                        escaped_line,
                        (w<d->shape[i].height-1? ',': ' ')
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

        list = (design_t **) calloc (design_idx+1, sizeof(design_t *));
        if (list == NULL) {
            perror (PROJECT);
            return 1;
        }

        for (i=0; i<anz_designs; ++i)
            list[i] = &(designs[i]);
        qsort (list, design_idx+1, sizeof(design_t *), style_sort);

        sprintf (buf, "%d", anz_designs);

        if (!opt.q) {
            fprintf (opt.outfile, "%d Available Style%s in \"%s\":\n",
                    anz_designs, anz_designs==1?"":"s", yyfilename);
            fprintf (opt.outfile, "-----------------------%s",
                    anz_designs==1? "": "-");
            for (i=strlen(yyfilename)+strlen(buf); i>0; --i)
                fprintf (opt.outfile, "-");
            fprintf (opt.outfile, "\n\n");
        }
        for (i=0; i<anz_designs; ++i) {
            if (opt.q) {
                fprintf(opt.outfile, "%s\n", list[i]->name);
            }
            else {
                if (list[i]->author && list[i]->designer && strcmp(list[i]->author, list[i]->designer) != 0) {
                    fprintf(opt.outfile, "%s\n%s, coded by %s:\n\n%s\n\n", list[i]->name,
                            list[i]->designer, list[i]->author, list[i]->sample);
                }
                else if (list[i]->designer) {
                    fprintf(opt.outfile, "%s\n%s:\n\n%s\n\n", list[i]->name,
                            list[i]->designer, list[i]->sample);
                }
                else if (list[i]->author) {
                    fprintf(opt.outfile, "%s\nunknown artist, coded by %s:\n\n%s\n\n", list[i]->name,
                            list[i]->author, list[i]->sample);
                }
                else {
                    fprintf(opt.outfile, "%s:\n\n%s\n\n", list[i]->name, list[i]->sample);
                }
            }
        }
        BFREE (list);
    }

    return 0;
}



static int get_indent (const line_t *lines, const size_t lanz)
/*
 *  Determine indentation of given lines in spaces.
 *
 *      lines   the lines to examine
 *      lanz    number of lines to examine
 *
 *  Lines are assumed to be free of trailing whitespace.
 *
 *  RETURNS:    >= 0   indentation in spaces
 *               < 0   error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t j;
    int    res = LINE_MAX;               /* result */
    int    nonblank = 0;                 /* true if one non-blank line found */

    if (lines == NULL) {
        fprintf (stderr, "%s: internal error\n", PROJECT);
        return -1;
    }
    if (lanz == 0)
        return 0;

    for (j=0; j<lanz; ++j) {
        if (lines[j].len > 0) {
            size_t ispc;
            nonblank = 1;
            ispc = strspn (lines[j].text, " ");
            if ((int) ispc < res)
                res = ispc;
        }
    }

    if (nonblank)
        return res;                      /* success */
    else
        return 0;                        /* success, but only blank lines */
}



static int apply_substitutions (const int mode)
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
    size_t     anz_rules;
    reprule_t *rules;
    size_t     j, k;
    char       buf[LINE_MAX*2];
    size_t     buf_len;                  /* length of string in buf */

    if (opt.design == NULL)
        return 1;

    if (mode == 0) {
        anz_rules = opt.design->anz_reprules;
        rules = opt.design->reprules;
    }
    else if (mode == 1) {
        anz_rules = opt.design->anz_revrules;
        rules = opt.design->revrules;
    }
    else {
        fprintf (stderr, "%s: internal error\n", PROJECT);
        return 2;
    }

    /*
     *  Compile regular expressions
     */
    errno = 0;
    opt.design->current_rule = rules;
    for (j=0; j<anz_rules; ++j, ++(opt.design->current_rule)) {
        rules[j].prog = regcomp (rules[j].search);
    }
    opt.design->current_rule = NULL;
    if (errno) return 3;

    /*
     *  Apply regular expression substitutions to input lines
     */
    for (k=0; k<input.anz_lines; ++k) {
        opt.design->current_rule = rules;
        for (j=0; j<anz_rules; ++j, ++(opt.design->current_rule)) {
            #ifdef REGEXP_DEBUG
                fprintf (stderr, "myregsub (0x%p, \"%s\", %d, \"%s\", buf, %d, \'%c\') == ",
                        rules[j].prog, input.lines[k].text,
                        input.lines[k].len, rules[j].repstr, LINE_MAX*2,
                        rules[j].mode);
            #endif
            errno = 0;
            buf_len = myregsub (rules[j].prog, input.lines[k].text,
                    input.lines[k].len, rules[j].repstr, buf, LINE_MAX*2,
                    rules[j].mode);
            #ifdef REGEXP_DEBUG
                fprintf (stderr, "%d\n", buf_len);
            #endif
            if (errno) return 1;

            BFREE (input.lines[k].text);
            input.lines[k].text = (char *) strdup (buf);
            if (input.lines[k].text == NULL) {
                perror (PROJECT);
                return 1;
            }

            input.lines[k].vischar += buf_len - input.lines[k].len;
            input.lines[k].len = buf_len;

            if (input.lines[k].vischar > input.maxline)
                input.maxline = input.lines[k].vischar;

            #ifdef REGEXP_DEBUG
                fprintf (stderr, "input.lines[%d] == {%d, \"%s\"}\n", k,
                        input.lines[k].len, input.lines[k].text);
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
        rc = get_indent (input.lines, input.anz_lines);
        if (rc >= 0)
            input.indent = (size_t) rc;
        else
            return 4;
    }

    return 0;
}



static int has_linebreak (const char *s, const int len)
/*
 *  Determine if the given line of raw text is ended by a line break.
 *
 *  s: the string to check
 *  len: length of s
 *
 *  RETURNS:  != 0   line break found
 *            == 0   line break not found
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int result = 0;
    if (s != NULL && len > 0) {
        char the_last = s[len - 1];
        result = the_last == '\r' || the_last == '\n';
        #if defined(DEBUG)
            fprintf(stderr, "has_linebreak: (%d) %d\n", the_last, result);
        #endif
    }
    return result;
}



static int read_all_input (const int use_stdin)
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
    char    buf[LINE_MAX+2];             /* input buffer */
    char    c;
    size_t  invis;                       /* counts invisible characters */
    int     ansipos;                     /* progression of ansi sequence */
    size_t  input_size = 0;              /* number of elements allocated */
    line_t *tmp = NULL;
    char   *temp = NULL;                 /* string resulting from tab exp. */
    size_t  newlen;                      /* line length after tab expansion */
    size_t  i;
    int     rc;

    input.indent = LINE_MAX;
    input.maxline = 0;

    if (use_stdin) {
        input.anz_lines = 0;

        /*
         *  Start reading
         */
        while (fgets (buf, LINE_MAX+1, opt.infile))
        {
            if (input_size % 100 == 0) {
                input_size += 100;
                tmp = (line_t *) realloc (input.lines, input_size*sizeof(line_t));
                if (tmp == NULL) {
                    perror (PROJECT);
                    BFREE (input.lines);
                    return 1;
                }
                input.lines = tmp;
            }

            input.lines[input.anz_lines].len = strlen (buf);
            input.lines[input.anz_lines].num_leading_blanks = 0;
            input.final_newline = has_linebreak(buf, input.lines[input.anz_lines].len);

            if (opt.r) {
                input.lines[input.anz_lines].len -= 1;
                if (buf[input.lines[input.anz_lines].len] == '\n')
                    buf[input.lines[input.anz_lines].len] = '\0';
            }
            else {
                btrim (buf, &(input.lines[input.anz_lines].len));
            }

            if (input.lines[input.anz_lines].len > 0) {
                newlen = expand_tabs_into (buf,
                        input.lines[input.anz_lines].len, opt.tabstop, &temp,
                        &(input.lines[input.anz_lines].tabpos),
                        &(input.lines[input.anz_lines].tabpos_len));
                if (newlen == 0) {
                    perror (PROJECT);
                    BFREE (input.lines);
                    return 1;
                }
                input.lines[input.anz_lines].text = temp;
                input.lines[input.anz_lines].len = newlen;
                temp = NULL;
            }
            else {
                input.lines[input.anz_lines].text = (char *) strdup (buf);
            }

            /*
             * Find ANSI CSI/ESC sequences
             */
            invis = 0;
            ansipos = 0;
            for (i=0; i<input.lines[input.anz_lines].len; ++i) {
                c = input.lines[input.anz_lines].text[i];
                if (ansipos == 0 && c == 0x1b){
                    /* Found an ESC char, count it as invisible and move 1 forward in the
                     * detection of CSI sequences */
                    ansipos++;
                    invis++;
                } else if (ansipos == 1 && c == '[') {
                    /* Found '[' char after ESC. A CSI sequence has started. */
                    ansipos++;
                    invis++;
                } else if (ansipos == 1 && c >= 0x40 && c <= 0x5f) {
                    /* Found a byte designating the end of a two-byte
                     * escape sequence */
                    invis++;
                    ansipos = 0;
                } else if (ansipos == 2) {
                    /* Inside CSI sequence - Keep counting bytes as invisible */
                    invis++;

                    /* A char between 0x40 and 0x7e signals the end of an CSI or escape sequence */
                    if (c >= 0x40 && c <= 0x7e)
                        ansipos = 0;
                }
            }

            /* Save the count of invisible chars and visible chars.
             * I'm happy about suggestions for a more elegant handling
             * of this and the use of .invis and .vischar (and .len)
             * in the other functions.
             */
            input.lines[input.anz_lines].invis = invis;
            input.lines[input.anz_lines].vischar = input.lines[input.anz_lines].len - invis;

            /*
             *  Update length of longest line
             */
            if (input.lines[input.anz_lines].vischar > input.maxline) {
                input.maxline = input.lines[input.anz_lines].vischar;
            }

            /*
             *  next please
             */
            ++input.anz_lines;
        }

        if (ferror (stdin)) {
            perror (PROJECT);
            BFREE (input.lines);
            return 1;
        }
    }

    else {
        /* recalculate input statistics for redrawing the mended box */
        for (i=0; i<input.anz_lines; ++i) {
            input.lines[i].len = strlen (input.lines[i].text);
            if (input.lines[i].len > input.maxline)
                input.maxline = input.lines[i].len;
        }
    }

    /*
     *  Exit if there was no input at all
     */
    if (input.lines == NULL || input.lines[0].text == NULL)
        return 0;

    /*
     *  Compute indentation
     */
    rc = get_indent (input.lines, input.anz_lines);
    if (rc >= 0)
        input.indent = (size_t) rc;
    else
        return 1;

    /*
     *  Remove indentation, unless we want to preserve it (when removing
     *  a box or if the user wants to retain it inside the box)
     */
    if (opt.design->indentmode != 't' && opt.r == 0) {
        for (i=0; i<input.anz_lines; ++i) {
            if (input.lines[i].len >= input.indent) {
                memmove (input.lines[i].text, input.lines[i].text+input.indent,
                        input.lines[i].len-input.indent+1);
                input.lines[i].len -= input.indent;
                input.lines[i].vischar -= input.indent;
            }
        }
        input.maxline -= input.indent;
    }

    /*
     *  Apply regular expression substitutions
     */
    if (opt.r == 0) {
        if (apply_substitutions(0) != 0)
            return 1;
    }

#if 0
    /*
     *  Debugging Code: Display contents of input structure
     */
    for (i=0; i<input.anz_lines; ++i) {
        fprintf (stderr, "%3d [%02d] \"%s\"", i, input.lines[i].len,
                input.lines[i].text);
        fprintf (stderr, "\tTabs: [");
        if (input.lines[i].tabpos != NULL) {
            size_t j;
            for (j=0; j<input.lines[i].tabpos_len; ++j) {
                fprintf (stderr, "%d", input.lines[i].tabpos[j]);
                if (j < input.lines[i].tabpos_len - 1) {
                    fprintf (stderr, ", ");
                }
            }
        }
        fprintf (stderr, "] (%d)\n", input.lines[i].tabpos_len);
    }
    fprintf (stderr, "\n Longest line: %d characters.\n", input.maxline);
    fprintf (stderr, "  Indentation: %2d spaces.\n", input.indent);
    fprintf (stderr, "Final newline:  %d.\n", input.final_newline);
#endif

    return 0;
}


/*       _\|/_
         (o o)
 +----oOO-{_}-OOo------------------------------------------------------------+
 |                       P r o g r a m   S t a r t                           |
 +--------------------------------------------------------------------------*/

int main (int argc, char *argv[])
{
    int    rc;                           /* general return code */
    size_t pad;
    int    i;
    int    saved_designwidth;            /* opt.design->minwith backup, used for mending */
    int    saved_designheight;           /* opt.design->minheight backup, used for mending */

    #ifdef DEBUG
        fprintf (stderr, "BOXES STARTING ...\n");
    #endif

    /*
     *  Process command line options
     */
    #ifdef DEBUG
        fprintf (stderr, "Processing Command Line ...\n");
    #endif
    rc = process_commandline (argc, argv);
    if (rc == 42)
        exit (EXIT_SUCCESS);
    if (rc)
        exit (EXIT_FAILURE);

    /*
     *  Parse config file, then reset design pointer
     */
    #ifdef DEBUG
        fprintf (stderr, "Parsing Config File ...\n");
    #endif
    if (opt.cld == NULL) {
        rc = yyparse();
        if (rc)
            exit (EXIT_FAILURE);
    }
    else {
        rc = build_design (&designs, opt.cld);
        if (rc)
            exit (EXIT_FAILURE);
        anz_designs = 1;
    }
    BFREE (opt.design);
    opt.design = designs;

    /*
     *  If "-l" option was given, list styles and exit.
     */
    if (opt.l) {
        rc = list_styles();
        exit (rc);
    }

    /*
     *  Adjust box size and indentmode to command line specification
     *  Increase box width/height by width/height of empty sides in order
     *  to match appearance of box with the user's expectations (if -s).
     */
    if (opt.reqheight > (long) opt.design->minheight)
        opt.design->minheight = opt.reqheight;
    if (opt.reqwidth > (long) opt.design->minwidth)
        opt.design->minwidth = opt.reqwidth;
    if (opt.reqwidth) {
        if (empty_side (opt.design->shape, BRIG))
            opt.design->minwidth  += opt.design->shape[SE].width;
        if (empty_side (opt.design->shape, BLEF))
            opt.design->minwidth  += opt.design->shape[NW].width;
    }
    if (opt.reqheight) {
        if (empty_side (opt.design->shape, BTOP))
            opt.design->minheight += opt.design->shape[NW].height;
        if (empty_side (opt.design->shape, BBOT))
            opt.design->minheight += opt.design->shape[SE].height;
    }
    if (opt.indentmode)
        opt.design->indentmode = opt.indentmode;
    saved_designwidth = opt.design->minwidth;
    saved_designheight = opt.design->minheight;

    do {
        if (opt.mend == 1)  /* Mending a box works in two phases: */
            opt.r = 0;      /* opt.mend == 2: remove box          */
        --opt.mend;         /* opt.mend == 1: add it back         */
        opt.design->minwidth = saved_designwidth;
        opt.design->minheight = saved_designheight;

        /*
         *  Read input lines
         */
        #ifdef DEBUG
            fprintf (stderr, "Reading all input ...\n");
        #endif
        rc = read_all_input (opt.mend);
        if (rc)
            exit (EXIT_FAILURE);
        if (input.anz_lines == 0)
            exit (EXIT_SUCCESS);

        /*
         *  Adjust box size to fit requested padding value
         *  Command line-specified box size takes precedence over padding.
         */
        for (i=0; i<ANZ_SIDES; ++i) {
            if (opt.padding[i] > -1)
                opt.design->padding[i] = opt.padding[i];
        }
        pad  = opt.design->padding[BTOP] + opt.design->padding[BBOT];
        if (pad > 0) {
            pad += input.anz_lines;
            pad += opt.design->shape[NW].height + opt.design->shape[SW].height;
            if (pad > opt.design->minheight) {
                if (opt.reqheight) {
                    for (i=0; i<(int)(pad-opt.design->minheight); ++i) {
                        if (opt.design->padding[i%2?BBOT:BTOP])
                            opt.design->padding[i%2?BBOT:BTOP] -= 1;
                        else if (opt.design->padding[i%2?BTOP:BBOT])
                            opt.design->padding[i%2?BTOP:BBOT] -= 1;
                        else
                            break;
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
                    for (i=0; i<(int)(pad-opt.design->minwidth); ++i) {
                        if (opt.design->padding[i%2?BRIG:BLEF])
                            opt.design->padding[i%2?BRIG:BLEF] -= 1;
                        else if (opt.design->padding[i%2?BLEF:BRIG])
                            opt.design->padding[i%2?BLEF:BRIG] -= 1;
                        else
                            break;
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
                if (empty_side (opt.design->shape, BTOP)
                        && empty_side (opt.design->shape, BBOT))
                    opt.killblank = 0;
                else
                    opt.killblank = 1;
            }
            rc = remove_box();
            if (rc)
                exit (EXIT_FAILURE);
            rc = apply_substitutions (1);
            if (rc)
                exit (EXIT_FAILURE);
            output_input (opt.mend > 0);
        }

        else {
            /*
             *  Generate box
             */
            sentry_t *thebox;

            #ifdef DEBUG
                fprintf (stderr, "Generating Box ...\n");
            #endif
            thebox = (sentry_t *) calloc (ANZ_SIDES, sizeof(sentry_t));
            if (thebox == NULL) {
                perror (PROJECT);
                exit (EXIT_FAILURE);
            }
            rc = generate_box (thebox);
            if (rc)
                exit (EXIT_FAILURE);
            output_box (thebox);
        }
    } while (opt.mend > 0);

    return EXIT_SUCCESS;
}

/*EOF*/                                                  /* vim: set sw=4: */
