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
 * Processing of command line options.
 */

#include "config.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <uniconv.h>
#include <unistd.h>

#ifdef __MINGW32__
#include <fcntl.h>  /* _O_BINARY */
#include <io.h>     /* _setmode() */
#endif

#include "boxes.h"
#include "discovery.h"
#include "query.h"
#include "tools.h"
#include "cmdline.h"


extern char *optarg;   /* for getopt() */
extern int optind;     /* for getopt() */


/* default tab stop distance (part of -t) */
#define DEF_TABSTOP 8

/* max. allowed tab stop distance */
#define MAX_TABSTOP 16


/* System default line terminator.
 * Used only for display in usage info. The real default is always "\n", with stdout in text mode. */
#ifdef __MINGW32__
    #define EOL_DEFAULT "\r\n"
#else
    #define EOL_DEFAULT "\n"
#endif



/**
 * Print abbreviated usage information on stream `st`.
 * @param st the stream to print to
 */
static void usage_short(FILE *st)
{
    fprintf(st, "Usage: %s [options] [infile [outfile]]\n", PROJECT);
    fprintf(st, "Try `%s -h' for more information.\n", PROJECT);
}



/**
 * Print usage information on stream `st`, including a header text.
 * @param st the stream to print to
 */
void usage_long(FILE *st)
{
    char *config_file = discover_config_file(0);

    fprintf(st, "%s - draws any kind of box around your text (or removes it)\n", PROJECT);
    fprintf(st, "        Website: https://boxes.thomasjensen.com/\n");
    fprintf(st, "Usage:  %s [options] [infile [outfile]]\n", PROJECT);
    fprintf(st, "        -a fmt   alignment/positioning of text inside box [default: hlvt]\n");
    fprintf(st, "        -c str   use single shape box design where str is the W shape\n");
    fprintf(st, "        -d name  box design [default: first one in file]\n");
    fprintf(st, "        -e eol   Override line break type (experimental) [default: %s]\n",
                                  strcmp(EOL_DEFAULT, "\r\n") == 0 ? "CRLF" : "LF");
    fprintf(st, "        -f file  configuration file [default: %s]\n", config_file != NULL ? config_file : "none");
    fprintf(st, "        -h       print usage information\n");
    fprintf(st, "        -i mode  indentation mode [default: box]\n");
    fprintf(st, "        -k bool  leading/trailing blank line retention on removal\n");
    fprintf(st, "        -l       list available box designs w/ samples\n");
    fprintf(st, "        -m       mend box, i.e. remove it and redraw it afterwards\n");
    fprintf(st, "        -n enc   Character encoding of input and output [default: %s]\n", locale_charset());
    fprintf(st, "        -p fmt   padding [default: none]\n");
    fprintf(st, "        -q qry   query the list of designs by tag\n"); /* with "(undoc)" as query, trigger undocumented webui stuff instead */
    fprintf(st, "        -r       remove box\n");
    fprintf(st, "        -s wxh   box size (width w and/or height h)\n");
    fprintf(st, "        -t str   tab stop distance and expansion [default: %de]\n", DEF_TABSTOP);
    fprintf(st, "        -v       print version information\n");

    BFREE(config_file);
}



static opt_t *create_new_opt()
{
    opt_t *result = (opt_t *) calloc(1, sizeof(opt_t));
    if (result != NULL) {
        /* all valued initialized with 0 or NULL */
        result->tabstop = DEF_TABSTOP;
        result->eol = "\n";      /* we must default to "\n" instead of EOL_DEFAULT as long as stdout is in text mode */
        result->tabexp = 'e';
        result->killblank = -1;
        for (int i = 0; i < NUM_SIDES; ++i) {
            result->padding[i] = -1;
        }
    }
    else {
        perror(PROJECT);
    }
    return result;
}



/**
 * Alignment/positioning of text inside box.
 * @param result the options struct we are building
 * @param optarg the argument to `-a` on the command line
 * @returns 0 on success, anything else on error
 */
static int alignment(opt_t *result, char *optarg)
{
    int errfl = 0;
    char *p = optarg;

    while (*p) {
        if (p[1] == '\0' && !strchr("lLcCrR", *p)) {
            errfl = 1;
            break;
        }

        switch (*p) {
            case 'h': case 'H':
                switch (p[1]) {
                    case 'c': case 'C': result->halign = 'c'; break;
                    case 'l': case 'L': result->halign = 'l'; break;
                    case 'r': case 'R': result->halign = 'r'; break;
                    default:            errfl = 1;            break;
                }
                ++p;
                break;

            case 'v': case 'V':
                switch (p[1]) {
                    case 'c': case 'C': result->valign = 'c'; break;
                    case 't': case 'T': result->valign = 't'; break;
                    case 'b': case 'B': result->valign = 'b'; break;
                    default:            errfl = 1;            break;
                }
                ++p;
                break;

            case 'j': case 'J':
                switch (p[1]) {
                    case 'l': case 'L': result->justify = 'l'; break;
                    case 'c': case 'C': result->justify = 'c'; break;
                    case 'r': case 'R': result->justify = 'r'; break;
                    default:            errfl = 1;             break;
                }
                ++p;
                break;

            case 'l': case 'L':
                result->justify = 'l';
                result->halign = 'l';
                result->valign = 'c';
                break;

            case 'r': case 'R':
                result->justify = 'r';
                result->halign = 'r';
                result->valign = 'c';
                break;

            case 'c': case 'C':
                result->justify = 'c';
                result->halign = 'c';
                result->valign = 'c';
                break;

            default:
                errfl = 1;
                break;
        }

        if (errfl) {
            break;
        } else {
            ++p;
        }
    }

    if (errfl) {
        fprintf(stderr, "%s: Illegal text format -- %s\n", PROJECT, optarg);
        return 1;
    }
    return 0;
}



/**
 * Command line design definition.
 * @param result the options struct we are building
 * @param optarg the argument to `-c` on the command line
 * @returns 0 on success, anything else on error
 */
static int command_line_design(opt_t *result, char *optarg)
{
    result->cld = (char *) strdup(optarg);
    if (result->cld == NULL) {
        perror(PROJECT);
        return 1;
    }
    else {
        line_t templine = {0};
        templine.len = strlen(result->cld);
        templine.text = result->cld;
        if (empty_line(&templine)) {
            fprintf(stderr, "%s: boxes may not consist entirely of whitespace\n", PROJECT);
            return 1;
        }
    }
    result->design_choice_by_user = 1;
    return 0;
}



/**
 * Box design selection.
 * @param result the options struct we are building
 * @param optarg the argument to `-d` on the command line
 * @returns 0 on success, anything else on error
 */
static int design_choice(opt_t *result, char *optarg)
{
    BFREE (result->design);
    result->design = (design_t *) ((char *) strdup(optarg));
    if (result->design == NULL) {
        perror(PROJECT);
        return 1;
    }
    result->design_choice_by_user = 1;
    return 0;
}



/**
 * EOL Override.
 * @param result the options struct we are building
 * @param optarg the argument to `-e` on the command line
 * @returns 0 on success, anything else on error
 */
static int eol_override(opt_t *result, char *optarg)
{
    result->eol_overridden = 1;
    if (strcasecmp(optarg, "CRLF") == 0) {
        result->eol = "\r\n";
    }
    else if (strcasecmp(optarg, "LF") == 0) {
        result->eol = "\n";
    }
    else if (strcasecmp(optarg, "CR") == 0) {
        result->eol = "\r";
    }
    else {
        fprintf(stderr, "%s: invalid eol spec -- %s\n", PROJECT, optarg);
        return 1;
    }
    return 0;
}



/**
 * Indentation mode.
 * @param result the options struct we are building
 * @param optarg the argument to `-i` on the command line
 * @returns 0 on success, anything else on error
 */
static int indentation_mode(opt_t *result, char *optarg)
{
    size_t optlen = strlen(optarg);
    if (optlen <= 3 && !strncasecmp("box", optarg, optlen)) {
        result->indentmode = 'b';
    }
    else if (optlen <= 4 && !strncasecmp("text", optarg, optlen)) {
        result->indentmode = 't';
    }
    else if (optlen <= 4 && !strncasecmp("none", optarg, optlen)) {
        result->indentmode = 'n';
    }
    else {
        fprintf(stderr, "%s: invalid indentation mode\n", PROJECT);
        return 1;
    }
    return 0;
}



/**
 * Kill blank lines or not [default: design-dependent].
 * @param result the options struct we are building
 * @param optarg the argument to `-k` on the command line
 * @returns 0 on success, anything else on error
 */
static int killblank(opt_t *result, char *optarg)
{
    if (result->killblank == -1) {
        if (strisyes(optarg)) {
            result->killblank = 1;
        }
        else if (strisno(optarg)) {
            result->killblank = 0;
        }
        else {
            fprintf(stderr, "%s: -k: invalid parameter\n", PROJECT);
            return 1;
        }
    }
    return 0;
}



/**
 * Padding. Format is `([ahvtrbl]n)+`.
 * @param result the options struct we are building
 * @param optarg the argument to `-p` on the command line
 * @returns 0 on success, anything else on error
 */
static int padding(opt_t *result, char *optarg)
{
    int errfl = 0;
    char *p = optarg;

    while (*p) {
        if (p[1] == '\0') {
            errfl = 1;
            break;
        }
        char c = *p;
        errno = 0;
        int size = (int) strtol(p + 1, &p, 10);
        if (errno || size < 0) {
            errfl = 1;
            break;
        }
        switch (c) {
            case 'a': case 'A':
                result->padding[BTOP] = size;
                result->padding[BBOT] = size;
                result->padding[BLEF] = size;
                result->padding[BRIG] = size;
                break;
            case 'h': case 'H':
                result->padding[BLEF] = size;
                result->padding[BRIG] = size;
                break;
            case 'v': case 'V':
                result->padding[BTOP] = size;
                result->padding[BBOT] = size;
                break;
            case 't': case 'T':
                result->padding[BTOP] = size;
                break;
            case 'l': case 'L':
                result->padding[BLEF] = size;
                break;
            case 'b': case 'B':
                result->padding[BBOT] = size;
                break;
            case 'r': case 'R':
                result->padding[BRIG] = size;
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
    return 0;
}



/**
 * Parse the tag query specified with `-q`.
 * @param result the options struct we are building
 * @param optarg the argument to `-q` on the command line
 * @returns 0 on success, anything else on error
 */
static int query(opt_t *result, char *optarg)
{
    char **query = parse_query(optarg);
    result->query = query;
    return query != NULL ? 0 : 1;
}



/**
 * Specify desired box target size.
 * @param result the options struct we are building
 * @param optarg the argument to `-s` on the command line
 * @returns 0 on success, anything else on error
 */
static int size_of_box(opt_t *result, char *optarg)
{
    char *p = strchr(optarg, 'x');
    if (!p) {
        p = strchr(optarg, 'X');
    }
    if (p) {
        *p = '\0';
    }
    errno = 0;
    if (optarg != p) {
        result->reqwidth = strtol(optarg, NULL, 10);
    }
    if (p) {
        result->reqheight = strtol(p + 1, NULL, 10);
        *p = 'x';
    }
    if (errno || (result->reqwidth == 0 && result->reqheight == 0) || result->reqwidth < 0 || result->reqheight < 0) {
        fprintf(stderr, "%s: invalid box size specification -- %s\n", PROJECT, optarg);
        return 1;
    }
    return 0;
}



/**
 * Tab handling. Format is `n[eku]`.
 * @param result the options struct we are building
 * @param optarg the argument to `-t` on the command line
 * @returns 0 on success, anything else on error
 */
static int tab_handling(opt_t *result, char *optarg)
{
    char *p;
    int width = (int) strtol(optarg, &p, 10);
    if (width < 1 || width > MAX_TABSTOP) {
        fprintf(stderr, "%s: invalid tab stop distance -- %d\n", PROJECT, width);
        return 1;
    }
    result->tabstop = width;

    int errfl = 0;
    if (*p != '\0') {
        if (p[1] != '\0') {
            errfl = 1;
        }
        else {
            switch (*p) {
                case 'e': case 'E':
                    result->tabexp = 'e';
                    break;
                case 'k': case 'K':
                    result->tabexp = 'k';
                    break;
                case 'u': case 'U':
                    result->tabexp = 'u';
                    break;
                default:
                    errfl = 1;
                    break;
            }
        }
    }
    if (errfl) {
        fprintf(stderr, "%s: invalid tab handling specification - %s\n", PROJECT, optarg);
        return 1;
    }
    return 0;
}



/**
 * Set *stdout* to binary mode, so that we can control the line terminator. This function only ever does anything on
 * Windows, because on Linux, we already do have control over line terminators.
 * @param result the options struct we are building
 * @return the *stdout* stream, reconfigured to binary if necessary
 */
static FILE *get_stdout_configured(opt_t *result)
{
    if (result->eol_overridden) {
        #ifdef __MINGW32__
            int rc = _setmode(fileno(stdout), _O_BINARY);
            if (rc == -1) {
                perror(PROJECT);
            }
        #endif
    }
    return stdout;
}



/**
 * Input and Output Files. After any command line options, an input file and an output file may be specified (in that
 * order). "-" may be substituted for standard input or output. A third file name would be invalid.
 * @param result the options struct we are building
 * @param argv the original command line options as specified
 * @param optind the index of the next element to be processed in `argv`
 * @returns 0 on success, anything else on error
 */
static int input_output_files(opt_t *result, char *argv[], int optind)
{
    if (argv[optind] == NULL) {                              /* neither infile nor outfile given */
        result->infile = stdin;
        result->outfile = get_stdout_configured(result);
    }

    else if (argv[optind + 1] && argv[optind + 2]) {         /* illegal third file */
        fprintf(stderr, "%s: illegal parameter -- %s\n", PROJECT, argv[optind + 2]);
        usage_short(stderr);
        return 1;
    }

    else {
        if (strcmp(argv[optind], "-") == 0) {
            result->infile = stdin;                          /* use stdin for input */
        }
        else {
            result->infile = fopen(argv[optind], "r");
            if (result->infile == NULL) {
                fprintf(stderr, "%s: Can\'t open input file -- %s\n", PROJECT, argv[optind]);
                return 9;                                    /* can't read infile */
            }
        }

        if (argv[optind + 1] == NULL) {
            result->outfile = get_stdout_configured(result); /* no outfile given */
        }
        else if (strcmp(argv[optind + 1], "-") == 0) {
            result->outfile = get_stdout_configured(result); /* use stdout for output */
        }
        else {
            result->outfile = fopen(argv[optind + 1], "wb");
            if (result->outfile == NULL) {
                perror(PROJECT);
                if (result->infile != stdin) {
                    fclose(result->infile);
                }
                return 10;
            }
        }
    }
    return 0;
}



static void print_debug_info(opt_t *result)
{
    if (result != NULL) {
        #if defined(DEBUG)
            fprintf (stderr, "Command line option settings (excerpt):\n");
            fprintf (stderr, "- Alignment (-a): horiz %c, vert %c\n",
                    result->halign ? result->halign : '?', result->valign ? result->valign : '?');
            fprintf (stderr, "- Line justification (-a): \'%c\'\n", result->justify ? result->justify : '?');
            fprintf (stderr, "- Design Definition W shape (-c): %s\n", result->cld ? result->cld : "n/a");
            fprintf (stderr, "- Line terminator used (-e): %s\n",
                strcmp(result->eol, "\r\n") == 0 ? "CRLF" : (strcmp(result->eol, "\r") == 0 ? "CR" : "LF"));
            fprintf (stderr, "- Explicit config file (-f): %s\n", result->f ? result->f : "no");
            fprintf (stderr, "- Indentmode (-i): \'%c\'\n", result->indentmode ? result->indentmode : '?');
            fprintf (stderr, "- Kill blank lines (-k): %d\n", result->killblank);
            fprintf (stderr, "- Mend box (-m): %d\n", result->mend);
            fprintf (stderr, "- Padding (-p): l:%d t:%d r:%d b:%d\n",
                    result->padding[BLEF], result->padding[BTOP], result->padding[BRIG], result->padding[BBOT]);
            fprintf (stderr, "- Tag Query / Special handling for Web UI (-q): ");
            if (result->query != NULL) {
                for (size_t qidx = 0; result->query[qidx] != NULL; ++qidx) {
                    fprintf(stderr, "%s%s", qidx > 0 ? ", " : "", result->query[qidx]);
                }
            } else {
                fprintf (stderr, "(none)");
            }
            fprintf (stderr, "\n");
            fprintf (stderr, "- Remove box (-r): %d\n", result->r);
            fprintf (stderr, "- Requested box size (-s): %ldx%ld\n", result->reqwidth, result->reqheight);
            fprintf (stderr, "- Tabstop distance (-t): %d\n", result->tabstop);
            fprintf (stderr, "- Tab handling (-t): \'%c\'\n", result->tabexp);
        #endif
    }
}



opt_t *process_commandline(int argc, char *argv[])
{
    opt_t *result = create_new_opt();

    /* Intercept '--help' and '-?' cases first, as they are not supported by getopt() */
    if (argc >= 2 && argv[1] != NULL && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-?") == 0)) {
        result->help = 1;
        return result;
    }

    int oc;   /* option character */
    do {
        oc = getopt(argc, argv, "a:c:d:e:f:hi:k:lmn:p:q:rs:t:v");

        switch (oc) {

            case 'a':
                if (alignment(result, optarg) != 0) {
                    return NULL;
                }
                break;

            case 'c':
                if (command_line_design(result, optarg) != 0) {
                    return NULL;
                }
                break;

            case 'd':
                if (design_choice(result, optarg) != 0) {
                    return NULL;
                }
                break;

            case 'e':
                if (eol_override(result, optarg) != 0) {
                    return NULL;
                }
                break;

            case 'f':
                result->f = strdup(optarg);   /* input file */
                break;

            case 'h':
                result->help = 1;
                return result;

            case 'i':
                if (indentation_mode(result, optarg) != 0) {
                    return NULL;
                }
                break;

            case 'k':
                if (killblank(result, optarg) != 0) {
                    return NULL;
                }
                break;

            case 'l':
                result->l = 1;   /* list available box styles */
                break;

            case 'm':
                result->mend = 2;   /*  Mend box: remove, then redraw */
                result->r = 1;
                result->killblank = 0;
                break;

            case 'n':
                result->encoding = strdup(optarg);   /* character encoding */
                if (result->encoding == NULL) {
                    perror(PROJECT);
                    return NULL;
                }
                break;

            case 'p':
                if (padding(result, optarg) != 0) {
                    return NULL;
                }
                break;

            case 'q':
                if (query(result, optarg) != 0) {
                    return NULL;
                }
                break;

            case 'r':
                result->r = 1;   /* remove box */
                break;

            case 's':
                if (size_of_box(result, optarg) != 0) {
                    return NULL;
                }
                break;

            case 't':
                if (tab_handling(result, optarg) != 0) {
                    return NULL;
                }
                break;

            case 'v':
                result->version_requested = 1;   /* print version number */
                return result;

            case ':':
            case '?':
                /* Missing argument or illegal option - do nothing else */
                usage_short(stderr);
                return NULL;

            case EOF:
                /* End of list, do nothing more */
                break;

            default:
                fprintf(stderr, "%s: internal error\n", PROJECT);
                return NULL;
        }
    } while (oc != EOF);

    if (input_output_files(result, argv, optind) != 0) {
        return NULL;
    }

    print_debug_info(result);
    return result;
}


/*EOF*/                                                  /* vim: set sw=4: */
