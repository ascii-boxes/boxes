/*
 *  File:             boxes.c
 *  Date created:     March 18, 1999 (Thursday, 15:09h)
 *  Author:           Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: boxes.c,v 1.9 1999/04/09 13:33:24 tsjensen Exp tsjensen $
 *  Language:         ANSI C
 *  Platforms:        sunos5/sparc, for now
 *  World Wide Web:   http://home.pages.de/~jensen/boxes/
 *  Purpose:          Filter to draw boxes around input text.
 *                    Intended for use with vim(1).
 *  Remarks:          - This version is leaking memory. The sizes of the
 *                      leaks do not depend on the number of lines
 *                      processed, so the leaks don't matter as long as
 *                      this program is executed as a single process.
 *                    - This is not a release.
 *
 *  Revision History:
 *
 *    $Log: boxes.c,v $
 *    Revision 1.9  1999/04/09 13:33:24  tsjensen
 *    Removed code related to OFFSET blocks (obsolete)
 *
 *    Revision 1.8  1999/04/04 16:09:01  tsjensen
 *    Added code for specification of indentation handling of input
 *    Added regular expression substitutions
 *    Some minor fixes
 *
 *    Revision 1.7  1999/04/02 18:42:44  tsjensen
 *    ... still programming ...
 *    Added infile/outfile parameter code (pasted from tal, more or less)
 *    Added code to remove trailing spaces from output lines
 *
 *    Revision 1.6  1999/04/01 17:26:18  tsjensen
 *    ... still programming ...
 *    Some bug fixes
 *    Added size option (-s)
 *    Added Alignment Option (-a)
 *    It seems actually usable for drawing boxes :-)
 *
 *    Revision 1.5  1999/03/31 17:34:21  tsjensen
 *    ... still programming ...
 *    (some bug fixes and restructuring)
 *
 *    Revision 1.4  1999/03/30 13:30:19  tsjensen
 *    Added minimum width/height for a design. Fixed screwed tiny boxes.
 *    Did not handle zero input.
 *
 *    Revision 1.3  1999/03/30 09:36:23  tsjensen
 *    ... still programming ...
 *    (removed setlocale() call and locale.h include)
 *
 *    Revision 1.2  1999/03/19 17:44:47  tsjensen
 *    ... still programming ...
 *
 *    Revision 1.1  1999/03/18 15:09:17  tsjensen
 *    Initial revision
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/* #define DEBUG */
/* #define REGEXP_DEBUG */

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "regexp.h"
#include "boxes.h"

extern int snprintf (char *, size_t, const char *, ...);        /* stdio.h */
extern int vsnprintf (char *, size_t, const char *, __va_list); /* stdio.h */
extern char *optarg;                     /* for getopt() */
extern int optind, opterr, optopt;       /* for getopt() */


static const char rcsid_boxes_c[] =
    "$Id: boxes.c,v 1.9 1999/04/09 13:33:24 tsjensen Exp tsjensen $";

extern FILE *yyin;                       /* lex input file */



/*  Number of spaces appended to all input lines prior to removing a box.
 *  A greater number takes more space and time, but enables the correct
 *  removal of boxes whose east sides consist of lots of spaces (the given
 *  value).
 */
#define EAST_PADDING    20

/*  max. allowed tab stop distance
 */
#define MAX_TABSTOP     16

/*  max. supported line length
 *  This is how many characters of a line will be read. Anything beyond
 *  will be discarded. Output may be longer though. The line feed character
 *  at the end does not count.
 *  (This should have been done via sysconf(), but I didn't do it in order
 *  to ease porting to non-unix platforms.)
 */
#if defined(LINE_MAX) && (LINE_MAX < 1024)
#undef LINE_MAX
#endif
#ifndef LINE_MAX
#define LINE_MAX        2048
#endif


char *yyfilename = NULL;                 /* file name of config file used */

design_t *designs = NULL;                /* available box designs */

int design_idx = 0;                      /* anz_designs-1 */
int anz_designs = 0;                     /* no of designs after parsing */


char *shape_name[] = {
    "NW", "NNW", "N", "NNE", "NE", "ENE", "E", "ESE",
    "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW"
};

shape_t north_side[SHAPES_PER_SIDE] = { NW, NNW, N, NNE, NE };  /* clockwise */
shape_t  east_side[SHAPES_PER_SIDE] = { NE, ENE, E, ESE, SE };
shape_t south_side[SHAPES_PER_SIDE] = { SE, SSE, S, SSW, SW };
shape_t  west_side[SHAPES_PER_SIDE] = { SW, WSW, W, WNW, NW };
shape_t corners[ANZ_CORNERS] = { NW, NE, SE, SW };
shape_t *sides[] = { north_side, east_side, south_side, west_side };


struct {                                 /* Command line options: */
    int       l;                         /* list available designs */
    int       r;                         /* remove box from input */
    int       tabstop;                   /* tab stop distance */
    design_t *design;                    /* currently used box design */
    int       design_choice_by_user;     /* true if design was chosen by user */
    long      reqwidth;                  /* requested box width (-s) */
    long      reqheight;                 /* requested box height (-s) */
    char      valign;                    /* text position inside box */
    char      halign;                    /* ( h[lcr]v[tcb] )         */
    FILE     *infile;                    /* where we get our input */
    FILE     *outfile;                   /* where we put our output */
} opt;


typedef struct {
    size_t len;
    char  *text;
} line_t;

struct {
    line_t *lines;
    size_t anz_lines;                    /* number of entries in input */
    size_t maxline;                      /* length of longest input line */
    size_t indent;                       /* number of leading spaces found */
} input = {NULL, 0, 0, LINE_MAX};




int yyerror (const char *fmt, ...)
/*
 *  Print configuration file parser errors.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    va_list ap;
    char buf[1024];

    va_start (ap, fmt);
    snprintf (buf, 1024-1, "%s: %s: line %d: ", PROJECT,
            yyfilename? yyfilename: "(null)", yylineno);
    vsnprintf (buf+strlen(buf), 1024-strlen(buf)-1, fmt, ap);
    strcat (buf, "\n");
    fprintf (stderr, buf);
    va_end (ap);

    return 0;
}



void regerror (char *msg)
/*
 *  Print regular expression andling error messages
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    fprintf (stderr, "%s: %s: line %d: %s\n",
            PROJECT, yyfilename? yyfilename: "(null)",
            opt.design->current_reprule? opt.design->current_reprule->line: 0,
            msg);
    errno = EINVAL;
}



int iscorner (const shape_t s)
/*
 *  Return true if shape s is a corner.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i;
    shape_t *p;

    for (i=0, p=corners; i<ANZ_CORNERS; ++i, ++p) {
        if (*p == s)
            return 1;
    }

    return 0;
}



shape_t *on_side (const shape_t s, const int idx)
/*
 *  Compute the side that shape s is on.
 *
 *      s    shape to look for
 *      idx  which occurence to return (0 == first, 1 == second (for corners)
 *
 *  RETURNS: pointer to a side list  on success
 *           NULL                    on error (e.g. idx==1 && s no corner)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int       side;
    int       i;
    shape_t **sp;
    shape_t  *p;
    int       found = 0;

    for (side=0,sp=sides; side<ANZ_SIDES; ++side,++sp) {
        for (i=0,p=*sp; i<SHAPES_PER_SIDE; ++i,++p) {
            if (*p == s) {
                if (found == idx)
                    return *sp;
                else
                    ++found;
            }
        }
    }

    return NULL;
}



int isempty (const sentry_t *shape)
/*
 *  Return true if shape is empty.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    if (shape == NULL)
        return 1;
    else if (shape->chars == NULL)
        return 1;
    else if (shape->width == 0 || shape->height == 0)
        return 1;
    else
        return 0;
}



int empty_line (const line_t *line)
/*
 *  Return true if line is empty.
 *
 *  Empty lines either consist entirely of whitespace or don't exist.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char *p;
    size_t j;

    if (!line)
        return 1;
    if (line->text == NULL || line->len <= 0)
        return 1;

    for (p=line->text, j=0; *p && j<line->len; ++j, ++p) {
        if (*p != ' ' && *p != '\t')
            return 0;
    }
    return 1;
}



int shapecmp (const sentry_t *shape1, const sentry_t *shape2)
/*
 *  Compare two shapes.
 *
 *  RETURNS: == 0   if shapes are equal
 *           != 0   if shapes differ
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int    e1 = isempty (shape1);
    int    e2 = isempty (shape2);
    size_t i;

    if ( e1 &&  e2) return 0;
    if (!e1 &&  e2) return 1;
    if ( e1 && !e2) return -1;

    if (shape1->width != shape2->width || shape1->height != shape2->height) {
        if (shape1->width * shape1->height > shape2->width * shape2->height)
            return 1;
        else
            return -1;
    }

    for (i=0; i<shape1->height; ++i) {
        int c = strcmp (shape1->chars[i], shape2->chars[i]);  /* no casecmp! */
        if (c) return c;
    }

    return 0;
}



shape_t *both_on_side (const shape_t shape1, const shape_t shape2)
/*
 *  Compute the side that *both* shapes are on.
 *
 *  RETURNS: pointer to a side list  on success
 *           NULL                    on error (e.g. shape on different sides)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i, j, found;

    for (i=0; i<ANZ_SIDES; ++i) {
        found = 0;
        for (j=0; j<SHAPES_PER_SIDE; ++j) {
            if (sides[i][j] == shape1 || sides[i][j] == shape2)
                ++found;
            if (found > 1) {
                switch (i) {
                    case 0: return north_side;
                    case 1: return east_side;
                    case 2: return south_side;
                    case 3: return west_side;
                    default: return NULL;
                }
            }
        }
    }

    return NULL;
}



int shape_distance (const shape_t s1, const shape_t s2)
/*
 *  Compute distance between two shapes which are located on the same side
 *  of the box. E.g. shape_distance(NW,N) == 2.
 *
 *  RETURNS: distance in steps   if ok
 *           -1                  on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i;
    int distance = -1;
    shape_t *workside = both_on_side (s1, s2);

    if (!workside) return -1;
    if (s1 == s2) return 0;

    for (i=0; i<SHAPES_PER_SIDE; ++i) {
        if (workside[i] == s1 || workside[i] == s2) {
            if (distance == -1)
                distance = 0;
            else if (distance > -1) {
                ++distance;
                break;
            }
        }
        else {
            if (distance > -1)
                ++distance;
        }
    }

    if (distance > 0 && distance < SHAPES_PER_SIDE)
        return distance;
    else
        return -1;
}



static void usage (FILE *st)
/*
 *  Print usage information on stream st.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    fprintf (st, "Usage: %s [options] [infile [outfile]]\n", PROJECT);
    fprintf (st, "       -a fmt   alignment/positioning of text inside box [default: hlvt]\n");
    fprintf (st, "       -d name  select box design\n");
    fprintf (st, "       -f file  use only file as configuration file\n");
    fprintf (st, "       -h       print usage information\n");
    fprintf (st, "       -l       list available box designs w/ samples\n");
    fprintf (st, "       -r       remove box from input\n");
    fprintf (st, "       -s wxh   specify box size (width w and/or height h)\n");
    fprintf (st, "       -t uint  set tab stop distance [default: %d]\n", DEF_TABSTOP);
    fprintf (st, "       -v       print version information\n");
}



static int process_commandline (int argc, char *argv[])
/*
 *
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int   oc;                            /* option character */
    FILE *f;                             /* potential input file */
    int   idummy;
    char *pdummy;
    int   errfl = 0;                     /* true on error */
    int   outfile_existed = 0;           /* true if we overwrite a file */

    memset (&opt, 0, sizeof(opt));
    opt.tabstop = DEF_TABSTOP;
    yyin = stdin;

    do {
        oc = getopt (argc, argv, "a:d:f:hlrs:t:v");

        switch (oc) {

            case 'a':
                /*
                 *  Alignment/positioning of text inside box
                 */
                errfl = 0;
                pdummy = optarg;
                while (*pdummy) {
                    if (pdummy[1] == '\0') {
                        errfl = 1;
                        break;
                    }
                    switch (*pdummy) {
                        case 'h':
                        case 'H':
                            switch (pdummy[1]) {
                                case 'c': case 'C': opt.halign = 'c'; break;
                                case 'l': case 'L': opt.halign = 'l'; break;
                                case 'r': case 'R': opt.halign = 'r'; break;
                                default:            errfl = 1;        break;
                            }
                            break;
                        case 'v':
                        case 'V':
                            switch (pdummy[1]) {
                                case 'c': case 'C': opt.valign = 'c'; break;
                                case 't': case 'T': opt.valign = 't'; break;
                                case 'b': case 'B': opt.valign = 'b'; break;
                                default:            errfl = 1;        break;
                            }
                            break;
                        default:
                            errfl = 1;
                            break;
                    }
                    if (errfl)
                        break;
                    else
                        pdummy += 2;
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
                opt.design = (design_t *) ((char *) strdup (optarg));
                if (opt.design == NULL) {
                    perror (PROJECT);
                    return 1;
                }
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
                printf ("%s - draws boxes around your text\n", PROJECT);
                printf ("        (c) Thomas Jensen <tsjensen@stud.informatik.uni-erlangen.de>\n");
                printf ("        Web page: http://home.pages.de/~jensen/%s/\n", PROJECT);
                usage (stdout);
                return 42;

            case 'l':
                /*
                 *  List available box styles
                 */
                opt.l = 1;
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

            default:                        /* This case must never be */
                fprintf (stderr, "%s: Uh-oh! This should have been "
                        "unreachable code. %%-)\n", PROJECT);
                return 1;
        }
    } while (oc != EOF);

    /*
     *  If no config file has been specified, try getting it from other
     *  locations. (1) contents of BOXES environment variable (2) file
     *  ~/.boxes. If neither file exists, complain and die.
     */
    if (yyin == stdin) {
        char *s = getenv ("BOXES");
        if (s) {
            f = fopen (s, "r");
            if (f == NULL) {
                fprintf (stderr, "%s: Couldn\'t open config file \'%s\' "
                        "for input (taken from environment).\n", PROJECT, s);
                return 1;
            }
            yyfilename = (char *) strdup (s);
            yyin = f;
        }
        else {
            char buf[PATH_MAX];              /* to build file name */
            s = getenv ("HOME");
            if (s) {
                strncpy (buf, s, PATH_MAX);
                buf[PATH_MAX-1-7] = '\0';    /* ensure space for "/.boxes" */
                strcat (buf, "/.boxes");
                f = fopen (buf, "r");
                if (f) {
                    yyfilename = (char *) strdup (buf);
                    yyin = f;
                }
                else {
                    fprintf (stderr, "%s: Could not find config file.\n",
                            PROJECT);
                    return 1;
                }
            }
            else {
                fprintf (stderr, "%s: Environment variable HOME must point to "
                        "the user\'s home directory.\n", PROJECT);
                return 1;
            }
        }
    }

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

    return 0;
}



int style_sort (const void *p1, const void *p2)
{
    return strcasecmp ((const char *) ((*((design_t **) p1))->name),
                       (const char *) ((*((design_t **) p2))->name));
}

int list_styles()
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
    design_t **list;                     /* temp list for sorting */

    list = (design_t **) calloc (design_idx+1, sizeof(design_t *));
    if (list == NULL) return 1;

    for (i=0; i<=design_idx; ++i)
        list[i] = &(designs[i]);
    qsort (list, design_idx+1, sizeof(design_t *), style_sort);

    fprintf (opt.outfile, "Available Styles:\n");
    fprintf (opt.outfile, "-----------------\n\n");
    for (i=0; i<=design_idx; ++i)
        fprintf (opt.outfile, "%s (%s):\n\n%s\n\n", list[i]->name,
                list[i]->author? list[i]->author: "unknown artist",
                list[i]->sample);

    BFREE (list);

    return 0;
}



static size_t expand_tabs_into (const char *input_buffer, const int in_len,
      const int tabstop, char **text)
/*
 *  Expand tab chars in input_buffer and store result in text.
 *
 *  input_buffer   Line of text with tab chars
 *  in_len         length of the string in input_buffer
 *  tabstop        tab stop distance
 *  text           address of the pointer that will take the result
 *
 *  Memory will be allocated for the result.
 *  Should only be called for lines of length > 0;
 *
 *  RETURNS:  Success: Length of the result line in characters (> 0)
 *            Error:   0       (e.g. out of memory)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
   static char temp [LINE_MAX*MAX_TABSTOP+1];  /* work string */
   int ii;                               /* position in input string */
   int io;                               /* position in work string */
   int jp;                               /* tab expansion jump point */

   *text = NULL;

   for (ii=0, io=0; ii<in_len && io<(LINE_MAX*tabstop-1); ++ii) {
      if (input_buffer[ii] == '\t') {
         for (jp=io+tabstop-(io%tabstop); io<jp; ++io)
            temp[io] = ' ';
      }
      else {
         temp[io] = input_buffer[ii];
         ++io;
      }
   }
   temp[io] = '\0';

   *text = (char *) strdup (temp);
   if (*text == NULL) return 0;

   return io;
}



void btrim (char *text, size_t *len)
/*
 *  Remove trailing whitespace from line.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    long idx = (long) (*len) - 1;

    while (idx >= 0 && (text[idx] == '\n' || text[idx] == '\r'
                     || text[idx] == '\t' || text[idx] == ' '))
    {
        text[idx--] = '\0';
    }

    *len = idx + 1;
}



int read_all_input()
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

    input.anz_lines = 0;
    input.indent = LINE_MAX;
    input.maxline = 0;

    /*
     *  Compile regular expressions
     */
    errno = 0;
    for (i=0; i<opt.design->anz_reprules; ++i) {
        opt.design->current_reprule = opt.design->reprules + i;
        opt.design->reprules[i].prog =
            regcomp (opt.design->reprules[i].search);
    }
    opt.design->current_reprule = NULL;
    if (errno) return 1;

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
         *  Apply regular expression substitutions to line
         */
        for (i=0; i<opt.design->anz_reprules; ++i) {
            opt.design->current_reprule = opt.design->reprules + i;
            errno = 0;
            #ifdef REGEXP_DEBUG
                fprintf (stderr, "myregsub (0x%p, \"%s\", %d, \"%s\", buf, %d, \'%c\') == ",
                        opt.design->reprules[i].prog,
                        input.lines[input.anz_lines].text,
                        input.lines[input.anz_lines].len,
                        opt.design->reprules[i].repstr, LINE_MAX+2,
                        opt.design->reprules[i].mode);
            #endif
            input.lines[input.anz_lines].len =
                myregsub (opt.design->reprules[i].prog,
                        input.lines[input.anz_lines].text,
                        input.lines[input.anz_lines].len,
                        opt.design->reprules[i].repstr,
                        buf, LINE_MAX+2, opt.design->reprules[i].mode);
            #ifdef REGEXP_DEBUG
                fprintf (stderr, "%d\n", input.lines[input.anz_lines].len);
            #endif
            if (errno) return 1;
            BFREE (input.lines[input.anz_lines].text);
            input.lines[input.anz_lines].text = (char *) strdup (buf);
            if (input.lines[input.anz_lines].text == NULL) {
                perror (PROJECT);
                return 1;
            }
            #ifdef REGEXP_DEBUG
                fprintf (stderr, "input.lines[input.anz_lines] == {%d, \"%s\"}\n",
                        input.lines[input.anz_lines].len,
                        input.lines[input.anz_lines].text);
            #endif
        }
        opt.design->current_reprule = NULL;

        /*
         *  Update length of longest line
         */
        if (input.lines[input.anz_lines].len > input.maxline)
            input.maxline = input.lines[input.anz_lines].len;

        /*
         *  Update current estimate for text indentation
         */
        if (input.lines[input.anz_lines].len > 0) {
            size_t ispc;
            ispc = strspn (input.lines[input.anz_lines].text, " ");
            if (ispc < input.indent)
                input.indent = ispc;
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

    /*
     *  Exit if there was no input at all
     */
    if (input.lines == NULL || input.lines[0].text == NULL) {
        return 0;
    }

    /*
     *  Remove indentation, unless we want to preserve it (when removing
     *  a box or if the user wants to retain it inside the box)
     */
    if (input.indent < LINE_MAX) {
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
    }
    else {
        input.indent = 0;                /* seems like blank lines only */
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



size_t highest (const sentry_t *sarr, const int n, ...)
/*
 *  Return height (vert.) of highest shape in given list.
 *
 *  sarr         array of shapes to examine
 *  n            number of shapes following
 *  ...          the shapes to consider
 *
 *  RETURNS:     height in lines (may be zero)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    va_list ap;
    int i;
    size_t max = 0;                      /* current maximum height */

    #if defined(DEBUG) && 0
        fprintf (stderr, "highest (%d, ...)\n", n);
    #endif

    va_start (ap, n);

    for (i=0; i<n; ++i) {
        shape_t r = va_arg (ap, shape_t);
        if (!isempty (sarr + r)) {
            if (sarr[r].height > max)
                max = sarr[r].height;
        }
    }

    va_end (ap);

    return max;
}



size_t widest (const sentry_t *sarr, const int n, ...)
/*
 *  Return width (horiz.) of widest shape in given list.
 *
 *  sarr         array of shapes to examine
 *  n            number of shapes following
 *  ...          the shapes to consider
 *
 *  RETURNS:     width in chars (may be zero)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    va_list ap;
    int i;
    size_t max = 0;                      /* current maximum width */

    #if defined(DEBUG) && 0
        fprintf (stderr, "widest (%d, ...)\n", n);
    #endif

    va_start (ap, n);

    for (i=0; i<n; ++i) {
        shape_t r = va_arg (ap, shape_t);
        if (!isempty (sarr + r)) {
            if (sarr[r].width > max)
                max = sarr[r].width;
        }
    }
    
    va_end (ap);

    return max;
}



static int horiz_precalc (const sentry_t *sarr,
        size_t *topiltf, size_t *botiltf, size_t *hspace)
/*
 *  Calculate data for horizontal box side generation.
 *
 *  sarr     Array of shapes from the current design
 *
 *  topiltf  RESULT: individual lines (columns) to fill by shapes 1, 2, and 3
 *  botiltf          in top part of box (topiltf) and bottom part of box
 *  hspace   RESULT: number of columns excluding corners (sum over iltf)
 * 
 *  RETURNS:  == 0   on success  (result values are set)
 *            != 0   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int    tnumsh;               /* number of existent shapes in top part */
    int    bnumsh;
    size_t twidth;               /* current hspace for top side */
    size_t bwidth;               /* current hspace for bottom side */
    int    i;
    size_t target_width;         /* assumed text width for minimum box size */
    int    btoggle, ttoggle;     /* for case 3 w/ 2 elastics */

    /*
     *  Initialize future result values
     */
    memset (topiltf, 0, (SHAPES_PER_SIDE-2) * sizeof(size_t));
    memset (botiltf, 0, (SHAPES_PER_SIDE-2) * sizeof(size_t));
    *hspace = 0;

    /*
     *  Ensure minimum width for the insides of a box in order to ensure
     *  minimum box size required by current design
     */
    if (input.maxline >= (opt.design->minwidth - sarr[north_side[0]].width -
            sarr[north_side[SHAPES_PER_SIDE-1]].width)) {
        target_width = input.maxline;
    }
    else {
        target_width = opt.design->minwidth - sarr[north_side[0]].width -
            sarr[north_side[SHAPES_PER_SIDE-1]].width;
    }

    /*
     *  Compute number of existent shapes in top and in bottom part
     */
    tnumsh = 0;  bnumsh = 0;
    for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
        if (!isempty(sarr+north_side[i])) tnumsh++;
        if (!isempty(sarr+south_side[i])) bnumsh++;
    }

    #ifdef DEBUG
        fprintf (stderr, "in horiz_precalc:\n    ");
        fprintf (stderr, "opt.design->minwidth %d, input.maxline %d, target_width"
                " %d, tnumsh %d, bnumsh %d\n", opt.design->minwidth,
                input.maxline, target_width, tnumsh, bnumsh);
    #endif

    twidth = 0;
    bwidth = 0;

    btoggle = 1;                         /* can be 1 or 3 */
    ttoggle = 1;

    do {
        shape_t *seite;                  /* ptr to north_side or south_side */
        size_t *iltf;                    /* ptr to botiltf or topiltf */
        size_t *res_hspace;              /* ptr to bwidth or twidth */
        int *stoggle;                    /* ptr to btoggle or ttoggle */
        int numsh;                       /* either bnumsh or tnumsh */

        /*
         *  Set pointers to the side which is currently shorter,
         *  so it will be advanced in this step.
         */
        if (twidth > bwidth) {           /* south (bottom) is behind */
            seite = south_side;
            iltf = botiltf;
            res_hspace = &bwidth;
            numsh = bnumsh;
            stoggle = &btoggle;
        }
        else {                           /* north (top) is behind */
            seite = north_side;
            iltf = topiltf;
            res_hspace = &twidth;
            numsh = tnumsh;
            stoggle = &ttoggle;
        }

        switch (numsh) {

            case 1:
                /*
                 *  only one shape -> it must be elastic
                 */
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!isempty(&(sarr[seite[i]]))) {
                        if (iltf[i-1] == 0 ||
                            *res_hspace < target_width ||
                            twidth != bwidth)
                        {
                            iltf[i-1] += sarr[seite[i]].width;
                            *res_hspace += sarr[seite[i]].width;
                        }
                        break;
                    }
                }
                break;

            case 2:
                /*
                 *  two shapes -> one must be elastic, the other must not
                 */
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!isempty (sarr+seite[i]) && !(sarr[seite[i]].elastic)) {
                        if (iltf[i-1] == 0) {
                            iltf[i-1] += sarr[seite[i]].width;
                            *res_hspace += sarr[seite[i]].width;
                            break;
                        }
                    }
                }
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!isempty (sarr+seite[i]) && sarr[seite[i]].elastic) {
                        if (iltf[i-1] == 0 ||
                            *res_hspace < target_width ||
                            twidth != bwidth)
                        {
                            iltf[i-1] += sarr[seite[i]].width;
                            *res_hspace += sarr[seite[i]].width;
                        }
                        break;
                    }
                }
                break;

            case 3:
                /*
                 *  three shapes -> one or two of them must be elastic
                 *  If two are elastic, they are the two outer ones.
                 */
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!(sarr[seite[i]].elastic) && iltf[i-1] == 0) {
                        iltf[i-1] += sarr[seite[i]].width;
                        *res_hspace += sarr[seite[i]].width;
                    }
                }
                if (sarr[seite[1]].elastic && sarr[seite[3]].elastic) {
                    if (iltf[*stoggle-1] == 0 ||
                        *res_hspace < target_width ||
                        twidth != bwidth)
                    {
                        *res_hspace += sarr[seite[*stoggle]].width;
                        iltf[*stoggle-1] += sarr[seite[*stoggle]].width;
                    }
                    *stoggle = *stoggle == 1? 3 : 1;
                }
                else {
                    for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                        if (sarr[seite[i]].elastic) {
                            if (iltf[i-1] == 0 ||
                                *res_hspace < target_width ||
                                twidth != bwidth)
                            {
                                iltf[i-1] += sarr[seite[i]].width;
                                *res_hspace += sarr[seite[i]].width;
                            }
                            break;
                        }
                    }
                }
                break;

            default:
                fprintf (stderr, "%s: internal error in horiz_precalc()\n", PROJECT);
                return 1;
        }

    } while (twidth != bwidth || twidth < target_width || bwidth < target_width);

    *hspace = twidth;                    /* return either one */

    return 0;                            /* all clear */
}



static int vert_precalc (const sentry_t *sarr,
        size_t *leftiltf, size_t *rightiltf, size_t *vspace)
/*
 *  Calculate data for vertical box side generation.
 *
 *  sarr      Array of shapes from the current design
 *
 *  leftiltf  RESULT: individual lines to fill by shapes 1, 2, and 3
 *  rightiltf         in left part of box (leftiltf) and right part of box
 *  vspace    RESULT: number of columns excluding corners (sum over iltf)
 * 
 *  RETURNS:  == 0   on success  (result values are set)
 *            != 0   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int    lnumsh;               /* number of existent shapes in top part */
    int    rnumsh;
    size_t lheight;              /* current vspace for top side */
    size_t rheight;              /* current vspace for bottom side */
    int    i;
    size_t target_height;        /* assumed text height for minimum box size */
    int    rtoggle, ltoggle;     /* for case 3 w/ 2 elastics */

    /*
     *  Initialize future result values
     */
    memset (leftiltf, 0, (SHAPES_PER_SIDE-2) * sizeof(size_t));
    memset (rightiltf, 0, (SHAPES_PER_SIDE-2) * sizeof(size_t));
    *vspace = 0;

    /*
     *  Ensure minimum height for insides of box in order to ensure
     *  minimum box size required by current design
     */
    if (input.anz_lines >= (opt.design->minheight - sarr[west_side[0]].height -
            sarr[west_side[SHAPES_PER_SIDE-1]].height)) {
        target_height = input.anz_lines;
    }
    else {
        target_height = opt.design->minheight - sarr[west_side[0]].height -
            sarr[west_side[SHAPES_PER_SIDE-1]].height;
    }

    /*
     *  Compute number of existent shapes in left and right part (1..3)
     */
    lnumsh = 0;  rnumsh = 0;
    for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
        if (!isempty(sarr+west_side[i])) lnumsh++;
        if (!isempty(sarr+east_side[i])) rnumsh++;
    }

    lheight = 0;
    rheight = 0;

    rtoggle = 1;                         /* can be 1 or 3 */
    ltoggle = 1;

    do {
        shape_t *seite;                  /* ptr to west_side or east_side */
        size_t *iltf;                    /* ptr to rightiltf or leftiltf */
        size_t *res_vspace;              /* ptr to rheight or lheight */
        int *stoggle;                    /* ptr to rtoggle or ltoggle */
        int numsh;                       /* either rnumsh or lnumsh */

        /*
         *  Set pointers to the side which is currently shorter,
         *  so it will be advanced in this step.
         */
        if (lheight > rheight) {         /* east (right) is behind */
            seite = east_side;
            iltf = rightiltf;
            res_vspace = &rheight;
            numsh = rnumsh;
            stoggle = &rtoggle;
        }
        else {                           /* west (left) is behind */
            seite = west_side;
            iltf = leftiltf;
            res_vspace = &lheight;
            numsh = lnumsh;
            stoggle = &ltoggle;
        }

        switch (numsh) {

            case 1:
                /*
                 *  only one shape -> it must be elastic
                 */
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!isempty(&(sarr[seite[i]]))) {
                        if (iltf[i-1] == 0 ||
                            *res_vspace < target_height ||
                            lheight != rheight)
                        {
                            iltf[i-1] += sarr[seite[i]].height;
                            *res_vspace += sarr[seite[i]].height;
                        }
                        break;
                    }
                }
                break;

            case 2:
                /*
                 *  two shapes -> one must be elastic, the other must not
                 */
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!isempty (sarr+seite[i]) && !(sarr[seite[i]].elastic)) {
                        if (iltf[i-1] == 0) {
                            iltf[i-1] += sarr[seite[i]].height;
                            *res_vspace += sarr[seite[i]].height;
                            break;
                        }
                    }
                }
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!isempty (sarr+seite[i]) && sarr[seite[i]].elastic) {
                        if (iltf[i-1] == 0 ||
                            *res_vspace < target_height ||
                            lheight != rheight)
                        {
                            iltf[i-1] += sarr[seite[i]].height;
                            *res_vspace += sarr[seite[i]].height;
                        }
                        break;
                    }
                }
                break;

            case 3:
                /*
                 *  three shapes -> one or two of them must be elastic
                 *  If two are elastic, they are the two outer ones.
                 */
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!(sarr[seite[i]].elastic) && iltf[i-1] == 0) {
                        iltf[i-1] += sarr[seite[i]].height;
                        *res_vspace += sarr[seite[i]].height;
                    }
                }
                if (sarr[seite[1]].elastic && sarr[seite[3]].elastic) {
                    if (iltf[*stoggle-1] == 0 ||
                        *res_vspace < target_height ||
                        lheight != rheight)
                    {
                        *res_vspace += sarr[seite[*stoggle]].height;
                        iltf[*stoggle-1] += sarr[seite[*stoggle]].height;
                    }
                    *stoggle = *stoggle == 1? 3 : 1;
                }
                else {
                    for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                        if (sarr[seite[i]].elastic) {
                            if (iltf[i-1] == 0 ||
                                *res_vspace < target_height ||
                                lheight != rheight)
                            {
                                iltf[i-1] += sarr[seite[i]].height;
                                *res_vspace += sarr[seite[i]].height;
                            }
                            break;
                        }
                    }
                }
                break;

            default:
                fprintf (stderr, "%s: internal error in vert_precalc()\n", PROJECT);
                return 1;
        }

    } while (lheight != rheight || lheight < target_height || rheight < target_height);

    *vspace = lheight;                   /* return either one */

    return 0;                            /* all clear */
}



static int vert_assemble (const sentry_t *sarr, const shape_t *seite,
        size_t *iltf, sentry_t *result)
/*
 *
 *  RETURNS:  == 0   on success  (result values are set)
 *            != 0   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t  j;
    size_t  line;
    int     cshape;                      /* current shape (idx to iltf) */

    for (line=0; line<result->height; ++line) {
        result->chars[line] = (char *) calloc (1, result->width+1);
        if (result->chars[line] == NULL) {
            perror (PROJECT);
            if ((long)--line >= 0) do {
                BFREE (result->chars[line--]);
            } while ((long)line >= 0);
            return 1;                    /* out of memory */
        }
    }

    cshape = (seite == north_side)? 0 : 2;

    for (j=0; j<result->width; j+=sarr[seite[cshape+1]].width) {
        while (iltf[cshape] == 0)
            cshape += (seite == north_side)? 1 : -1;
        for (line=0; line<result->height; ++line)
            strcat (result->chars[line], sarr[seite[cshape+1]].chars[line]);
        iltf[cshape] -= sarr[seite[cshape+1]].width;
    }

    return 0;                            /* all clear */
}



static void horiz_assemble (const sentry_t *sarr, const shape_t *seite,
        size_t *iltf, sentry_t *result)
{
    size_t  j;
    size_t  sc;                          /* index to shape chars (lines) */
    int     cshape;                      /* current shape (idx to iltf) */
    shape_t ctop, cbottom;

    if (seite == east_side) {
        ctop = seite[0];
        cbottom = seite[SHAPES_PER_SIDE-1];
        cshape = 0;
    }
    else {
        ctop = seite[SHAPES_PER_SIDE-1];
        cbottom = seite[0];
        cshape = 2;
    }

    for (j=0; j<sarr[ctop].height; ++j)
        result->chars[j] = sarr[ctop].chars[j];
    for (j=0; j<sarr[cbottom].height; ++j)
        result->chars[result->height-sarr[cbottom].height+j] =
            sarr[cbottom].chars[j];

    sc = 0;
    for (j=sarr[ctop].height; j < result->height-sarr[cbottom].height; ++j)
    {
        while (iltf[cshape] == 0) {
            if (seite == east_side)
                ++cshape;
            else
                --cshape;
            sc = 0;
        }
        if (sc == sarr[seite[cshape+1]].height)
            sc = 0;
        result->chars[j] = sarr[seite[cshape+1]].chars[sc];
        ++sc;
        iltf[cshape] -= 1;
    }
}



static int horiz_generate (sentry_t *tresult, sentry_t *bresult)
/*
 *  Generate top and bottom parts of box (excluding corners).
 *
 *  RETURNS:  == 0  if successful (resulting char array is stored in [bt]result)
 *            != 0  on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t biltf[SHAPES_PER_SIDE-2];     /* individual lines to fill (bottom) */
    size_t tiltf[SHAPES_PER_SIDE-2];     /* individual lines to fill (top) */
    int    rc;                           /* received return code */

    tresult->height = highest (opt.design->shape,
            SHAPES_PER_SIDE, NW, NNW, N, NNE, NE);
    bresult->height = highest (opt.design->shape,
            SHAPES_PER_SIDE, SW, SSW, S, SSE, SE);

    rc = horiz_precalc (opt.design->shape, tiltf, biltf, &(tresult->width));
    if (rc) return rc;
    bresult->width = tresult->width;

    #ifdef DEBUG
        fprintf (stderr, "Top side box rect width %d, height %d.\n",
                tresult->width, tresult->height);
        fprintf (stderr, "Top columns to fill: %s %d, %s %d, %s %d.\n",
                shape_name[north_side[1]], tiltf[0],
                shape_name[north_side[2]], tiltf[1],
                shape_name[north_side[3]], tiltf[2]);
        fprintf (stderr, "Bottom side box rect width %d, height %d.\n",
                bresult->width, bresult->height);
        fprintf (stderr, "Bottom columns to fill: %s %d, %s %d, %s %d.\n",
                shape_name[south_side[1]], biltf[0],
                shape_name[south_side[2]], biltf[1],
                shape_name[south_side[3]], biltf[2]);
    #endif

    tresult->chars = (char **) calloc (tresult->height, sizeof(char *));
    bresult->chars = (char **) calloc (bresult->height, sizeof(char *));
    if (tresult->chars == NULL || bresult->chars == NULL) return 1;

    rc = vert_assemble (opt.design->shape, north_side, tiltf, tresult);
    if (rc) return rc;
    rc = vert_assemble (opt.design->shape, south_side, biltf, bresult);
    if (rc) return rc;

    #if defined(DEBUG) && 1
    {
        /*
         *  Debugging code - Output horizontal sides of box
         */
        size_t j;
        fprintf (stderr, "TOP SIDE:\n");
        for (j=0; j<tresult->height; ++j) {
            fprintf (stderr, "  %2d: \'%s\'\n", j,
                    tresult->chars[j]? tresult->chars[j] : "(null)");
        }
        fprintf (stderr, "BOTTOM SIDE:\n");
        for (j=0; j<bresult->height; ++j) {
            fprintf (stderr, "  %2d: \'%s\'\n", j,
                    bresult->chars[j]? bresult->chars[j] : "(null)");
        }
    }
    #endif

    return 0;                            /* all clear */
}



static int vert_generate (sentry_t *lresult, sentry_t *rresult)
/*
 *  Generate vertical sides of box.
 *
 *  RETURNS:  == 0   on success  (resulting char array is stored in [rl]result)
 *            != 0   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t vspace = 0;
    size_t leftiltf[SHAPES_PER_SIDE-2];  /* individual lines to fill */
    size_t rightiltf[SHAPES_PER_SIDE-2]; /* individual lines to fill */
    int    rc;                           /* received return code */

    lresult->width = widest (opt.design->shape,
            SHAPES_PER_SIDE, SW, WSW, W, WNW, NW);
    rresult->width = widest (opt.design->shape,
            SHAPES_PER_SIDE, SE, ESE, E, ENE, NE);

    rc = vert_precalc (opt.design->shape, leftiltf, rightiltf, &vspace);
    if (rc) return rc;

    lresult->height = vspace +
        opt.design->shape[NW].height + opt.design->shape[SW].height;
    rresult->height = vspace +
        opt.design->shape[NE].height + opt.design->shape[SE].height;

    #ifdef DEBUG
        fprintf (stderr, "Left side box rect width %d, height %d, vspace %d.\n",
                lresult->width, lresult->height, vspace);
        fprintf (stderr, "Left lines to fill: %s %d, %s %d, %s %d.\n",
                shape_name[west_side[1]], leftiltf[0],
                shape_name[west_side[2]], leftiltf[1],
                shape_name[west_side[3]], leftiltf[2]);
        fprintf (stderr, "Right side box rect width %d, height %d, vspace %d.\n",
                rresult->width, rresult->height, vspace);
        fprintf (stderr, "Right lines to fill: %s %d, %s %d, %s %d.\n",
                shape_name[east_side[1]], rightiltf[0],
                shape_name[east_side[2]], rightiltf[1],
                shape_name[east_side[3]], rightiltf[2]);
    #endif

    lresult->chars = (char **) calloc (lresult->height, sizeof(char *));
    if (lresult->chars == NULL) return 1;
    rresult->chars = (char **) calloc (rresult->height, sizeof(char *));
    if (rresult->chars == NULL) return 1;

    horiz_assemble (opt.design->shape, west_side, leftiltf, lresult);
    horiz_assemble (opt.design->shape, east_side, rightiltf, rresult);

    #if defined(DEBUG) && 1
    {
        /*
         *  Debugging code - Output left and right side of box
         */
        size_t j;
        fprintf (stderr, "LEFT SIDE:\n");
        for (j=0; j<lresult->height; ++j) {
            fprintf (stderr, "  %2d: \'%s\'\n", j,
                    lresult->chars[j]? lresult->chars[j] : "(null)");
        }
        fprintf (stderr, "RIGHT SIDE:\n");
        for (j=0; j<rresult->height; ++j) {
            fprintf (stderr, "  %2d: \'%s\'\n", j,
                    rresult->chars[j]? rresult->chars[j] : "(null)");
        }
    }
    #endif

    return 0;                            /* all clear */
}



static int generate_box (sentry_t *thebox)
/*
 *
 *  RETURNS:  == 0  if successful  (thebox is set)
 *            != 0  on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int rc;
    int i;

    rc = horiz_generate (&(thebox[0]), &(thebox[2]));
    if (rc) goto err;

    rc = vert_generate (&(thebox[3]), &(thebox[1]));
    if (rc) goto err;

    return 0;                            /* all clear */

err:
    for (i=0; i<ANZ_SIDES; ++i) {
        if (!isempty(&(thebox[i]))) {
            BFREE (thebox[i].chars);     /* free only pointer array */
            memset (thebox+i, 0, sizeof(sentry_t));
        }
    }
    return rc;                           /* error */
}



static design_t *select_design (design_t *darr, char *sel)
/*
 *  Select a design to use for our box.
 *
 *  darr      design array as read from config file
 *  sel       name of desired design
 *
 *  If the specified name is not found, defaults to "C" design;
 *  If "C" design is not found, default to design number 0;
 *  If there are no designs, print error message and return error.
 *
 *  RETURNS:  pointer to current design   on success
 *            NULL                        on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i;

    if (sel) {
        opt.design_choice_by_user = 1;
        for (i=0; i<anz_designs; ++i) {
            if (strcasecmp (darr[i].name, sel) == 0)
                return &(darr[i]);
        }
        fprintf (stderr, "%s: unknown box design -- %s\n", PROJECT, sel);
        return NULL;
    }

    for (i=0; i<anz_designs; ++i) {
        if (strcasecmp (darr[i].name, "C") == 0)
            return &(darr[i]);
    }

    if (darr[0].name != NULL)
        return darr;

    fprintf (stderr, "%s: Internal error -- no box designs found\n", PROJECT);
    return NULL;
}



static int output_box (const sentry_t *thebox)
/*
 *
 *  RETURNS:  == 0  if successful
 *            != 0  on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t j;
    size_t nol = thebox[BRIG].height;    /* number of output lines */
    char   trailspc[LINE_MAX+1];
    char  *indentspc;
    size_t vfill, vfill1, vfill2;        /* empty lines/columns in box */
    size_t hfill;
    char  *hfill1, *hfill2;              /* space before/after text */
    size_t r;
    char   obuf[LINE_MAX+1];             /* final output buffer */
    size_t obuf_len;                     /* length of content of obuf */

    /*
     *  Create string of spaces for indentation
     */
    if (opt.design->indentmode == 'b') {
        indentspc = (char *) malloc (input.indent+1);
        if (indentspc == NULL) {
            perror (PROJECT);
            return 1;
        }
        memset (indentspc, (int)' ', input.indent);
        indentspc[input.indent] = '\0';
    }
    else {
        indentspc = (char *) strdup ("");
        if (indentspc == NULL) {
            perror (PROJECT);
            return 1;
        }
    }

    /*
     *  Provide string of spaces for filling of space between text and
     *  right side of box
     */
    memset (trailspc, (int)' ', LINE_MAX);
    trailspc[LINE_MAX] = '\0';

    /*
     *  Compute number of empty lines in box (vfill).
     */
    vfill = nol - thebox[BTOP].height - thebox[BBOT].height - input.anz_lines;
    if (opt.valign == 'c') {
        vfill1 = vfill / 2;
        vfill2 = vfill1 + ((vfill % 2)? 1 : 0);
    }
    else if (opt.valign == 'b') {
        vfill1 = vfill;
        vfill2 = 0;
    }
    else {
        vfill1 = 0;
        vfill2 = vfill;
    }

    /*
     *  Provide strings for horizontal text alignment.
     */
    hfill = thebox[BTOP].width - input.maxline;
    hfill1 = (char *) malloc (hfill+1);
    hfill2 = (char *) malloc (hfill+1);
    if (!hfill1 || !hfill2) {
        perror (PROJECT);
        return 1;
    }
    memset (hfill1, (int)' ', hfill+1);
    memset (hfill2, (int)' ', hfill+1);
    hfill1[hfill] = '\0';
    hfill2[hfill] = '\0';
    if (hfill == 0) {
        hfill1[0] = '\0';
        hfill2[0] = '\0';
    }
    else if (hfill == 1) {
        if (opt.halign == 'r') {
            hfill1[1] = '\0';
            hfill2[0] = '\0';
        }
        else {
            hfill1[0] = '\0';
            hfill2[1] = '\0';
        }
    }
    else {
        if (opt.halign == 'c') {
            hfill1[hfill/2] = '\0';
            if (hfill % 2)
                hfill2[hfill/2+1] = '\0';
            else
                hfill2[hfill/2] = '\0';
        }
        else if (opt.halign == 'r') {
            hfill2[0] = '\0';
        }
        else {
            hfill1[0] = '\0';
        }
    }

    #if defined(DEBUG)
        fprintf (stderr, "Alignment: hfill %d hfill1 %dx\' \' hfill2 %dx\' \' "
                "vfill %d vfill1 %d vfill2 %d.\n", hfill, strlen(hfill1),
                strlen(hfill2), vfill, vfill1, vfill2);
    #endif

    /*
     *  Generate actual output
     */
    for (j=0; j<nol; ++j) {

        if (j < thebox[BTOP].height) {
            snprintf (obuf, LINE_MAX+1, "%s%s%s%s", indentspc,
                    thebox[BLEF].chars[j], thebox[BTOP].chars[j],
                    thebox[BRIG].chars[j]);
        }

        else if (vfill1) {
            r = thebox[BTOP].width;
            trailspc[r] = '\0';
            snprintf (obuf, LINE_MAX+1, "%s%s%s%s", indentspc,
                    thebox[BLEF].chars[j], trailspc, thebox[BRIG].chars[j]);
            trailspc[r] = ' ';
            --vfill1;
        }

        else if (j < nol-thebox[BBOT].height) {
            long ti = j - thebox[BTOP].height - (vfill-vfill2);
            if (ti < (long) input.anz_lines) {
                r = input.maxline - input.lines[ti].len;
                trailspc[r] = '\0';
                snprintf (obuf, LINE_MAX+1, "%s%s%s%s%s%s%s", indentspc,
                        thebox[BLEF].chars[j], hfill1,
                        ti >= 0? input.lines[ti].text : "", hfill2,
                        trailspc, thebox[BRIG].chars[j]);
            }
            else {
                r = thebox[BTOP].width;
                trailspc[r] = '\0';
                snprintf (obuf, LINE_MAX+1, "%s%s%s%s", indentspc,
                        thebox[BLEF].chars[j], trailspc, thebox[BRIG].chars[j]);
            }
            trailspc[r] = ' ';
        }

        else {
            snprintf (obuf, LINE_MAX+1, "%s%s%s%s", indentspc,
                    thebox[BLEF].chars[j],
                    thebox[BBOT].chars[j-(nol-thebox[BBOT].height)],
                    thebox[BRIG].chars[j]);
        }

        obuf_len = strlen (obuf);        /* TODO Can't we compute this?! */

        if (obuf_len > LINE_MAX) {
            size_t newlen = LINE_MAX;
            btrim (obuf, &newlen);
        }
        else {
            btrim (obuf, &obuf_len);
        }

        fprintf (opt.outfile, "%s\n", obuf);
    }

    BFREE (indentspc);
    BFREE (hfill1);
    BFREE (hfill2);
    return 0;                            /* all clear */
}



char *strrstr (const char *s1, const char *s2, const size_t s2_len, int skip)
/*
 *  Return pointer to last occurrence of string s2 in string s1.
 *
 *      s1       string to search
 *      s2       string to search for in s1
 *      s2_len   length in characters of s2
 *      skip     number of finds to ignore before returning anything
 *
 *  RETURNS: pointer to last occurrence of string s2 in string s1
 *           NULL if not found or error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char *p;
    int comp;

    if (!s2 || *s2 == '\0')
        return (char *) s1;
    if (!s1 || *s1 == '\0')
        return NULL;
    if (skip < 0)
        skip = 0;

    p = strrchr (s1, s2[0]);
    if (!p)
        return NULL;

    while (p >= s1) {
        comp = strncmp (p, s2, s2_len);
        if (comp == 0) {
            if (skip--)
                --p;
            else
                return p;
        }
        else {
            --p;
        }
    }

    return NULL;
}



int best_match (const line_t *line, char **ws, char **we, char **es, char **ee)
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
    size_t    numw = 0;                /* number of shape lines on west side */
    size_t    nume = 0;                /* number of shape lines on east side */
    size_t    j;                       /* counts number of lines of all shapes tested */
    size_t    k;                       /* line counter within shape */
    int       w;                       /* shape counter */
    sentry_t *cs;                      /* current shape */
    char     *s;                       /* duplicate of current shape part */
    char     *p;                       /* position found by strstr */
    size_t    cq;                      /* current quality */
    char     *q;                       /* space check rover */
    size_t    quality;

    *ws = *we = *es = *ee = NULL;

    numw  = opt.design->shape[WNW].height;
    numw += opt.design->shape[ W ].height;
    numw += opt.design->shape[WSW].height;

    nume  = opt.design->shape[ENE].height;
    nume += opt.design->shape[ E ].height;
    nume += opt.design->shape[ESE].height;

    /*
     *  Find match for WEST side
     */
    quality = 0;
    cs = opt.design->shape + WNW;
    for (j=0,k=0,w=3; j<numw; ++j,++k) {
        if (k == cs->height) {
            k = 0;
            cs = opt.design->shape + west_side[--w];
        }

        s = (char *) strdup (cs->chars[k]);
        if (s == NULL) {
            perror (PROJECT);
            return -1;
        }
        cq = cs->width;

        do {
            p = strstr (line->text, s);
            if (p) {
                q = p-1;
                while (q >= line->text) {
                    if (*q-- != ' ') {
                        p = NULL;
                        break;
                    }
                }
                if (p)
                    break;
            }
            if (!p && cq) {
                if (*s == ' ')
                    memmove (s, s+1, cq--);
                else if (s[cq-1] == ' ')
                    s[--cq] = '\0';
                else {
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

    /*
     *  Find match for EAST side
     */
    quality = 0;
    cs = opt.design->shape + ENE;
    for (j=0,k=0,w=1; j<numw; ++j,++k) {
        if (k == cs->height) {
            k = 0;
            cs = opt.design->shape + east_side[++w];
        }

        s = (char *) strdup (cs->chars[k]);
        if (s == NULL) {
            perror (PROJECT);
            return -1;
        }
        cq = cs->width;

        do {
            p = strrstr (line->text, s, cq, 0);
            if (p) {
                q = p + cq;
                while (*q) {
                    if (*q++ != ' ') {
                        p = NULL;
                        break;
                    }
                }
                if (p)
                    break;
            }
            if (!p && cq) {
                if (*s == ' ')
                    memmove (s, s+1, cq--);
                else if (s[cq-1] == ' ')
                    s[--cq] = '\0';
                else {
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

    return *ws || *es ? 1:0;
}



shape_t leftmost (const int aside, const int cnt)
/*
 *  Return leftmost existing shape in specification for side aside
 *  (BTOP or BBOT), skipping cnt shapes. Corners are not considered.
 *
 *  RETURNS:    shape       if shape was found
 *              ANZ_SHAPES  on error (e.g. cnt too high)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int c = 0;
    int s;

    if (cnt < 0)
        return ANZ_SHAPES;

    if (aside == BTOP) {
        s = 0;
        do {
            ++s;
            while (s < SHAPES_PER_SIDE-1 &&
                    isempty(opt.design->shape + north_side[s]))
                ++s;
            if (s == SHAPES_PER_SIDE-1)
                return ANZ_SHAPES;
        } while (c++ < cnt);
        return north_side[s];
    }

    else if (aside == BBOT) {
        s = SHAPES_PER_SIDE - 1;
        do {
            --s;
            while (s && isempty(opt.design->shape + south_side[s]))
                --s;
            if (!s)
                return ANZ_SHAPES;
        } while (c++ < cnt);
        return south_side[s];
    }

    return ANZ_SHAPES;
}



static int hmm (const int aside, const size_t follow,
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
 *  Recursive helper function for detect_horiz()
 *
 *  RETURNS:  == 0  success
 *            != 0  error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int       cmp;
    sentry_t *cs;
    shape_t   sh;
    int       rc;

    #ifdef DEBUG
        fprintf (stderr, "hmm (%s, %d, \'%c\', \'%c\', %d)\n",
                aside==BTOP?"BTOP":"BBOT", follow, p[0], *ecs, cnt);
    #endif

    if (p > ecs)                         /* last shape tried was too long */
        return 2;

    sh = leftmost (aside, cnt);
    if (sh == ANZ_SHAPES)
        return 1;

    cs = opt.design->shape + sh;

    cmp = strncmp (p, cs->chars[follow], cs->width);

    if (cmp == 0) {
        if (p+cs->width == ecs) {
            if (leftmost (aside, cnt+1) == ANZ_SHAPES)
                return 0;                /* good! all clear, it matched */
            else
                return 3;                /* didn't use all shapes to do it */
        }
        if (cs->elastic) {
            rc = hmm (aside, follow, p+cs->width, ecs, cnt);
            #ifdef DEBUG
                fprintf (stderr, "hmm returned %d\n", rc);
            #endif
            if (rc) {
                rc = hmm (aside, follow, p+cs->width, ecs, cnt+1);
                #ifdef DEBUG
                    fprintf (stderr, "hmm returned %d\n", rc);
                #endif
            }
        }
        else {
            rc = hmm (aside, follow, p+cs->width, ecs, cnt+1);
            #ifdef DEBUG
                fprintf (stderr, "hmm returned %d\n", rc);
            #endif
        }
        if (rc == 0)
            return 0;                    /* we're on the way back */
        else
            return 4;                    /* can't continue on this path */
    }
    else {
        return 5;                        /* no match */
    }
}



int detect_horiz (const int aside, size_t *hstart, size_t *hend)
/*
 *  Detect which part of the input belongs to the top of the box
 *
 *      aside   part of box to detect (BTOP or BBOT)
 *      hstart  index of first line of detected box part (result)
 *      hend    index of first line following detected box part (result)
 *
 *  We assume the horizontal parts of the box to be in one piece, i.e. no
 *  blank lines inserted. Lines may be missing, though. Lines may not be
 *  duplicated. They may be shifted left and right by inserting whitespace,
 *  but whitespace which is part of the box must not have been deleted.
 *  Unfortunately, they may even differ in length as long as each line is
 *  in itself a valid horizontal box line.
 *
 *  RETURNS:  == 0   success (hstart & hend are set)
 *            != 0   error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t    follow;                    /* possible box line */
    sentry_t *cs;                        /* current shape */
    line_t   *line;                      /* currently processed input line */
    size_t    lcnt;                      /* index of currently proc.inp.line */
    char     *p;                         /* middle line part scanner */
    char     *q;                         /* space check rover */
    char     *wcs = NULL;                /* west corner shape position */
    char     *ecs = NULL;                /* east corner shape position */
    char     *ecs_save;                  /* temp copy of ecs */
    int       mmok;                      /* true if middle match was ok */
    size_t    mheight;                   /* regular height of box part */
    int       result_init = 0;           /* true if hstart was set */
    int       goeast, gowest;

    *hstart = *hend = 0;

    mheight = opt.design->shape[sides[aside][0]].height;
    if (aside == BTOP) {
        follow = 0;
        line=input.lines;
    }
    else {
        follow = mheight - 1;
        line = input.lines + input.anz_lines - 1;
    }

    for (lcnt=0; lcnt<mheight; ++lcnt) {

        goeast = gowest = 0;

        #ifdef DEBUG
            if (aside == BTOP)
                fprintf (stderr, "----- Processing line index %2d --------------"
                        "---------------------------------\n", lcnt);
            else
                fprintf (stderr, "----- Processing line index %2d -----------"
                        "------------------------------------\n",
                        input.anz_lines - lcnt - 1);
        #endif

        do {
            /*
             *  Look for west corner shape
             */
            cs = opt.design->shape + sides[aside][aside==BTOP?0:SHAPES_PER_SIDE-1];
            if (gowest) {
                wcs = strstr (wcs+1, cs->chars[follow]);
                gowest = 0;
            }
            else if (!wcs) {
                wcs = strstr (line->text, cs->chars[follow]);
            }
            if (wcs) {
                for (q=wcs-1; q>=line->text; --q) {
                    if (*q != ' ' && *q != '\t')
                        break;
                }
                if (q >= line->text)
                    wcs = NULL;
            }
            #ifdef DEBUG
                if (wcs)
                    fprintf (stderr, "West corner shape matched at position %d.\n",
                            wcs - line->text);
                else
                    fprintf (stderr, "West corner shape not found.\n");
            #endif

            p = wcs + cs->width;

            /*
             *  Look for east corner shape
             */
            cs = opt.design->shape + sides[aside][aside==BTOP?SHAPES_PER_SIDE-1:0];
            ecs_save = ecs;
            ecs = strrstr (p, cs->chars[follow], cs->width, goeast);
            if (ecs) {
                for (q=ecs+cs->width; *q; ++q) {
                    if (*q != ' ' && *q != '\t')
                        break;
                }
                if (*q)
                    ecs = NULL;
            }
            if (!ecs) {
                gowest = 1;
                goeast = 0;
                ecs = ecs_save;
            }
            #ifdef DEBUG
                if (ecs)
                    fprintf (stderr, "East corner shape matched at position %d.\n",
                            ecs-line->text);
                else
                    fprintf (stderr, "East corner shape not found.\n");
            #endif

            /*
             *  Check if text between corner shapes is valid
             */
            if (wcs && ecs) {
                mmok = !hmm (aside, follow, p, ecs, 0);
                #ifdef DEBUG
                    fprintf (stderr, "Text between corner shapes is%s valid.\n",
                            mmok? "": " NOT");
                #endif
                if (!mmok)
                    ++goeast;
            }

        } while (!mmok && wcs);

        /*
         *  Proceed to next line
         */
        if (wcs && ecs && mmok) {  /* match found */
            if (!result_init) {
                result_init = 1;
                if (aside == BTOP)
                    *hstart = lcnt;
                else
                    *hend = (input.anz_lines - lcnt - 1) + 1;
            }
            if (aside == BTOP)
                *hend = lcnt + 1;
            else
                *hstart = input.anz_lines - lcnt - 1;
        }
        else {
            if (result_init)
                break;
        }
        wcs = NULL;
        ecs = NULL;

        if (aside == BTOP) {
            ++follow;
            ++line;
        }
        else {
            --follow;
            --line;
        }
    }

    return result_init? 0: 1;
}



int remove_box()
/*
 *  foo
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t textstart = 0;            /* index of 1st line of box body */
    size_t textend = 0;              /* index of 1st line of south side */
    size_t boxstart = 0;             /* index of 1st line of box */
    size_t boxend = 0;               /* index of 1st line trailing the box */
    int m;                           /* true if a match was found */
    size_t j;                        /* loop counter */

    #ifdef DEBUG
        fprintf (stderr, "remove_box()\n");
    #endif

    /*
     *  If the user didn't specify a design to remove, autodetect it.
     */
    if (opt.design_choice_by_user == 0) {
        /* TODO */
        fprintf (stderr, "%s: Can\'t autodetect designs yet. Use -d.\n",
                PROJECT);
        return 1;
    }

    /*
     *  Add trailing spaces to input lines (needed for recognition)
     */
    input.maxline += EAST_PADDING;
    for (j=0; j<input.anz_lines; ++j) {
        input.lines[j].text = (char *)
            realloc (input.lines[j].text, input.maxline+1);
        if (input.lines[j].text == NULL) {
            perror (PROJECT);
            return 1;
        }
        memset (input.lines[j].text + input.lines[j].len, ' ',
                input.maxline - input.lines[j].len);
        input.lines[j].text[input.maxline] = '\0';
        input.lines[j].len = input.maxline;
    }

    /*
     *  Debugging Code: Display contents of input structure
     */
    #if defined(DEBUG) && 1
        for (j=0; j<input.anz_lines; ++j) {
            fprintf (stderr, "%3d [%02d] \"%s\"\n", j, input.lines[j].len,
                    input.lines[j].text);
        }
        fprintf (stderr, "\nLongest line: %d characters.\n", input.maxline);
        fprintf (stderr, " Indentation: %2d spaces.\n", input.indent);
    #endif

    /*
     *  Phase 1: Try to find out how many lines belong to the top of the box
     */
    boxstart = 0;
    textstart = 0;
    detect_horiz (BTOP, &boxstart, &textstart);
    #ifdef DEBUG
        fprintf (stderr, "----> First line of box is %d, ", boxstart);
        fprintf (stderr, "first line of box body (text) is %d.\n", textstart);
    #endif


    /*
     *  Phase 2: Find out how many lines belong to the bottom of the box
     */
    textend = 0;
    boxend = 0;
    detect_horiz (BBOT, &textend, &boxend);
    #ifdef DEBUG
        fprintf (stderr, "----> Last line of box body (text) is %d, ", textend-1);
        fprintf (stderr, "last line of box is %d.\n", boxend-1);
    #endif

    /*
     *  Phase 3: Iterate over body lines, removing box sides where applicable
     */
    for (j=textstart; j<textend; ++j) {
        char *ws, *we, *es, *ee;         /* west start & end, east start&end */
        char *p;
        int c;

        m = best_match (input.lines+j, &ws, &we, &es, &ee);
        if (m < 0)
            return 1;                    /* internal error */
        if (m == 0) {
            #ifdef DEBUG
                fprintf (stderr, "line %d: no side match\n", j);
            #endif
        }
        if (m > 0) {
            #ifdef DEBUG
                fprintf (stderr, "\033[00;33mline %2d: west: %d (\'%c\') to "
                    "%d (\'%c\') [len %d];  east: %d (\'%c\') to %d (\'%c\')"
                    " [len %d]\033[00m\n", j,
                    ws? ws-input.lines[j].text:0,   ws?ws[0]:'?',
                    we? we-input.lines[j].text-1:0, we?we[-1]:'?',
                    ws&&we? (we-input.lines[j].text-(ws-input.lines[j].text)):0,
                    es? es-input.lines[j].text:0,   es?es[0]:'?',
                    ee? ee-input.lines[j].text-1:0, ee?ee[-1]:'?',
                    es&&ee? (ee-input.lines[j].text-(es-input.lines[j].text)):0);
            #endif
            if (ws && we) {
                for (p=ws; p<we; ++p)
                    *p = ' ';
            }
            if (es && ee) {
                for (p=es; p<ee; ++p)
                    *p = ' ';
            }
        }
        for (c=0; c<(int)opt.design->shape[NW].width; ++c) {
            if (input.lines[j].text[c] != ' ')
                break;
        }
        memmove (input.lines[j].text, input.lines[j].text + c,
                input.lines[j].len - c);
    }

    /*
     *  Phase 4: Remove box top and body lines from input
     */
    while (empty_line(input.lines+textstart) && textstart < textend) {
        #ifdef DEBUG
            fprintf (stderr, "Killing leading blank line in box body.\n");
        #endif
        ++textstart;
    }
    while (empty_line(input.lines+textend-1) && textend > textstart) {
        #ifdef DEBUG
            fprintf (stderr, "Killing trailing blank line in box body.\n");
        #endif
        --textend;
    }

    if (textstart > boxstart) {
        for (j=boxstart; j<textstart; ++j)
            BFREE (input.lines[j].text);
        memmove (input.lines+boxstart, input.lines+textstart,
                (input.anz_lines-textstart)*sizeof(line_t));
        input.anz_lines -= textstart - boxstart;
        textend -= textstart - boxstart;
        boxend -= textstart - boxstart;
    }
    if (boxend > textend) {
        for (j=textend; j<boxend; ++j)
            BFREE (input.lines[j].text);
        if (boxend < input.anz_lines) {
            memmove (input.lines+textend, input.lines+boxend,
                    (input.anz_lines-boxend)*sizeof(line_t));
        }
        input.anz_lines -= boxend - textend;
    }
    input.maxline = 0;
    for (j=0; j<input.anz_lines; ++j) {
        if (input.lines[j].len > input.maxline)
            input.maxline = input.lines[j].len;
    }
    memset (input.lines + input.anz_lines, 0,
            ((textstart - boxstart > 0 ? textstart - boxstart : 0) +
             (boxend - textend > 0 ? boxend - textend : 0)) * sizeof(line_t));

    #ifdef DEBUG
        #if 0
            for (j=0; j<input.anz_lines; ++j) {
                fprintf (stderr, "%3d [%02d] \"%s\"\n", j, input.lines[j].len,
                        input.lines[j].text);
            }
        #endif
        fprintf (stderr, "Number of lines shrunk by %d.\n",
                (textstart - boxstart > 0 ? textstart - boxstart : 0) +
                (boxend - textend > 0 ? boxend - textend : 0));
    #endif

    return 0;                            /* all clear */
}



void output_input()
{
    size_t j;

    for (j=0; j<input.anz_lines; ++j) {
        if (input.lines[j].text) {
            btrim (input.lines[j].text, &(input.lines[j].len));
            printf ("%s\n", input.lines[j].text);
        }
    }
}



int main (int argc, char *argv[])
{
    extern int yyparse();
    int rc;                              /* general return code */
    design_t *tmp;
    sentry_t *thebox;

    #ifdef DEBUG
        fprintf (stderr, "BOXES STARTING ...\n");
    #endif

    thebox = (sentry_t *) calloc (ANZ_SIDES, sizeof(sentry_t));
    if (thebox == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }

    #ifdef DEBUG
        fprintf (stderr, "Processing Comand Line ...\n");
    #endif

    rc = process_commandline (argc, argv);
    if (rc == 42) exit (EXIT_SUCCESS);
    if (rc) exit (EXIT_FAILURE);

    designs = (design_t *) calloc (1, sizeof(design_t));
    if (designs == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    designs->indentmode = DEF_INDENTMODE;

    /*
     *  If the following parser is one created by lex, the application must
     *  be careful to ensure that LC_CTYPE and LC_COLLATE are set to the
     *  POSIX locale.
     */
    #ifdef DEBUG
        fprintf (stderr, "Parsing Config File ...\n");
    #endif
    rc = yyparse();
    if (rc) exit (EXIT_FAILURE);
    --design_idx;
    tmp = (design_t *) realloc (designs, (design_idx+1)*sizeof(design_t));
    if (tmp) {
        designs = tmp;                   /* yyparse() allocates space */
    }                                    /* for one design too much   */
    else {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    anz_designs = design_idx + 1;

    #ifdef DEBUG
        fprintf (stderr, "Selecting Design ...\n");
    #endif
    tmp = select_design (designs, (char *) opt.design);
    if (tmp == NULL) exit (EXIT_FAILURE);
    BFREE (opt.design);
    opt.design = tmp;

    if (opt.l) {
        rc = list_styles();
        if (rc) {
            perror (PROJECT);
            exit (EXIT_FAILURE);
        }
        exit (EXIT_SUCCESS);
    }

    #ifdef DEBUG
        fprintf (stderr, "Reading all input ...\n");
    #endif
    rc = read_all_input();
    BFREE (yyfilename);
    if (rc) exit (EXIT_FAILURE);
    if (input.anz_lines == 0)
        exit (EXIT_SUCCESS);

    if (opt.reqheight > (long) opt.design->minheight)
        opt.design->minheight = opt.reqheight;
    if (opt.reqwidth > (long) opt.design->minwidth)
        opt.design->minwidth = opt.reqwidth;

    if (opt.r) {
        rc = remove_box();
        if (rc) {
            perror (PROJECT);
            exit (EXIT_FAILURE);
        }
        output_input();
        exit (EXIT_SUCCESS);
    }

    #ifdef DEBUG
        fprintf (stderr, "Generating Box ...\n");
    #endif
    rc = generate_box (thebox);
    if (rc) exit (EXIT_FAILURE);

    #ifdef DEBUG
        fprintf (stderr, "Generating Output ...\n");
    #endif
    output_box (thebox);

    return EXIT_SUCCESS;
}

/*EOF*/                                                  /* vim: set sw=4: */
