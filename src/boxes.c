
#include <locale.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "boxes.h"


extern char *optarg;                     /* for getopt() */
extern int optind, opterr, optopt;

extern FILE *yyin;                       /* lex input file */

char *yyfilename = NULL;                 /* file name of config file used */

design_t *designs = NULL;                /* available box designs */

int design_idx = 0;                      /* anz_designs-1 */
int anz_designs = 0;                     /* no of designs after parsing */


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

    if (yyin == stdin) {
        fprintf (stderr, "%s: Could not find config file\n", PROJECT);
        return 1;
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
    design_t *tmp;

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
