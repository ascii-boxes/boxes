%{
/*
 *  File:             parser.y
 *  Date created:     March 16, 1999 (Tuesday, 17:17h)
 *  Author:           Copyright (C) 1999 Thomas Jensen <boxes@thomasjensen.com>
 *  Version:          $Id: parser.y,v 1.24 2006/07/22 19:30:55 tsjensen Exp tsjensen $
 *  Language:         GNU bison (ANSI C)
 *  World Wide Web:   http://boxes.thomasjensen.com/
 *  Purpose:          Yacc parser for boxes configuration files
 *
 *  Remarks: o This program is free software; you can redistribute it and/or
 *             modify it under the terms of the GNU General Public License as
 *             published by the Free Software Foundation; either version 2 of
 *             the License, or (at your option) any later version.
 *           o This program is distributed in the hope that it will be useful,
 *             but WITHOUT ANY WARRANTY; without even the implied warranty of
 *             MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *             GNU General Public License for more details.
 *           o You should have received a copy of the GNU General Public
 *             License along with this program; if not, write to the Free
 *             Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *             MA 02111-1307  USA
 *
 *  Revision History:
 *
 *    $Log: parser.y,v $
 *    Revision 1.24  2006/07/22 19:30:55  tsjensen
 *    Fix: Renamed yylineno to tjlineno to enable compilation on certain
 *    flexes (reported by Andreas Heiduk)
 *
 *    Revision 1.23  2006/07/12 05:31:25  tsjensen
 *    Updated email and web addresses in comment header
 *
 *    Revision 1.22  1999-08-21 08:53:58-07  tsjensen
 *    Removed check for empty sample block, which is also done in lexer.l
 *
 *    Revision 1.21  1999/08/18 15:38:44  tsjensen
 *    Added new tokens YCHGDEL and YDELWORD
 *    Added code for DELIMITER statements
 *
 *    Revision 1.20  1999/08/16 16:29:25  tsjensen
 *    Changed SAMPLE block syntax to get rid of all the quoting sh*t in SAMPLE
 *    blocks. Samples now look authentic even in the config file. Very simple now.
 *    Added new token YENDSAMPLE in the process.
 *
 *    Revision 1.19  1999/08/14 19:19:25  tsjensen
 *    Added anz_shapespec variable to count non-deepempty user-specified shapes
 *    (must at least be 1)
 *    Moved yylex declaration to lexer.h, where it belongs
 *    Bugfix: bison and yacc seem to differ in the way action code is treated
 *    which is associated with an error token. Since bison calls the action
 *    routine whenever a token is skipped while recovering, added skipping
 *    variable to ensure that only one "skipping to next design" message is
 *    printed. That was not necessary before. %-/
 *    Removed existence check from corner_check() routine (obsolete).
 *    Added code to generate required shapes which were not specified by the user
 *    Clean-up in shape_def rule (use freeshape() and some more)
 *
 *    Revision 1.18  1999/07/23 16:14:17  tsjensen
 *    Added computation of height of highest shape in design (maxshapeheight)
 *    Options -l and -d together now call quickinfo mode -> parse only 1 design
 *
 *    Revision 1.17  1999/07/22 12:27:16  tsjensen
 *    Added GNU GPL disclaimer
 *    Renamed parser.h include to lexer.h (same file)
 *    Added include config.h
 *
 *    Revision 1.16  1999/07/02 11:54:52  tsjensen
 *    Some minor changes to please compiler
 *    Communication of speed mode to lexer
 *    Bugfix: Forgot to check for opt.l before calling YYACCEPT
 *
 *    Revision 1.15  1999/06/30 12:13:47  tsjensen
 *    Now parsing only those designs which will be needed later on
 *    Checks formerly done in boxes.c now done here (no valid designs etc.)
 *
 *    Revision 1.14  1999/06/28 18:32:51  tsjensen
 *    Unified appearance of error messages, which are now all based on yyerror()
 *    Eliminated duplicate code by introducing intermediate rules
 *    New tokens YTO, YWITH, and YRXPFLAG to reduce strcasecmp() usage
 *    New token YUNREC for integration of lexer into error handling
 *    Some code restructuring and rule renaming for better consistency
 *    Added out-of-memory-checks to many strdup()s and such
 *
 *    Revision 1.13  1999/06/28 12:15:47  tsjensen
 *    Added error handling. Now skips to next design on error.
 *    Replaced DEBUG macro with PARSER_DEBUG, which is now activated in boxes.h
 *    Added rule first_rule, which performs init and cleanup formerly done in main()
 *    Introduced symbols YBOX and YEND
 *
 *    Revision 1.12  1999/06/22 12:01:01  tsjensen
 *    Added #undef DEBUG, because DEBUGging is now activated in boxes.h
 *    Added #include tools.h
 *
 *    Revision 1.11  1999/06/20 14:18:51  tsjensen
 *    Adden YPADDING and YNUMBER tokens plus code for padding blocks
 *
 *    Revision 1.10  1999/06/17 19:04:45  tsjensen
 *    Added detection of empty sample blocks (we don't want that)
 *    Added detection of duplicate sample blocks
 *
 *    Revision 1.9  1999/06/14 12:13:01  tsjensen
 *    Added YREVERSE token
 *    Added code for regexp reversion
 *
 *    Revision 1.7  1999/04/09 13:31:54  tsjensen
 *    Added checks for duplicate design names
 *    Added checks for valid design names (no extended ASCII or ctrl chars)
 *    Removed all code related to OFFSET blocks (obsolete)
 *
 *    Revision 1.6  1999/04/04 16:07:53  tsjensen
 *    Enforced use of PROJECT macro
 *    Added "indent" directive to grammar
 *    Added "replace" directive to grammar
 *
 *    Revision 1.5  1999/03/30 13:29:50  tsjensen
 *    Added computation of minimum width/height of each design.
 *
 *    Revision 1.4  1999/03/30 09:37:51  tsjensen
 *    It drew a correct box for the first time!
 *
 *    Revision 1.3  1999/03/24 17:29:12  tsjensen
 *    Added detection of empty shapes ("") which are now cleared (+warning)
 *    Changed rcs string to #ident directive
 *
 *    Revision 1.1  1999/03/18 15:10:06  tsjensen
 *    Initial revision
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shape.h"
#include "boxes.h"
#include "tools.h"
#include "lexer.h"


const char rcsid_parser_y[] =
    "$Id: parser.y,v 1.24 2006/07/22 19:30:55 tsjensen Exp tsjensen $";


static int pflicht = 0;
static int time_for_se_check = 0;
static int anz_shapespec = 0;            /* number of user-specified shapes */

int speeding = 0;                        /* true if we're skipping designs, */
                                         /* but no error                    */
static int skipping = 0;                 /* used to limit "skipping" msgs */



static int check_sizes()
/*
 *  For the author's convenience, it is required that shapes on one side
 *  have equal width (vertical sides) and height (horizontal sides).
 *
 *  RETURNS:  == 0   no problem detected
 *            != 0   on error (prints error message, too)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i, j, k;

    #ifdef PARSER_DEBUG
        fprintf (stderr, "check_sizes()\n");
    #endif

    for (i=0; i<ANZ_SIDES; ++i) {
        if (i == 0 || i == 2) {
            /*
             *  horizontal
             */
            for (j=0; j<SHAPES_PER_SIDE-1; ++j) {
                if (designs[design_idx].shape[sides[i][j]].height == 0)
                    continue;
                for (k=j+1; k<SHAPES_PER_SIDE; ++k) {
                    if (designs[design_idx].shape[sides[i][k]].height == 0)
                        continue;
                    if (designs[design_idx].shape[sides[i][j]].height
                            != designs[design_idx].shape[sides[i][k]].height) {
                        yyerror ("All shapes on horizontal sides must be of "
                                "equal height (%s: %d, %s: %d)\n",
                                shape_name[sides[i][j]],
                                designs[design_idx].shape[sides[i][j]].height,
                                shape_name[sides[i][k]],
                                designs[design_idx].shape[sides[i][k]].height);
                        return 1;
                    }
                }
            }
        }
        else {
            /*
             *  vertical
             */
            for (j=0; j<SHAPES_PER_SIDE-1; ++j) {
                if (designs[design_idx].shape[sides[i][j]].width == 0)
                    continue;
                for (k=j+1; k<SHAPES_PER_SIDE; ++k) {
                    if (designs[design_idx].shape[sides[i][k]].width == 0)
                        continue;
                    if (designs[design_idx].shape[sides[i][j]].width
                            != designs[design_idx].shape[sides[i][k]].width) {
                        yyerror ("All shapes on vertical sides must be of "
                                "equal width (%s: %d, %s: %d)\n",
                                shape_name[sides[i][j]],
                                designs[design_idx].shape[sides[i][j]].width,
                                shape_name[sides[i][k]],
                                designs[design_idx].shape[sides[i][k]].width);
                        return 1;
                    }
                }
            }
        }
    }

    return 0;                            /* all clear */
}



static int corner_check()
/*
 *  Check that no corners are elastic.
 *
 *  RETURNS:  == 0   no problem detected
 *            != 0   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int c;

    #ifdef PARSER_DEBUG
        fprintf (stderr, "corner_check()\n");
    #endif

    for (c=0; c<ANZ_CORNERS; ++c) {
        if (designs[design_idx].shape[corners[c]].elastic) {
            yyerror ("Corners may not be elastic (%s)", shape_name[corners[c]]);
            return 1;
        }
    }

    return 0;                            /* all clear */
}



static shape_t non_existent_elastics()
{
    shape_t i;

    #ifdef PARSER_DEBUG
        fprintf (stderr, "non_existent_elastics()\n");
    #endif

    for (i=0; i<ANZ_SHAPES; ++i) {
        if (designs[design_idx].shape[i].elastic
         && isempty(designs[design_idx].shape+i))
            return i;
    }

    return (shape_t) ANZ_SHAPES;         /* all elastic shapes exist */
}



static int insufficient_elasticity()
{
    int i, j, ef;

    #ifdef PARSER_DEBUG
        fprintf (stderr, "insufficient_elasticity()\n");
    #endif

    for (i=0; i<ANZ_SIDES; ++i) {
        for (j=1,ef=0; j<4; ++j)
            if (designs[design_idx].shape[sides[i][j]].elastic)
                ++ef;
        if (ef != 1 && ef != 2)
            return 1;                    /* error */
    }

    return 0;                            /* all clear */
}



static int adjoining_elastics()
{
    int i, j, ef;

    #ifdef PARSER_DEBUG
        fprintf (stderr, "adjoining_elastics()\n");
    #endif

    for (i=0; i<ANZ_SIDES; ++i) {
        ef = 0;
        for (j=1; j<4; ++j) {
            if (isempty(designs[design_idx].shape+sides[i][j]))
                continue;
            if (designs[design_idx].shape[sides[i][j]].elastic) {
                if (ef)
                    return 1;            /* error detected */
                else
                    ef = 1;
            }
            else {
                ef = 0;
            }
        }
    }

    return 0;                            /* all clear */
}



static int perform_se_check()
/*
 *  (shape-elastic check)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    shape_t s_rc;

    s_rc = non_existent_elastics();
    if (s_rc != ANZ_SHAPES) {
        yyerror ("Shape %s has been specified as elastic but doesn't exist",
                shape_name[s_rc]);
        return 1;
    }

    if (corner_check()) {
        /* Error message printed in check func */
        return 1;
    }

    if (insufficient_elasticity()) {
        yyerror ("There must be exactly one or two elastic shapes per side");
        return 1;
    }

    if (adjoining_elastics()) {
        yyerror ("Two adjoining shapes may not be elastic");
        return 1;
    }

    return 0;
}



static void recover()
/*
 *  Reset parser to neutral state, so a new design can be parsed.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
     pflicht = 0;
     time_for_se_check = 0;
     anz_shapespec = 0;
     chg_strdelims ('\\', '\"');

     /*
      *  Clear current design
      */
     BFREE (designs[design_idx].name);
     BFREE (designs[design_idx].author);
     BFREE (designs[design_idx].created);
     BFREE (designs[design_idx].revision);
     BFREE (designs[design_idx].revdate);
     BFREE (designs[design_idx].sample);
     memset (designs+design_idx, 0, sizeof(design_t));
     designs[design_idx].indentmode = DEF_INDENTMODE;
}



static int design_needed (const char *name, const int design_idx)
/*
 *  Return true if design of name name will be needed later on
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    if (opt.design_choice_by_user) {
        return !strcasecmp (name, (char *) opt.design);
    }
    else {
        if (opt.r || opt.l)
            return 1;
        if (design_idx == 0)
            return 1;
    }

    return 0;
}


%}


%union {
    char *s;
    char c;
    shape_t shape;
    sentry_t sentry;
    int num;
}

%token YSHAPES YELASTIC YPADDING YSAMPLE YENDSAMPLE YBOX YEND YUNREC
%token YREPLACE YREVERSE YTO YWITH YCHGDEL
%token <s> KEYWORD
%token <s> WORD
%token <s> STRING
%token <shape> SHAPE
%token <num> YNUMBER
%token <c> YRXPFLAG
%token <s> YDELWORD

%type <sentry> shape_def
%type <sentry> shape_lines
%type <c> rflag

%start first_rule


%%


first_rule:
    {
        /*
         *  Initialize parser data structures
         */
        designs = (design_t *) calloc (1, sizeof(design_t));
        if (designs == NULL) {
            perror (PROJECT);
            YYABORT;
        }
        designs->indentmode = DEF_INDENTMODE;
    }
config_file
    {
        /*
         *  Clean up parser data structures
         */
        design_t *tmp;

        if (design_idx == 0) {
            BFREE (designs);
            anz_designs = 0;
            if (opt.design_choice_by_user) {
                fprintf (stderr, "%s: unknown box design -- %s\n",
                        PROJECT, (char *) opt.design);
            }
            else {
                yyerror ("no valid designs found");
            }
            YYABORT;
        }

        --design_idx;
        anz_designs = design_idx + 1;
        tmp = (design_t *) realloc (designs, anz_designs*sizeof(design_t));
        if (!tmp) {
            perror (PROJECT);
            YYABORT;
        }
        designs = tmp;
    }
;


config_file: config_file design_or_error | design_or_error ;


design_or_error: design | error
    {
        if (!speeding && !skipping) {
            recover();
            yyerror ("skipping to next design");
            skipping = 1;
        }
    }
;
    

design: YBOX WORD
    {
        chg_strdelims ('\\', '\"');
        skipping = 0;
        if (!design_needed ($2, design_idx)) {
            speeding = 1;
            begin_speedmode();
            YYERROR;
        }
    }
layout YEND WORD
    {
        design_t *tmp;
        int i;
        char *p;

        #ifdef PARSER_DEBUG
            fprintf (stderr, "--------- ADDING DESIGN \"%s\".\n", $2);
        #endif

        if (strcasecmp ($2, $6)) {
            yyerror ("box design name differs at BOX and END");
            YYERROR;
        }
        if (pflicht < 3) {
            yyerror ("entries SAMPLE, SHAPES, and ELASTIC are mandatory");
            YYERROR;
        }
        
        for (i=0; i<design_idx; ++i) {
            if (strcasecmp ($2, designs[i].name) == 0) {
                yyerror ("duplicate box design name -- %s", $2);
                YYERROR;
            }
        }

        p = $2;
        while (*p) {
            if (*p < 32 || *p > 126) {
                yyerror ("box design name must consist of printable standard "
                         "ASCII characters.");
                YYERROR;
            }
            ++p;
        }

        designs[design_idx].name = (char *) strdup ($2);
        if (designs[design_idx].name == NULL) {
            perror (PROJECT);
            YYABORT;
        }
        pflicht = 0;
        time_for_se_check = 0;
        anz_shapespec = 0;

        /*
         *  Check if we need to continue parsing. If not, return.
         *  The condition here must correspond to design_needed().
         */
        if (opt.design_choice_by_user || (!opt.r && !opt.l)) {
            anz_designs = design_idx + 1;
            YYACCEPT;
        }

        /*
         *  Allocate space for next design
         */
        ++design_idx;
        tmp = (design_t *) realloc (designs, (design_idx+1)*sizeof(design_t));
        if (tmp == NULL) {
            perror (PROJECT);
            YYABORT;
        }
        designs = tmp;
        memset (&(designs[design_idx]), 0, sizeof(design_t));
        designs[design_idx].indentmode = DEF_INDENTMODE;
    }
;


layout: layout entry | layout block | entry | block ;


entry: KEYWORD STRING
    {
        #ifdef PARSER_DEBUG
            fprintf (stderr, "entry rule fulfilled [%s = %s]\n", $1, $2);
        #endif
        if (strcasecmp ($1, "author") == 0) {
            designs[design_idx].author = (char *) strdup ($2);
            if (designs[design_idx].author == NULL) {
                perror (PROJECT);
                YYABORT;
            }
        }
        else if (strcasecmp ($1, "revision") == 0) {
            designs[design_idx].revision = (char *) strdup ($2);
            if (designs[design_idx].revision == NULL) {
                perror (PROJECT);
                YYABORT;
            }
        }
        else if (strcasecmp ($1, "created") == 0) {
            designs[design_idx].created = (char *) strdup ($2);
            if (designs[design_idx].created == NULL) {
                perror (PROJECT);
                YYABORT;
            }
        }
        else if (strcasecmp ($1, "revdate") == 0) {
            designs[design_idx].revdate = (char *) strdup ($2);
            if (designs[design_idx].revdate == NULL) {
                perror (PROJECT);
                YYABORT;
            }
        }
        else if (strcasecmp ($1, "indent") == 0) {
            if (strcasecmp ($2, "text") == 0 ||
                strcasecmp ($2, "box") == 0 ||
                strcasecmp ($2, "none") == 0) {
                designs[design_idx].indentmode = $2[0];
            }
            else {
                yyerror ("indent keyword must be followed by \"text\", "
                         "\"box\", or \"none\"");
                YYERROR;
            }
        }
        else {
            yyerror ("internal parser error (unrecognized: %s) in line %d "
                    "of %s.", $1, __LINE__, __FILE__);
            YYERROR;
        }
    }

| YCHGDEL YDELWORD
    {
        if (strlen($2) != 2) {
            yyerror ("invalid string delimiter specification -- %s", $2);
            YYERROR;
        }
        if (($2)[0] == ($2)[1]) {
            yyerror ("string delimiter and escape char may not be the same");
            YYERROR;
        }
        if (strchr (LEX_SDELIM, ($2)[1]) == NULL) {
            yyerror ("invalid string delimiter -- %c (try one of %s)",
                    ($2)[1], LEX_SDELIM);
            YYERROR;
        }
        chg_strdelims ($2[0], $2[1]);
    }

| WORD STRING
    {
        #ifdef PARSER_DEBUG
            fprintf (stderr, "%s: Discarding entry [%s = %s].\n", PROJECT, $1, $2);
        #endif
    }
;


block: YSAMPLE STRING YENDSAMPLE
    {
        /*
         *  SAMPLE block    (STRING is non-empty if we get here)
         */
        char *line;

        #ifdef PARSER_DEBUG
            fprintf (stderr, "SAMPLE block rule satisfied\n");
        #endif

        if (designs[design_idx].sample) {
            yyerror ("duplicate SAMPLE block");
            YYERROR;
        }
        line = (char *) strdup ($2);
        if (line == NULL) {
            perror (PROJECT);
            YYABORT;
        }

        designs[design_idx].sample = line;
        ++pflicht;
    }

| YSHAPES  '{' slist  '}'
    {
        int     i,j;
        shape_t fshape;                  /* found shape */
        int     fside;                   /* first side */
        int     sc;                      /* side counter */
        int     side;                    /* effective side */
        int     rc;                      /* received return code */

        /*
         *  At least one shape must be specified
         */
        if (anz_shapespec < 1) {
            yyerror ("must specify at least one non-empty shape per design");
            YYERROR;
        }

        /*
         *  Ensure that all corners have been specified. Generate corners
         *  as necessary, starting at any side which already includes at
         *  least one shape in order to ensure correct measurements.
         */
        fshape = findshape (designs[design_idx].shape, ANZ_SHAPES);
        if (fshape == ANZ_SHAPES) {
            yyerror ("internal error");
            YYABORT;                        /* never happens ;-) */
        }
        fside = on_side (fshape, 0);
        if (fside == ANZ_SIDES) {
            yyerror ("internal error");
            YYABORT;                        /* never happens ;-) */
        }

        for (sc=0,side=fside; sc<ANZ_SIDES; ++sc,side=(side+1)%ANZ_SIDES) {
            shape_t   nshape;               /* next shape */
            sentry_t *c;                    /* corner to be processed */
            c = designs[design_idx].shape + sides[side][SHAPES_PER_SIDE-1];

            if (isempty(c)) {
                nshape = findshape (c, SHAPES_PER_SIDE);
                if (side == BLEF || side == BRIG) {
                    if (nshape == SHAPES_PER_SIDE)
                        c->height = 1;
                    else
                        c->height = c[nshape].height;
                    c->width = designs[design_idx].shape[fshape].width;
                }
                else {
                    if (nshape == SHAPES_PER_SIDE)
                        c->width = 1;
                    else
                        c->width = c[nshape].width;
                    c->height = designs[design_idx].shape[fshape].height;
                }
                c->elastic = 0;
                rc = genshape (c->width, c->height, &(c->chars));
                if (rc)
                    YYABORT;
            }

            fshape = sides[side][SHAPES_PER_SIDE-1];
        }

        /*
         *  For all sides whose side shapes have not been defined, generate
         *  an elastic middle side shape.
         */
        for (side=0; side<ANZ_SIDES; ++side) {
            int found = 0;
            for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                if (isempty (designs[design_idx].shape + sides[side][i]))
                    continue;
                else
                    found = 1;
            }
            if (!found) {
                sentry_t *c = designs[design_idx].shape
                    + sides[side][SHAPES_PER_SIDE/2];
                if (side == BLEF || side == BRIG) {
                    c->width = designs[design_idx].shape[sides[side][0]].width;
                    c->height = 1;
                }
                else {
                    c->width = 1;
                    c->height = designs[design_idx].shape[sides[side][0]].height;
                }
                c->elastic = 1;
                rc = genshape (c->width, c->height, &(c->chars));
                if (rc)
                    YYABORT;
            }
        }

        if (check_sizes())
            YYERROR;

        ++pflicht;
        if (++time_for_se_check > 1) {
            if (perform_se_check() != 0)
                YYERROR;
        }

        /*
         *  Compute minimum height/width of a box of current design
         */
        for (i=0; i<ANZ_SIDES; ++i) {
            size_t c = 0;
            if (i % 2) {                 /* vertical sides */
                for (j=0; j<SHAPES_PER_SIDE; ++j) {
                    if (!isempty(designs[design_idx].shape + sides[i][j]))
                        c += designs[design_idx].shape[sides[i][j]].height;
                }
                if (c > designs[design_idx].minheight)
                    designs[design_idx].minheight = c;
            }
            else {                       /* horizontal sides */
                for (j=0; j<SHAPES_PER_SIDE; ++j) {
                    if (!isempty(designs[design_idx].shape + sides[i][j]))
                        c += designs[design_idx].shape[sides[i][j]].width;
                }
                if (c > designs[design_idx].minwidth)
                    designs[design_idx].minwidth = c;
            }
        }

        /*
         *  Compute height of highest shape in design
         */
        for (i=0; i<ANZ_SHAPES; ++i) {
            if (isempty(designs[design_idx].shape + i))
                continue;
            if (designs[design_idx].shape[i].height > designs[design_idx].maxshapeheight)
                designs[design_idx].maxshapeheight = designs[design_idx].shape[i].height;
        }
        #ifdef PARSER_DEBUG
            fprintf (stderr, "Minimum box dimensions: width %d height %d\n",
                    designs[design_idx].minwidth, designs[design_idx].minheight);
            fprintf (stderr, "Maximum shape height: %d\n",
                    designs[design_idx].maxshapeheight);
        #endif
    }

| YELASTIC '(' elist ')'
    {
        ++pflicht;
        if (++time_for_se_check > 1) {
            if (perform_se_check() != 0)
                YYERROR;
        }
    }

| YREPLACE rflag STRING YWITH STRING
    {
        int a = designs[design_idx].anz_reprules;

        #ifdef PARSER_DEBUG
            fprintf (stderr, "Adding replacement rule: \"%s\" with \"%s\" (%c)\n",
                    $3, $5, $2);
        #endif

        designs[design_idx].reprules = (reprule_t *) realloc
            (designs[design_idx].reprules, (a+1) * sizeof(reprule_t));
        if (designs[design_idx].reprules == NULL) {
            perror (PROJECT);
            YYABORT;
        }
        memset (&(designs[design_idx].reprules[a]), 0, sizeof(reprule_t));
        designs[design_idx].reprules[a].search =
            (char *) strdup ($3);
        designs[design_idx].reprules[a].repstr =
            (char *) strdup ($5);
        if (designs[design_idx].reprules[a].search == NULL
         || designs[design_idx].reprules[a].repstr == NULL)
        {
            perror (PROJECT);
            YYABORT;
        }
        designs[design_idx].reprules[a].line = tjlineno;
        designs[design_idx].reprules[a].mode = $2;
        designs[design_idx].anz_reprules = a + 1;
    }

| YREVERSE rflag STRING YTO STRING
    {
        int a = designs[design_idx].anz_revrules;

        #ifdef PARSER_DEBUG
            fprintf (stderr, "Adding reversion rule: \"%s\" to \"%s\" (%c)\n",
                    $3, $5, $2);
        #endif

        designs[design_idx].revrules = (reprule_t *) realloc
            (designs[design_idx].revrules, (a+1) * sizeof(reprule_t));
        if (designs[design_idx].revrules == NULL) {
            perror (PROJECT);
            YYABORT;
        }
        memset (&(designs[design_idx].revrules[a]), 0, sizeof(reprule_t));
        designs[design_idx].revrules[a].search =
            (char *) strdup ($3);
        designs[design_idx].revrules[a].repstr =
            (char *) strdup ($5);
        if (designs[design_idx].revrules[a].search == NULL
         || designs[design_idx].revrules[a].repstr == NULL)
        {
            perror (PROJECT);
            YYABORT;
        }
        designs[design_idx].revrules[a].line = tjlineno;
        designs[design_idx].revrules[a].mode = $2;
        designs[design_idx].anz_revrules = a + 1;
    }

| YPADDING '{' wlist '}'
    {
        #ifdef PARSER_DEBUG
            fprintf (stderr, "Padding set to (l%d o%d r%d u%d)\n",
                    designs[design_idx].padding[BLEF],
                    designs[design_idx].padding[BTOP],
                    designs[design_idx].padding[BRIG],
                    designs[design_idx].padding[BBOT]);
        #endif
    }
;


rflag: YRXPFLAG
    {
        $$ = $1;
    }
|
    {
        $$ = 'g';
    }
;


elist: elist ',' elist_entry | elist_entry;


elist_entry: SHAPE
    {
        #ifdef PARSER_DEBUG
            fprintf (stderr, "Marked \'%s\' shape as elastic\n",
                    shape_name[(int)$1]);
        #endif
        designs[design_idx].shape[$1].elastic = 1;
    }
;


slist: slist slist_entry | slist_entry ;


slist_entry: SHAPE shape_def
    {
        #ifdef PARSER_DEBUG
            fprintf (stderr, "Adding shape spec for \'%s\' (width %d "
                    "height %d)\n", shape_name[$1], $2.width, $2.height);
        #endif

        if (isempty (designs[design_idx].shape + $1)) {
            designs[design_idx].shape[$1] = $2;
            if (!isdeepempty(&($2)))
                ++anz_shapespec;
        }
        else {
            yyerror ("duplicate specification for %s shape", shape_name[$1]);
            YYERROR;
        }
    }
;


shape_def: '(' shape_lines ')'
    {
        if ($2.width == 0 || $2.height == 0) {
            yyerror ("minimum shape dimension is 1x1 - clearing");
            freeshape (&($2));
        }
        $$ = $2;
    }

| '(' ')'
    {
        $$ = SENTRY_INITIALIZER;
    }
;


shape_lines: shape_lines ',' STRING
    {
        sentry_t rval = $1;
        size_t slen = strlen ($3);
        char **tmp;

        #ifdef PARSER_DEBUG
            fprintf (stderr, "Extending a shape entry\n");
        #endif

        if (slen != rval.width) {
            yyerror ("all elements of a shape spec must be of equal length");
            YYERROR;
        }

        rval.height++;
        tmp = (char **) realloc (rval.chars, rval.height*sizeof(char*));
        if (tmp == NULL) {
            perror (PROJECT": shape_lines11");
            YYABORT;
        }
        rval.chars = tmp;
        rval.chars[rval.height-1] = (char *) strdup ($3);
        if (rval.chars[rval.height-1] == NULL) {
            perror (PROJECT": shape_lines12");
            YYABORT;
        }
        $$ = rval;
    }

| STRING
    {
        sentry_t rval = SENTRY_INITIALIZER;

        #ifdef PARSER_DEBUG
            fprintf (stderr, "Initializing a shape entry with first line\n");
        #endif

        rval.width = strlen ($1);
        rval.height = 1;
        rval.chars = (char **) malloc (sizeof(char*));
        if (rval.chars == NULL) {
            perror (PROJECT": shape_lines21");
            YYABORT;
        }
        rval.chars[0] = (char *) strdup ($1);
        if (rval.chars[0] == NULL) {
            perror (PROJECT": shape_lines22");
            YYABORT;
        }
        $$ = rval;
    }
;


wlist: wlist wlist_entry | wlist_entry;


wlist_entry: WORD YNUMBER
    {
        if ($2 < 0) {
            yyerror ("padding must be a positive integer (%s %d) (ignored)",
                    $1, $2);
        }
        else {
            size_t len1 = strlen ($1);
            if (len1 <= 3 && !strncasecmp ("all", $1, len1)) {
                designs[design_idx].padding[BTOP] = $2;
                designs[design_idx].padding[BBOT] = $2;
                designs[design_idx].padding[BLEF] = $2;
                designs[design_idx].padding[BRIG] = $2;
            }
            else if (len1 <= 10 && !strncasecmp ("horizontal", $1, len1)) {
                designs[design_idx].padding[BRIG] = $2;
                designs[design_idx].padding[BLEF] = $2;
            }
            else if (len1 <= 8 && !strncasecmp ("vertical", $1, len1)) {
                designs[design_idx].padding[BTOP] = $2;
                designs[design_idx].padding[BBOT] = $2;
            }
            else if (len1 <= 3 && !strncasecmp ("top", $1, len1)) {
                designs[design_idx].padding[BTOP] = $2;
            }
            else if (len1 <= 5 && !strncasecmp ("right", $1, len1)) {
                designs[design_idx].padding[BRIG] = $2;
            }
            else if (len1 <= 4 && !strncasecmp ("left", $1, len1)) {
                designs[design_idx].padding[BLEF] = $2;
            }
            else if (len1 <= 6 && !strncasecmp ("bottom", $1, len1)) {
                designs[design_idx].padding[BBOT] = $2;
            }
            else {
                yyerror ("invalid padding area %s (ignored)", $1);
            }
        }
    }
;


%%

/*EOF*/                                          /* vim: set sw=4 cindent: */
