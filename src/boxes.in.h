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
 * Project-wide globals and data structures
 */

#ifndef BOXES_H
#define BOXES_H

/* #define DEBUG 1 */
/* #define REGEXP_DEBUG 1 */
/* #define PARSER_DEBUG 1 */
/* #define LEXER_DEBUG 1 */

#include <stdio.h>
#include <unitypes.h>
#include "regulex.h"
#include "shape.h"



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


#define BTOP 0                       /* for use with sides */
#define BRIG 1
#define BBOT 2
#define BLEF 3


typedef struct {
    char       *search;
    char       *repstr;
    pcre2_code *prog;                /* compiled search pattern */
    int         line;                /* line of definition in config file */
    char        mode;                /* 'g' or 'o' */
} reprule_t;


typedef struct {
    char      *name;                 /* primary name of the box design */
    char     **aliases;              /* zero-terminated array of alias names of the design */
    char      *author;               /* creator of the configuration file entry */
    char      *designer;             /* creator of the original ASCII artwork */
    char      *created;              /* date created, free format */
    char      *revision;             /* revision number of design */
    char      *revdate;              /* date of current revision */
    char      *sample;               /* the complete sample block in one string */
    char       indentmode;           /* 'b', 't', or 'n' */
    sentry_t   shape[NUM_SHAPES];
    size_t     maxshapeheight;       /* height of highest shape in design */
    size_t     minwidth;
    size_t     minheight;
    int        padding[NUM_SIDES];
    char     **tags;
    char      *defined_in;           /* path to config file where this was defined */

    reprule_t *current_rule;
    reprule_t *reprules;             /* applied when drawing a box */
    size_t     anz_reprules;
    reprule_t *revrules;             /* applied upon removal of a box */
    size_t     anz_revrules;
} design_t;

extern design_t *designs;
extern int num_designs;


typedef struct {                     /* Command line options: */
    char      valign;                /** `-a`: text position inside box */
    char      halign;                /** `-a`: ( h[lcr]v[tcb] )         */
    char      justify;               /** `-a`: 'l', 'c', 'r', or '\0' */
    char     *cld;                   /** `-c`: commandline design definition */
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
    FILE     *infile;
    FILE     *outfile;
} opt_t;

extern opt_t opt;


typedef struct {
    size_t    len;                   /* length of visible text in columns (visible character positions in a text terminal), which is the same as the length of the 'text' field */
    char     *text;                  /* ASCII line content, tabs expanded, ansi escapes removed, multi-byte chars replaced with one or more 'x' */
    size_t    invis;                 /* number of invisble columns/characters (part of an ansi sequence) */

    uint32_t *mbtext;                /* multi-byte (original) line content, tabs expanded. We use UTF-32 in order to enable pointer arithmetic. */
    size_t    num_chars;             /* total number of characters in mbtext, visible + invisible */
    uint32_t *mbtext_org;            /* mbtext as originally allocated, so that we can free it again */

    size_t   *tabpos;                /* tab positions in expanded work strings, or NULL if not needed */
    size_t    tabpos_len;            /* number of tabs in a line */
    size_t   *posmap;                /* for each character in `text`, position of corresponding char in `mbtext`. Needed for box removal. */
} line_t;

typedef struct {
    line_t *lines;
    size_t  num_lines;               /* number of entries in input */
    size_t  maxline;                 /* length of longest input line */
    size_t  indent;                  /* number of leading spaces found */
    int     final_newline;           /* true if the last line of input ends with newline */
} input_t;

extern input_t input;


#endif /* BOXES_H */

/*EOF*/                                  /* vim: set cindent sw=4: */
