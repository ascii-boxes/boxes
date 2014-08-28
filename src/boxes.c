/*
 *  File:             boxes.c
 *  Date created:     March 18, 1999 (Thursday, 15:09h)
 *  Author:           Copyright (C) 1999 Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: boxes.c,v 1.32 1999/08/22 11:34:46 tsjensen Exp tsjensen $
 *  Language:         ANSI C
 *  Platforms:        sunos5/sparc, for now
 *  World Wide Web:   http://home.pages.de/~jensen/boxes/
 *  Purpose:          Filter to draw boxes around input text (and remove it).
 *                    Intended for use with vim(1).
 *
 *  Remarks: o This program is free software; you can redistribute it
 *             and/or modify it under the terms of the GNU General Public
 *             License as published by the Free Software Foundation; either
 *             version 2 of the License, or (at your option) any later
 *             version.
 *           o This program is distributed in the hope that it will be
 *             useful, but WITHOUT ANY WARRANTY; without even the implied
 *             warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *             PURPOSE. See the GNU General Public License for more details.
 *           o You should have received a copy of the GNU General Public
 *             License along with this program; if not, write to the Free
 *             Software Foundation, Inc., 59 Temple Place, Suite 330,
 *             Boston, MA  02111-1307  USA
 *
 *           - This version is leaking a small bit of memory. The sizes of
 *             the leaks do not depend on the number of lines processed, so
 *             the leaks don't matter as long as this program is executed as
 *             a single process.
 *           - The decision to number box shapes in clockwise order was a
 *             major design mistake. Treatment of box parts of the same
 *             alignment (N-S and E-W) is usually combined in one function,
 *             which must now deal with the numbering being reversed all the
 *             time. This is nasty, but changing the shape order would
 *             pretty much mean a total rewrite of the code, so we'll have
 *             to live with it. :-(
 *           - All shapes defined in a box design must be used in any box of
 *             that design at least once. In other words, there must not be
 *             a shape which is defined in the config file but cannot be
 *             found in an actual box of that design. This sort of limits
 *             how small your boxes can get. However, in practice it is not
 *             a problem, because boxes which must be small usually consist
 *             of small shapes which can be packed pretty tightly anyway.
 *             And again, changing this would pretty much mean a total
 *             rewrite.
 *
 *  Revision History:
 *
 *    $Log: boxes.c,v $
 *    Revision 1.32  1999/08/22 11:34:46  tsjensen
 *    Bugfix: no-input-check must take place before indentation computation
 *
 *    Revision 1.31  1999/08/21 23:33:03  tsjensen
 *    Added usage of system-wide config file (GLOBALCONF from boxes.h)
 *    Moved config file selection code into it own function (get_config_file())
 *
 *    Revision 1.30  1999/08/21 15:55:42  tsjensen
 *    Updated usage information
 *    Updated quickinfo (-l -d) with killblank default value
 *    Made indentation computation into a function of its own (get_indent())
 *    Bugfix: REVERSION code may change indentation -> recompute if necessary
 *
 *    Revision 1.29  1999/08/16 18:29:39  tsjensen
 *    Added output of total number of designs for -l
 *
 *    Revision 1.28  1999/07/23 16:15:48  tsjensen
 *    Added quickinfo mode to list_styles(). Called with -l and -d together.
 *
 *    Revision 1.27  1999/07/21 16:53:17  tsjensen
 *    Bugfix: Empty box sides were still counted when -s was specified (so
 *    -s x5 would get you only three lines).
 *
 *    Revision 1.26  1999/07/20 18:45:29  tsjensen
 *    Added GNU GPL disclaimer
 *    Added -k option (kill leading/trailing blank lines on removal yes/no)
 *    Bugfix: REPLACE and indentation conflict. Now applying regexp substitutions
 *    only after indentation was already handled.
 *
 *    Revision 1.25  1999/07/12 18:16:36  tsjensen
 *    Added include "config.h" to top of file
 *
 *    Revision 1.24  1999/06/30 12:37:25  tsjensen
 *    Added [lcr] "shorthand" to -a option
 *
 *    Revision 1.23  1999/06/30 12:16:38  tsjensen
 *    Removed entire select_design stuff, because the parser now returns much
 *    smaller data structures. The first design pointed to by designs is always
 *    the one we need (except for ops on *all* designs).
 *    The first design in config file is now default design (no DEF_DESIGN anymore).
 *
 *    Revision 1.22  1999/06/28 12:19:29  tsjensen
 *    Moved parser init and cleanup from main() to parser.y
 *    Some further cleanup in main()
 *
 *    Revision 1.21  1999/06/25 18:46:39  tsjensen
 *    Bugfix: Mixed up SW and NE in padding calculation (tricky, that one)
 *    Added indent mode command line option for grammar overrides
 *    Moved empty_side() to shape.c after small change in signature
 *    Added text-in-block justification (j) to alignment option (-a)
 *
 *    Revision 1.20  1999/06/23 19:17:27  tsjensen
 *    Removed snprintf() and vsnprintf() prototypes (why were they in anyway?)
 *    along with stdarg.h include
 *    Exported input global data and empty_side() function
 *    Declared non-exported functions static
 *    Moved horiz_precalc(), vert_precalc(), horiz_assemble(), vert_assemble(),
 *    horiz_generate(), vert_generate(), generate_box() and output_box() to a
 *    new file generate.c
 *    Moved best_match(), hmm(), detect_horiz(), detect_design(), remove_box()
 *    and output_input() to a new file remove.c
 *
 *    Revision 1.19  1999/06/23 12:31:26  tsjensen
 *    Improvements on design detection (could still be better though)
 *    Moved iscorner(), on_side(), isempty(), shapecmp(), both_on_side(),
 *    shape_distance(), empty_side(), highest(), and widest() as well as
 *    some data structures and macros related to shapes to new file shape.c
 *
 *    Revision 1.18  1999/06/22 12:03:33  tsjensen
 *    DEBUGging is now activated in boxes.h
 *    Moved MAX_TABSTOP, LINE_MAX macros and struct opt_t to boxes.h
 *    Moved BMAX, strrstr(), btrim(), yyerror(), expand_tabs_into(),
 *    regerror(), empty_line() to a new file tools.c, added #include tools.h
 *    Some cleanup in main()
 *    Declared style_sort() helper function static
 *    Renamed strrstr() to my_strnrstr() (now defined in tools.c)
 *    Default design is now defined by DEF_DESIGN macro
 *    Changed select_design(), because default design value now set in process_commandline()
 *    Bugfix: segfaulted if shape was bigger than input when detecting design
 *    Bugfix: Padding was not removed when box was removed
 *    Bugfix: Forgot null byte when removing west box side
 *
 *    Revision 1.17  1999/06/20 14:20:29  tsjensen
 *    Added code for padding handling (-p)
 *    Added BMAX macro (returns maximum of two values)
 *
 *    Revision 1.16  1999/06/17 19:07:06  tsjensen
 *    Moved line_t to boxes.h
 *    empty_line() now also considers \r and \n whitespace
 *    Added empty_side() function
 *    Added handling of empty box sides in output_box()
 *
 *    Revision 1.15  1999/06/15 12:07:39  tsjensen
 *    Removed a stray debug message
 *    Move apply_substitutions() regexp handling function up in file
 *    Use apply_substitutions() in read_input() routine also
 *    Moved "extern int yyparse()" prototype to start of file
 *
 *    Revision 1.14  1999/06/14 12:08:49  tsjensen
 *    Bugfix: best_match() box side detection used numw instead of nume
 *    Added apply_substitutions() routine for central regexp handling
 *    Added regexp reversion code for box removal
 *    Unified use of current_re[pv]rule
 *    Added a few comments and debugging code
 *
 *    Revision 1.13  1999/06/13 15:28:31  tsjensen
 *    Some error message clean-up
 *    Regular expression substitutions on input text only if *drawing* a box,
 *    not if the box is to be removed (requires other substitutions, todo).
 *    Added code for box design auto-detection when removing a box.
 *
 *    Revision 1.12  1999/06/04 18:13:26  tsjensen
 *    Don't adjust indentation after removing a box unless something was
 *    removed on the west side
 *    East Padding made dynamic, i.e. dependant on the east side size
 *
 *    Revision 1.11  1999/06/03 19:24:14  tsjensen
 *    A few fixes related to box removal (as expected)
 *
 *    Revision 1.10  1999/06/03 18:54:05  tsjensen
 *    Lots of fixes
 *    Added remove box functionality (-r), which remains to be tested
 *
 *    Revision 1.9  1999/04/09 13:33:24  tsjensen
 *    Removed code related to OFFSET blocks (obsolete)
 *
 *    Revision 1.8  1999/04/04 16:09:01  tsjensen
 *    Added code for specification of indentation handling of input
 *    Added regular expression substitutions
 *    Some minor fixes
 *
 *    Revision 1.7  1999/04/02 18:42:44  tsjensen
 *    Added infile/outfile parameter code (pasted from tal, more or less)
 *    Added code to remove trailing spaces from output lines
 *
 *    Revision 1.6  1999/04/01 17:26:18  tsjensen
 *    Some bug fixes
 *    Added size option (-s)
 *    Added Alignment Option (-a)
 *    It seems actually usable for drawing boxes :-)
 *
 *    Revision 1.4  1999/03/30 13:30:19  tsjensen
 *    Added minimum width/height for a design. Fixed screwed tiny boxes.
 *    Bugfix: Did not handle zero input.
 *
 *    Revision 1.1  1999/03/18 15:09:17  tsjensen
 *    Initial revision
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#include "config.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "shape.h"
#include "boxes.h"
#include "tools.h"
#include "regexp.h"
#include "generate.h"
#include "remove.h"

extern char *optarg;                     /* for getopt() */
extern int optind, opterr, optopt;       /* for getopt() */


static const char rcsid_boxes_c[] =
    "$Id: boxes.c,v 1.32 1999/08/22 11:34:46 tsjensen Exp tsjensen $";


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
    fprintf (st, "        -d name  select box design [default: first one in file]\n");
    fprintf (st, "        -f file  use only file as configuration file\n");
    fprintf (st, "        -h       print usage information\n");
    fprintf (st, "        -i mode  indentation mode [default: box]\n");
    fprintf (st, "        -k bool  kill leading/trailing blank lines on removal or not\n");
    fprintf (st, "        -l       list available box designs w/ samples\n");
    fprintf (st, "        -p fmt   padding [default: none]\n");
    fprintf (st, "        -r       remove box from input\n");
    fprintf (st, "        -s wxh   specify box size (width w and/or height h)\n");
    fprintf (st, "        -t uint  set tab stop distance [default: %d]\n", DEF_TABSTOP);
    fprintf (st, "        -v       print version information\n");
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
 *  RETURNS:    == 0    success  (yyin and yyfilename are set)
 *              != 0    error    (yyin is unmodified)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    FILE *new_yyin = NULL;
    char *s;                             /* points to data from environment */

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
        yyfilename = (char *) strdup (s);
        if (yyfilename == NULL) {
            perror (PROJECT);
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
        char buf[PATH_MAX];              /* to build file name */
        strncpy (buf, s, PATH_MAX);
        buf[PATH_MAX-1-7] = '\0';        /* ensure space for "/.boxes" */
        strcat (buf, "/.boxes");
        new_yyin = fopen (buf, "r");
        if (new_yyin) {
            yyfilename = (char *) strdup (buf);
            if (yyfilename == NULL) {
                perror (PROJECT);
                return 1;
            }
            yyin = new_yyin;
            return 0;
        }
    }
    else {
        fprintf (stderr, "%s: warning: Environment variable HOME not set!\n",
                PROJECT);
    }

    /*
     *  Try reading the system-wide config file
     */
    new_yyin = fopen (GLOBALCONF, "r");
    if (new_yyin) {
        yyfilename = (char *) strdup (GLOBALCONF);
        if (yyfilename == NULL) {
            perror (PROJECT);
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
    int    outfile_existed = 0;          /* true if we overwrite a file */
    size_t optlen;
    int    rc;

    /*
     *  Set default values
     */
    memset (&opt, 0, sizeof(opt_t));
    opt.tabstop = DEF_TABSTOP;
    opt.killblank = -1;
    for (idummy=0; idummy<ANZ_SIDES; ++idummy)
        opt.padding[idummy] = -1;
    yyin = stdin;

    /*
     *  Parse Command Line
     */
    do {
        oc = getopt (argc, argv, "a:d:f:hi:k:lp:rs:t:v");

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
                yyfilename = (char *) strdup (optarg);
                if (yyfilename == NULL) {
                    perror (PROJECT);
                    return 1;
                }
                yyin = f;
                break;

            case 'h':
                /*
                 *  Display usage information and terminate
                 */
                printf ("%s - draws any kind of box around your text (and removes it)\n", PROJECT);
                printf ("        (c) Thomas Jensen <tsjensen@stud.informatik.uni-erlangen.de>\n");
                printf ("        Web page: http://home.pages.de/~jensen/%s/\n", PROJECT);
                usage (stdout);
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
                if (strisyes (optarg))
                    opt.killblank = 1;
                else if (strisno (optarg))
                    opt.killblank = 0;
                else {
                    fprintf (stderr, "%s: -k: invalid parameter\n", PROJECT);
                    return 1;
                }
                break;

            case 'l':
                /*
                 *  List available box styles
                 */
                opt.l = 1;
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
                errno = 0;
                opt.reqwidth = strtol (optarg, NULL, 10);
                idummy = errno;
                if (idummy) {
                    fprintf (stderr, "%s: invalid box size specification: %s\n",
                            PROJECT, strerror(idummy));
                    return 1;
                }
                if (pdummy) {
                    errno = 0;
                    opt.reqheight = strtol (pdummy+1, NULL, 10);
                    idummy = errno;
                    if (idummy) {
                        fprintf (stderr, "%s: invalid box size specification: %s\n",
                                PROJECT, strerror(idummy));
                        return 1;
                    }
                }
                if ((opt.reqwidth == 0 && opt.reqheight == 0)
                  || opt.reqwidth < 0 || opt.reqheight < 0) {
                    fprintf (stderr, "%s: invalid box size specification -- %s\n",
                            PROJECT, optarg);
                    return 1;
                }
                break;

            case 't':
                /*
                 *  Tab stop distance
                 */
                idummy = (int) strtol (optarg, NULL, 10);
                if (idummy < 1 || idummy > MAX_TABSTOP) {
                    fprintf (stderr, "%s: invalid tab stop distance -- %d\n",
                            PROJECT, idummy);
                    return 1;
                }
                opt.tabstop = idummy;
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
     *  If no config file has as yet been specified, try getting it elsewhere.
     */
    rc = get_config_file();              /* sets yyin and yyfilename */
    if (rc)
        return rc;

    /*
     *  Input and Output Files
     *
     *  After any command line options, an input file and an output file may
     *  be specified (in that order). "-" may be substituted for standard
     *  input or output. A third file name would be invalid.
     *  The alogrithm is as follows:
     *
     *  If no files are given, use stdin and stdout.
     *  Else If infile is "-", use stdin for input
     *       Else open specified file (die if it doesn't work)
     *       If no output file is given, use stdout for output
     *       Else If outfile is "-", use stdout for output
     *            Else open specified file for writing (die if it doesn't work)
     *            If a third file is given, die.
     */
    if (argv[optind] == NULL) {          /* neither infile nor outfile given */
        opt.infile = stdin;
        opt.outfile = stdout;
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
        else {
            if (strcmp (argv[optind+1], "-") == 0) {
                opt.outfile = stdout;    /* use stdout for output */
            }
            else {
                outfile_existed = !access (argv[optind+1], F_OK);
                opt.outfile = fopen (argv[optind+1], "w");
                if (opt.outfile == NULL) {
                    perror (PROJECT);
                    if (opt.infile != stdin)
                        fclose (opt.infile);
                    return 10;
                }
            }
            if (argv[optind+2]) {        /* illegal third file */
                fprintf (stderr, "%s: illegal parameter -- %s\n",
                        PROJECT, argv[optind+2]);
                usage (stderr);
                if (opt.infile != stdin)
                    fclose (opt.infile);
                if (opt.outfile != stdout) {
                    fclose (opt.outfile);
                    if (!outfile_existed) unlink (argv[optind+1]);
                }
                return 1;
            }
        }
    }

    #if defined(DEBUG) || 0
        fprintf (stderr, "Command line option settings (excerpt):\n");
        fprintf (stderr, "- Padding: l%d o%d r%d u%d\n", opt.padding[BLEF],
                opt.padding[BTOP], opt.padding[BRIG], opt.padding[BBOT]);
        fprintf (stderr, "- Requested box size: %ldx%ld\n", opt.reqwidth,
                opt.reqheight);
        fprintf (stderr, "- Tabstop distance: %d\n", opt.tabstop);
        fprintf (stderr, "- Alignment: horiz %c, vert %c\n",
                opt.halign?opt.halign:'?', opt.valign?opt.valign:'?');
        fprintf (stderr, "- Indentmode: \'%c\'\n",
                opt.indentmode? opt.indentmode: '?');
        fprintf (stderr, "- Line justification: \'%c\'\n",
                opt.justify? opt.justify: '?');
        fprintf (stderr, "- Kill blank lines: %d\n", opt.killblank);
    #endif

    return 0;
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
        int until = -1;
        int sstart = 0;
        size_t w = 0;
        size_t j;
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
                d->author? d->author: "(unknown artist)");
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
                d->minwidth, d->minheight);

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
        fprintf (opt.outfile, "Defined Shapes:       ");
        until = -1;
        sstart = 0;
        do {
            sstart = until + 1;
            for (w=0, i=sstart; i<ANZ_SHAPES; ++i) {
                if (isempty(d->shape+i))
                    continue;
                w += 6;
                w += d->shape[i].width;
                w += strlen(shape_name[i]);
                if (i == 0)
                    w -= 2;
                else if (w > 56) {       /* assuming an 80 character screen */
                    until = i - 1;
                    break;
                }
            }
            if (i == ANZ_SHAPES)
                until = ANZ_SHAPES - 1;
            for (w=0, i=sstart; i<=until; ++i) {
                if (d->shape[i].height > w)
                    w = d->shape[i].height;
            }
            for (j=0; j<w; ++j) {
                if (j > 0 || sstart > 0)
                    fprintf (opt.outfile, "                      ");
                for (i=sstart; i<=until; ++i) {
                    if (isempty(d->shape+i))
                        continue;
                    fprintf (opt.outfile, "  ");
                    if (j == 0)
                        fprintf (opt.outfile, "%s: ", shape_name[i]);
                    else {
                        space[strlen(shape_name[i])+2] = '\0';
                        fprintf (opt.outfile, space);
                        space[strlen(shape_name[i])+2] = ' ';
                    }
                    if (j < d->shape[i].height) {
                        fprintf (opt.outfile, "\"%s\"", d->shape[i].chars[j]);
                    }
                    else {
                        space[d->shape[i].width+2] = '\0';
                        fprintf (opt.outfile, space);
                        space[d->shape[i].width+2] = ' ';
                    }
                }
                fprintf (opt.outfile, "\n");
            }
            if (until < ANZ_SHAPES-1 && d->maxshapeheight > 2)
                fprintf (opt.outfile, "\n");
        } while (until < ANZ_SHAPES-1);
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

        fprintf (opt.outfile, "%d Available Style%s in \"%s\":\n",
                anz_designs, anz_designs==1?"":"s", yyfilename);
        fprintf (opt.outfile, "-----------------------%s",
                anz_designs==1? "": "-");
        for (i=strlen(yyfilename)+strlen(buf); i>0; --i)
            fprintf (opt.outfile, "-");
        fprintf (opt.outfile, "\n\n");
        for (i=0; i<anz_designs; ++i)
            fprintf (opt.outfile, "%s (%s):\n\n%s\n\n", list[i]->name,
                    list[i]->author? list[i]->author: "unknown artist",
                    list[i]->sample);

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
            input.lines[k].len = buf_len;
            if (input.lines[k].len > input.maxline)
                input.maxline = input.lines[k].len;
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



static int read_all_input()
/*
 *  Read entire input from stdin and store it in 'input' array.
 *
 *  Tabs are expanded.
 *  Might allocate slightly more memory than it needs. Trade-off for speed.
 *
 *  RETURNS:  != 0   on error (out of memory)
 *            == 0   on success
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char    buf[LINE_MAX+2];             /* input buffer */
    size_t  input_size = 0;              /* number of elements allocated */
    line_t *tmp = NULL;
    char   *temp = NULL;                 /* string resulting from tab exp. */
    size_t  newlen;                      /* line length after tab expansion */
    size_t  i;
    int     rc;

    input.anz_lines = 0;
    input.indent = LINE_MAX;
    input.maxline = 0;

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
                    input.lines[input.anz_lines].len, opt.tabstop, &temp);
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
         *  Update length of longest line
         */
        if (input.lines[input.anz_lines].len > input.maxline)
            input.maxline = input.lines[input.anz_lines].len;

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
        fprintf (stderr, "%3d [%02d] \"%s\"\n", i, input.lines[i].len,
                input.lines[i].text);
    }
    fprintf (stderr, "\nLongest line: %d characters.\n", input.maxline);
    fprintf (stderr, " Indentation: %2d spaces.\n", input.indent);
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

    #ifdef DEBUG
        fprintf (stderr, "BOXES STARTING ...\n");
    #endif

    /*
     *  Process command line options
     */
    #ifdef DEBUG
        fprintf (stderr, "Processing Comand Line ...\n");
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
    rc = yyparse();
    if (rc)
        exit (EXIT_FAILURE);
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

    /*
     *  Read input lines
     */
    #ifdef DEBUG
        fprintf (stderr, "Reading all input ...\n");
    #endif
    rc = read_all_input();
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
        output_input();
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

    return EXIT_SUCCESS;
}

/*EOF*/                                                  /* vim: set sw=4: */
