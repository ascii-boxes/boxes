/*
 *  File:             boxes.c
 *  Date created:     March 18, 1999 (Thursday, 15:09h)
 *  Author:           Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id$
 *  Language:         ANSI C
 *  Platforms:        sunos5/sparc, for now
 *  World Wide Web:   http://home.pages.de/~jensen/boxes/
 *  Purpose:          Filter to draw boxes around input text.
 *                    Intended for use with vim(1).
 *  Remarks:          This is not a release.
 *
 *  Revision History:
 *
 *    $Log$
 *    Revision 1.1  1999/03/18  15:09:17  tsjensen
 *    Initial revision
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */


#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "boxes.h"


extern char *optarg;                     /* for getopt() */
extern int optind, opterr, optopt;

extern FILE *yyin;                       /* lex input file */

const char rcsid[] = "$Id$";

char *yyfilename = NULL;                 /* file name of config file used */

design_t *designs = NULL;                /* available box designs */

int design_idx = 0;                      /* anz_designs-1 */
int anz_designs = 0;                     /* no of designs after parsing */


char *shape_name[] = {
    "NW", "NNW", "N", "NNE", "NE", "ENE", "E", "ESE",
    "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW"
};

char *ofs_name[] =   {
    "NW_NNW", "NNW_N", "N_NNE", "NNE_NE", "NE_ENE", "ENE_E", "E_ESE", "ESE_SE",
    "SE_SSE", "SSE_S", "S_SSW", "SSW_SW", "SW_WSW", "WSW_W", "W_WNW", "WNW_NW"
};

shape_t north_side[SHAPES_PER_SIDE] = { NW, NNW, N, NNE, NE };  /* clockwise */
shape_t  east_side[SHAPES_PER_SIDE] = { NE, ENE, E, ESE, SE };
shape_t south_side[SHAPES_PER_SIDE] = { SE, SSE, S, SSW, SW };
shape_t  west_side[SHAPES_PER_SIDE] = { SW, WSW, W, WNW, NW };
shape_t corners[ANZ_CORNERS] = { NW, NE, SE, SW };
shape_t *sides[] = { north_side, east_side, south_side, west_side };


struct {                                 /* command line options */
    int l;                               /* list available designs */
} opt;




int yyerror (const char *fmt, ...)
{
    va_list ap;
    char buf[1024];

    va_start (ap, fmt);
    snprintf (buf, 1024-1, "%s: %s: line %d: ", PROJECT,
            yyfilename? yyfilename: "(null)", yylineno);
    vsnprintf (buf+strlen(buf), 1024-strlen(buf)-1, fmt, ap);
    strcat (buf, "\n");
    (void) fprintf (stderr, buf);
    va_end (ap);

    return 0;
}



shape_t *on_side (const shape_t shape1, const shape_t shape2)
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
    shape_t *workside = on_side (s1, s2);

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



void usage (FILE *st)
{
    fprintf (st, "Usage: %s [options] [infile [outfile]]\n", PROJECT);
    fprintf (st, "       -c box   exchange current box for new box\n");
    fprintf (st, "       -f file  use only file as configuration file\n");
    fprintf (st, "       -h       usage information\n");
    fprintf (st, "       -l       generate listing of available box designs w/ samples\n");
    fprintf (st, "       -r       repair broken box\n");
    fprintf (st, "       -s wxh   resize box to width w and/or height h\n");
    fprintf (st, "       -v       print version information\n");
    fprintf (st, "       -x       remove box from text\n");
}



int process_commandline (int argc, char *argv[])
{
    int oc;                              /* option character */
    FILE *f;                             /* potential input file */

    memset (&opt, 0, sizeof(opt));
    yyin = stdin;

    do {
        oc = getopt (argc, argv, "f:hlv");

        switch (oc) {

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
                buf[PATH_MAX-1-7] = '\0';    /* ensure space for /.boxes */
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

    return 0;
}



int style_sort (const void *p1, const void *p2)
{
    return strcasecmp ((const char *) ((*((design_t **) p1))->name),
                       (const char *) ((*((design_t **) p2))->name));
}

int list_styles()
{
    int i;
    design_t **list;

    list = (design_t **) calloc (design_idx+1, sizeof(design_t *));
    if (list == NULL) return 1;

    for (i=0; i<=design_idx; ++i)
        list[i] = &(designs[i]);
    qsort (list, design_idx+1, sizeof(design_t *), style_sort);

    printf ("Available Styles:\n");
    printf ("-----------------\n\n");
    for (i=0; i<=design_idx; ++i)
        printf ("%s (%s):\n\n%s\n\n", list[i]->name,
                list[i]->author? list[i]->author: "unknown artist",
                list[i]->sample);

    free (list);
    list = NULL;

    return 0;
}



int main (int argc, char *argv[])
{
    extern int yyparse();
    int rc;                              /* general return code */
    design_t *tmp = (design_t *) rcsid;  /* make rcsid stay */

    rc = process_commandline (argc, argv);
    if (rc == 42) exit (EXIT_SUCCESS);
    if (rc) exit (EXIT_FAILURE);

    setlocale (LC_ALL, "");              /* muﬂ des sein? TODO */

    designs = (design_t *) calloc (1, sizeof(design_t));
    if (designs == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }

    /*
     *  If the following parser is one created by lex, the application must
     *  be careful to ensure that LC_CTYPE and LC_COLLATE are set to the
     *  POSIX locale.
     */
    rc = yyparse();
    if (rc) exit (EXIT_FAILURE);
    if (yyfilename) {
        free (yyfilename);
        yyfilename = NULL;
    }
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


    if (opt.l) {
        rc = list_styles();
        if (rc) {
            perror (PROJECT);
            exit (EXIT_FAILURE);
        }
        exit (EXIT_SUCCESS);
    }


    return EXIT_SUCCESS;
}

/*EOF*/                                                  /* vim: set sw=4: */
