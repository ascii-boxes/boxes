/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2024 Thomas Jensen and the boxes contributors
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License, version 3, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/*
 * Processing of command line options.
 */

#include "config.h"
#include <errno.h>
#include <getopt.h>
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
#include "bxstring.h"
#include "discovery.h"
#include "logging.h"
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

#define EXTRA_UNDOC "(undoc)"
#define EXTRA_DEBUG "debug"



/**
 * Print abbreviated usage information on stream `st`.
 * @param st the stream to print to
 */
static void usage_short(FILE *st)
{
    bx_fprintf(st, "Usage: %s [options] [infile [outfile]]\n", PROJECT);
    bx_fprintf(st, "Try `%s -h' for more information.\n", PROJECT);
}



/**
 * Print usage information on stream `st`, including a header text.
 * @param st the stream to print to
 */
void usage_long(FILE *st)
{
    bxstr_t *config_file = discover_config_file(0);

    fprintf(st, "%s - draws any kind of box around your text (or removes it)\n", PROJECT);
    fprintf(st, "        Website: https://boxes.thomasjensen.com/\n");
    fprintf(st, "Usage:  %s [options] [infile [outfile]]\n", PROJECT);
    fprintf(st, "  -a, --align <fmt>     Alignment/positioning of text inside box [default: hlvt]\n");
    fprintf(st, "  -c, --create <str>    Use single shape box design where str is the W shape\n");
    fprintf(st, "      --color           Force output of ANSI sequences if present\n");
    fprintf(st, "      --no-color        Force monochrome output (no ANSI sequences)\n");
    fprintf(st, "  -d, --design <name>   Box design [default: first one in file]\n");
    fprintf(st, "  -e, --eol <eol>       Override line break type (experimental) [default: %s]\n",
                                         strcmp(EOL_DEFAULT, "\r\n") == 0 ? "CRLF" : "LF");
    fprintf(st, "  -f, --config <file>   Configuration file [default: %s]\n",
                                         config_file != NULL ? bxs_to_output(config_file) : "none");
    fprintf(st, "  -h, --help            Print usage information\n");
    fprintf(st, "  -i, --indent <mode>   Indentation mode [default: box]\n");
    fprintf(st, "  -k <bool>             Leading/trailing blank line retention on removal\n");
    fprintf(st, "      --kill-blank      Kill leading/trailing blank lines on removal (like -k true)\n");
    fprintf(st, "      --no-kill-blank   Retain leading/trailing blank lines on removal (like -k false)\n");
    fprintf(st, "  -l, --list            List available box designs w/ samples\n");
    fprintf(st, "  -m, --mend            Mend (repair) box\n");
    fprintf(st, "  -n, --encoding <enc>  Character encoding of input and output [default: %s]\n", locale_charset());
    fprintf(st, "  -p, --padding <fmt>   Padding [default: none]\n");
    fprintf(st, "  -q, --tag-query <qry> Query the list of designs by tag\n");
    fprintf(st, "  -r, --remove          Remove box\n");
    fprintf(st, "  -s, --size <wxh>      Box size (width w and/or height h)\n");
    fprintf(st, "  -t, --tabs <str>      Tab stop distance and expansion [default: %de]\n", DEF_TABSTOP);
    fprintf(st, "  -v, --version         Print version information\n");
    /* fprintf(st, "  -x, --extra <arg>     If <arg> starts with "debug:", activate debug logging for specified log
                areas which follow in a comma-separated list [default area: MAIN]. If <arg> is "(undoc)", trigger
                undocumented behavior of design detail lister.");  // undocumented */

    bxs_free(config_file);
}



static opt_t *create_new_opt()
{
    opt_t *result = (opt_t *) calloc(1, sizeof(opt_t));
    if (result != NULL) {
        /* all values initialized with 0 or NULL */
        result->color = color_from_terminal;
        result->tabstop = DEF_TABSTOP;
        result->eol = "\n";      /* we must default to "\n" instead of EOL_DEFAULT as long as stdout is in text mode */
        result->tabexp = 'e';
        result->killblank = -1;
        result->debug = (int *) calloc(NUM_LOG_AREAS, sizeof(int));
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
    int errfl = 1;
    char *p = optarg;

    while (*p) {
        errfl = 0;
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
        bx_fprintf(stderr, "%s: Illegal text format -- %s\n", PROJECT, optarg);
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
    if (strlen(optarg) == 0) {
        bx_fprintf(stderr, "%s: empty command line design definition\n", PROJECT);
        return 2;
    }
    result->cld = (char *) strdup(optarg);
    if (result->cld == NULL) {
        perror(PROJECT);
        return 1;
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
        bx_fprintf(stderr, "%s: invalid eol spec -- %s\n", PROJECT, optarg);
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
        bx_fprintf(stderr, "%s: invalid indentation mode\n", PROJECT);
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
            bx_fprintf(stderr, "%s: -k: invalid parameter\n", PROJECT);
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
    int errfl = 1;
    char *p = optarg;

    while (*p) {
        errfl = 0;
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
        bx_fprintf(stderr, "%s: invalid padding specification - %s\n", PROJECT, optarg);
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
        bx_fprintf(stderr, "%s: invalid box size specification -- %s\n", PROJECT, optarg);
        return 1;
    }
    return 0;
}



static int debug_areas(opt_t *result, char *optarg)
{
    char *dup = NULL;
    if (optarg != NULL) {
        dup = strdup(optarg);   /* required because strtok() modifies its input */
    }
    else {
        dup = strdup(log_area_names[MAIN]);
    }

    for (char *a = strtok(dup, ","); a != NULL; a = strtok(NULL, ","))
    {
        char *trimmed = trimdup(a, a + strlen(a) - 1);
        if (strlen(trimmed) == 0) {
            BFREE(trimmed);
            continue;
        }
        int valid = 0;
        for (size_t i = ALL; i < NUM_LOG_AREAS + 2; i++) {
            if (strcasecmp(trimmed, log_area_names[i]) == 0) {
                valid = 1;
                if (i == ALL) {
                    for (size_t i = 0; i < NUM_LOG_AREAS; i++) {
                        result->debug[i] = 1;
                    }
                }
                else {
                    result->debug[i - 2] = 1;
                }
                break;
            }
        }
        if (!valid) {
            bx_fprintf(stderr, "%s: invalid debug area -- %s. Valid values: ", PROJECT, trimmed);
            for (size_t i = 1; i < NUM_LOG_AREAS + 2; i++) {
                bx_fprintf(stderr, "%s%s", log_area_names[i], i < (NUM_LOG_AREAS + 2 - 1) ? ", " : "\n");
            }
            BFREE(trimmed);
            return 1;
        }
        BFREE(trimmed);
    }
    BFREE(dup);
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
        bx_fprintf(stderr, "%s: invalid tab stop distance -- %d\n", PROJECT, width);
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
        bx_fprintf(stderr, "%s: invalid tab handling specification - %s\n", PROJECT, optarg);
        return 1;
    }
    return 0;
}



/**
 * Handle undocumented options (-x).
 * @param result the options struct we are building
 * @param optarg the argument to `-x` on the command line
 * @returns 0 on success, anything else on error
 */
static int undocumented_options(opt_t *result, char *optarg)
{
    char *s = optarg;

    if (strncmp(s, EXTRA_UNDOC, strlen(EXTRA_UNDOC)) == 0) {
        result->qundoc = 1;
        s += strlen(EXTRA_UNDOC);
    }

    if (strncasecmp(s, EXTRA_DEBUG, strlen(EXTRA_DEBUG)) == 0) {
        s += strlen(EXTRA_DEBUG);
        if (*s == ':') {
            if (debug_areas(result, s + 1) != 0) {
                return 1;
            }
        }
        else if (*s == '\0') {
            /* default to MAIN */
            debug_areas(result, NULL);
        }
        else {
            bx_fprintf(stderr, "%s: invalid option -x %s\n", PROJECT, optarg);
            return 2;
        }
        activate_debug_logging(result->debug);
    }

    if (!result->qundoc && !is_debug_activated()) {
        bx_fprintf(stderr, "%s: invalid option -x %s\n", PROJECT, optarg);
        return 3;
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
        bx_fprintf(stderr, "%s: illegal parameter -- %s\n", PROJECT, argv[optind + 2]);
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
                bx_fprintf(stderr, "%s: Can\'t open input file -- %s\n", PROJECT, argv[optind]);
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
    if (result != NULL && is_debug_logging(MAIN)) {
        log_debug(__FILE__, MAIN, "Command line option settings (excerpt):\n");
        log_debug(__FILE__, MAIN, "  - Alignment (-a): horiz %c, vert %c\n",
                result->halign ? result->halign : '?', result->valign ? result->valign : '?');
        log_debug(__FILE__, MAIN, "  - Line justification (-a): \'%c\'\n", result->justify ? result->justify : '?');
        log_debug(__FILE__, MAIN, "  - Design Definition W shape (-c): %s\n", result->cld ? result->cld : "n/a");
        log_debug(__FILE__, MAIN, "  - Color mode: %d\n", result->color);

        log_debug(__FILE__, MAIN, "  - Debug areas (-x debug:...): ");
        int dbgfirst = 1;
        for (size_t i = 0; i < NUM_LOG_AREAS; i++) {
            if (result->debug[i]) {
                log_debug_cont(MAIN, "%s%s", dbgfirst ? "" : ", ", log_area_names[i + 2]);
                dbgfirst = 0;
            }
        }
        log_debug_cont(MAIN, "%s\n", dbgfirst ? "(off)" : "");

        log_debug(__FILE__, MAIN, "  - Line terminator used (-e): %s\n",
            strcmp(result->eol, "\r\n") == 0 ? "CRLF" : (strcmp(result->eol, "\r") == 0 ? "CR" : "LF"));
        log_debug(__FILE__, MAIN, "  - Explicit config file (-f): %s\n", result->f ? result->f : "no");
        log_debug(__FILE__, MAIN, "  - Indentmode (-i): \'%c\'\n", result->indentmode ? result->indentmode : '?');
        log_debug(__FILE__, MAIN, "  - Kill blank lines (-k): %d\n", result->killblank);
        log_debug(__FILE__, MAIN, "  - Mend box (-m): %d\n", result->mend);
        log_debug(__FILE__, MAIN, "  - Padding (-p): l:%d t:%d r:%d b:%d\n",
                result->padding[BLEF], result->padding[BTOP], result->padding[BRIG], result->padding[BBOT]);

        log_debug(__FILE__, MAIN, "  - Tag Query (-q): ");
        if (result->query != NULL) {
            for (size_t qidx = 0; result->query[qidx] != NULL; ++qidx) {
                log_debug_cont(MAIN, "%s%s", qidx > 0 ? ", " : "", result->query[qidx]);
            }
        } else {
            log_debug_cont(MAIN, "(none)");
        }
        log_debug_cont(MAIN, "\n");

        log_debug(__FILE__, MAIN, "  - qundoc (-x): %d\n", result->qundoc);
        log_debug(__FILE__, MAIN, "  - Remove box (-r): %d\n", result->r);
        log_debug(__FILE__, MAIN, "  - Requested box size (-s): %ldx%ld\n", result->reqwidth, result->reqheight);
        log_debug(__FILE__, MAIN, "  - Tabstop distance (-t): %d\n", result->tabstop);
        log_debug(__FILE__, MAIN, "  - Tab handling (-t): \'%c\'\n", result->tabexp);
    }
}



opt_t *process_commandline(int argc, char *argv[])
{
    if (is_debug_logging(MAIN)) {
        log_debug(__FILE__, MAIN, "argc = %d\n", argc);
        log_debug(__FILE__, MAIN, "argv = [");
        for(int i=0; i<=argc; i++) {
            log_debug_cont(MAIN, "%s%s", argv[i], i < argc ? ", " : "]\n");
        }
    }

    opt_t *result = create_new_opt();

    /* Intercept '-?' case first, as it is not supported by getopt_long() */
    if (argc >= 2 && argv[1] != NULL && strcmp(argv[1], "-?") == 0) {
        result->help = 1;
        return result;
    }

    optind = 1;   /* ensure that getopt() will process all arguments, even in unit test situations */
    int option_index = 0;
    const struct option long_options[] = {
        { "align",         required_argument, NULL, 'a' },
        { "create",        required_argument, NULL, 'c' },
        { "color",         no_argument,       NULL, OPT_COLOR },
        { "no-color",      no_argument,       NULL, OPT_NO_COLOR },
        { "design",        required_argument, NULL, 'd' },
        { "eol",           required_argument, NULL, 'e' },
        { "config",        required_argument, NULL, 'f' },
        { "help",          no_argument,       NULL, 'h' },
        { "indent",        required_argument, NULL, 'i' },
        { "kill-blank",    no_argument,       NULL, OPT_KILLBLANK },
        { "no-kill-blank", no_argument,       NULL, OPT_NO_KILLBLANK },
        { "list",          no_argument,       NULL, 'l' },
        { "mend",          no_argument,       NULL, 'm' },
        { "encoding",      required_argument, NULL, 'n' },
        { "padding",       required_argument, NULL, 'p' },
        { "tag-query",     required_argument, NULL, 'q' },
        { "remove",        no_argument,       NULL, 'r' },
        { "size",          required_argument, NULL, 's' },
        { "tabs",          required_argument, NULL, 't' },
        { "version",       no_argument,       NULL, 'v' },
        { "extra",         required_argument, NULL, 'x' },
        { NULL,            0,                 NULL,  0  }
    };
    const char *short_options = "a:c:d:e:f:hi:k:lmn:p:q:rs:t:vx:";

    int oc;   /* option character */
    do {
        oc = getopt_long(argc, argv, short_options, long_options, &option_index);

        switch (oc) {

            case 'a':
                if (alignment(result, optarg) != 0) {
                    BFREE(result);
                    return NULL;
                }
                break;

            case 'c':
                if (command_line_design(result, optarg) != 0) {
                    BFREE(result);
                    return NULL;
                }
                break;
            
            case OPT_COLOR:
                result->color = force_ansi_color;
                break;

            case OPT_NO_COLOR:
                result->color = force_monochrome;
                break;

            case 'd':
                if (design_choice(result, optarg) != 0) {
                    BFREE(result);
                    return NULL;
                }
                break;

            case 'e':
                if (eol_override(result, optarg) != 0) {
                    BFREE(result);
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
                    BFREE(result);
                    return NULL;
                }
                break;

            case 'k':
                if (killblank(result, optarg) != 0) {
                    BFREE(result);
                    return NULL;
                }
                break;
            
            case OPT_KILLBLANK:
                if (result->killblank == -1) {
                    result->killblank = 1;
                }
                break;

            case OPT_NO_KILLBLANK:
                if (result->killblank == -1) {
                    result->killblank = 0;
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
                    BFREE(result);
                    return NULL;
                }
                break;

            case 'p':
                if (padding(result, optarg) != 0) {
                    BFREE(result);
                    return NULL;
                }
                break;

            case 'q':
                if (query(result, optarg) != 0) {
                    BFREE(result);
                    return NULL;
                }
                break;

            case 'r':
                result->r = 1;   /* remove box */
                break;

            case 's':
                if (size_of_box(result, optarg) != 0) {
                    BFREE(result);
                    return NULL;
                }
                break;

            case 't':
                if (tab_handling(result, optarg) != 0) {
                    BFREE(result);
                    return NULL;
                }
                break;

            case 'v':
                result->version_requested = 1;   /* print version number */
                return result;

            case 'x':
                if (undocumented_options(result, optarg) != 0) {
                    BFREE(result);
                    return NULL;
                }
                break;

            case ':':
            case '?':
                /* Missing argument or illegal option - do nothing else */
                usage_short(stderr);
                BFREE(result);
                return NULL;

            case EOF:
                /* End of list, do nothing more */
                break;

            default:
                bx_fprintf(stderr, "%s: internal error\n", PROJECT);
                BFREE(result);
                return NULL;
        }
    } while (oc != EOF);

    if (input_output_files(result, argv, optind) != 0) {
        BFREE(result);
        return NULL;
    }

    print_debug_info(result);
    return result;
}


/* vim: set sw=4: */
