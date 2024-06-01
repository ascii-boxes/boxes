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
 * Project-wide globals and data structures
 */

#ifndef BOXES_H
#define BOXES_H

#include <stdio.h>
#include <unitypes.h>

#include "bxstring.h"
#include "regulex.h"



#define PROJECT "boxes"                  /* name of program */
#define VERSION "--BVERSION--"           /* current release of project */
#define GLOBALCONF "--GLOBALCONF--"      /* name of system-wide config file */


#define DEF_INDENTMODE 'b'   /* default indent mode of a design (indent box, not text) */


/*
 *  max. supported line length in bytes
 *  This is how many characters of a line will be read. Anything beyond
 *  will be discarded. The line feed character at the end does not count.
 *  (This should have been done via sysconf(), but I didn't do it in order
 *  to ease porting to non-unix platforms.)
 */
#if defined(LINE_MAX_BYTES) && (LINE_MAX_BYTES < 2048)
#undef LINE_MAX_BYTES
#endif
#ifndef LINE_MAX_BYTES
#define LINE_MAX_BYTES        16382
#endif

/* Macro to declare a function parameter as intentionally unused in order to avoid compiler warnings */
#define UNUSED(variable) ((void)(variable))


#define BTOP 0                       /* for use with sides */
#define BRIG 1
#define BBOT 2
#define BLEF 3


typedef enum {
    NW, NNW, N, NNE, NE, ENE, E, ESE, SE, SSE, S, SSW, SW, WSW, W, WNW
} shape_t;


typedef struct {
    shape_t   name;
    char    **chars;
    bxstr_t **mbcs;
    size_t    height;
    size_t    width;

    /** elastic is used only in original definition */
    int       elastic;

    /** For each shape line 0..height-1, a flag which is 1 if all shapes to the left of this shape are blank on the
     *  same shape line. Always 1 if the shape is part of the left (west) box side. */
    int      *blank_leftward;

    /** For each shape line 0..height-1, a flag which is 1 if all shapes to the right of this shape are blank on the
     *  same shape line. Always 1 if the shape is part of the right (east) box side. */
    int      *blank_rightward;
} sentry_t;

#define SENTRY_INITIALIZER (sentry_t) {NW, NULL, NULL, 0, 0, 0, NULL, NULL}

#define NUM_SHAPES 16
#define NUM_SIDES   4
#define NUM_CORNERS 4


typedef struct {
    bxstr_t    *search;
    bxstr_t    *repstr;
    pcre2_code *prog;                /* compiled search pattern */
    int         line;                /* line of definition in config file */
    char        mode;                /* 'g' or 'o' */
} reprule_t;


typedef struct {
    char      *name;                 /* primary name of the box design */
    char     **aliases;              /* zero-terminated array of alias names of the design */
    bxstr_t   *author;               /* creator of the configuration file entry */
    bxstr_t   *designer;             /* creator of the original ASCII artwork */
    bxstr_t   *sample;               /* the complete sample block in one string */
    char       indentmode;           /* 'b', 't', or 'n' */
    sentry_t   shape[NUM_SHAPES];
    size_t     maxshapeheight;       /* height of highest shape in design */
    size_t     minwidth;
    size_t     minheight;
    int        padding[NUM_SIDES];
    char     **tags;
    bxstr_t   *defined_in;           /* path to config file where this was defined */

    reprule_t *current_rule;
    reprule_t *reprules;             /* applied when drawing a box */
    size_t     num_reprules;
    reprule_t *revrules;             /* applied upon removal of a box */
    size_t     num_revrules;
} design_t;

extern design_t *designs;
extern int num_designs;


typedef struct {                     /* Command line options: */
    char      valign;                /** `-a`: text position inside box */
    char      halign;                /** `-a`: ( h[lcr]v[tcb] )         */
    char      justify;               /** `-a`: 'l', 'c', 'r', or '\0' */
    char     *cld;                   /** `-c`: commandline design definition */
    int       color;                 /** `--color` or `--no-color`: `force_monochrome`, `color_from_terminal`, or `force_ansi_color` */
    design_t *design;                /** `-d`: currently used box design */
    int       design_choice_by_user; /** `-d`, `-c`: true if design was chosen by user */
    char     *eol;                   /** `-e`: line break to use. Never NULL, default to "\n". */
    int       eol_overridden;        /** `-e`: 0: value in `eol` is the default; 1: value in `eol` specified via `-e` */
    char     *f;                     /** `-f`: config file path */
    int       help;                  /** `-h`: flags if help argument was specified */
    char      indentmode;            /** `-i`: 'b', 't', 'n', or '\0' */
    int       killblank;             /** `-k`: kill blank lines, -1 if not set */
    int       l;                     /** `-l`: list available designs */
    int       mend;                  /** `-m`: 1 if -m is given, 2 in 2nd loop */
    char     *encoding;              /** `-n`: character encoding override for input and output text */
    int       padding[NUM_SIDES];    /** `-p`: in spaces or lines resp. */
    char    **query;                 /** `-q`: parsed tag query expression passed in via -q; also, special handling of web UI needs */
    int       r;                     /** `-r`: remove box from input */
    long      reqwidth;              /** `-s`: requested box width */
    long      reqheight;             /** `-s`: requested box height */
    int       tabstop;               /** `-t`: tab stop distance */
    char      tabexp;                /** `-t`: tab expansion mode (for leading tabs) */
    int       version_requested;     /** `-v`: request to show version number */
    int      *debug;                 /** `-x debug:`: activate debug logging for given debug log areas */
    int       qundoc;                /** `-x (undoc)`: flag if "(undoc)" was specified, put directly before "debug:" */
    FILE     *infile;
    FILE     *outfile;
} opt_t;

extern opt_t opt;

/* The possible values of the `color` field from `opt_t`: */
#define force_monochrome 0
#define color_from_terminal 1
#define force_ansi_color 2

/** Flag indicating if ANSI color codes should be printed (1) or not (0) */
extern int color_output_enabled;


typedef struct {
    bxstr_t  *text;                  /* text content of the line as a boxes string */
    uint32_t *cache_visible;         /* only the visible characters of `text`, initially NULL. This is a cache. */
    size_t   *tabpos;                /* tab positions in expanded work strings, or NULL if not needed */
    size_t    tabpos_len;            /* number of tabs in a line */
} line_t;

typedef struct {
    line_t *lines;
    size_t  num_lines;               /* number of entries in input */
    size_t  maxline;                 /* length in columns of longest input line */
    size_t  indent;                  /* common number of leading spaces found in all input lines */
    int     final_newline;           /* true if the last line of input ends with newline */
} input_t;

extern input_t input;


#endif /* BOXES_H */

/* vim: set cindent sw=4: */
