%{
/*
 *  File:             parser.y
 *  Date created:     March 16, 1999 (Tuesday, 17:17h)
 *  Author:           Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: parser.y,v 1.4 1999/03/30 09:37:51 tsjensen Exp tsjensen $
 *  Language:         yacc (ANSI C)
 *  Purpose:          Yacc parser for boxes configuration files
 *  Remarks:          ---
 *
 *  Revision History:
 *
 *    $Log: parser.y,v $
 *    Revision 1.4  1999/03/30 09:37:51  tsjensen
 *    It drew a correct box for the first time!
 *
 *    Revision 1.3  1999/03/24 17:29:12  tsjensen
 *    Added detection of empty shapes ("") which are now cleared (+warning)
 *    Changed rcs string to #ident directive
 *
 *    Revision 1.2  1999/03/19 17:57:20  tsjensen
 *    ... still programming ...
 *
 *    Revision 1.1  1999/03/18 15:10:06  tsjensen
 *    Initial revision
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/* #define DEBUG */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boxes.h"

#ident "$Id: parser.y,v 1.4 1999/03/30 09:37:51 tsjensen Exp tsjensen $";


static int pflicht = 0;
static int time_for_se_check = 0;



int check_sizes()
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

    #ifdef DEBUG
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



int corner_check()
/*
 *  Check that all corners are explicitly specified in the config file.
 *  Check that no corners are elastic.
 *
 *  RETURNS:  == 0   no problem detected
 *            != 0   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int c;

    #ifdef DEBUG
        fprintf (stderr, "corner_check()\n");
    #endif

    for (c=0; c<ANZ_CORNERS; ++c) {
        if (isempty(designs[design_idx].shape+corners[c])) {
            yyerror ("Missing shape specification for %s corner",
                    shape_name[corners[c]]);
            return 1;
        }
        if (designs[design_idx].shape[corners[c]].elastic) {
            yyerror ("Corners may not be elastic (%s)", shape_name[corners[c]]);
            return 1;
        }
    }

    return 0;                            /* all clear */
}



shape_t non_existent_elastics()
{
    shape_t i;

    #ifdef DEBUG
        fprintf (stderr, "non_existent_elastics()\n");
    #endif

    for (i=0; i<ANZ_SHAPES; ++i) {
        if (designs[design_idx].shape[i].elastic
         && isempty(designs[design_idx].shape+i))
            return i;
    }

    return (shape_t) ANZ_SHAPES;         /* all elastic shapes exist */
}



int insufficient_elasticity()
{
    int i, j, ef;

    #ifdef DEBUG
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



int adjoining_elastics()
{
    int i, j, ef;

    #ifdef DEBUG
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



int perform_se_check()
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

%}


%union {
    int num;
    char *s;
    shape_t shape;
    soffset_t offset;
    sentry_t sentry;
}

%token YSHAPES YOFFSETS YELASTIC YSAMPLE
%token <s> KEYWORD
%token <s> WORD
%token <s> STRING
%token <num> NUMBER
%token <shape> SHAPE
%token <offset> OFFSET

%type <sentry> slist
%type <sentry> slist_entries

%%

config_file: config_file design | design ;

design: KEYWORD WORD layout KEYWORD WORD
    {
        design_t *tmp;

        #ifdef DEBUG
            fprintf (stderr, "--------- ADDING DESIGN \"%s\".\n", $2);
        #endif

        if (strcasecmp ($2, $5)) {
            yyerror ("Box design name differs at BOX and END");
            YYABORT;
        }
        if (strcasecmp ($1, "BOX")) {
            yyerror ("Box design must start with \"BOX boxname\"");
            YYABORT;
        }
        if (strcasecmp ($4, "END")) {
            yyerror ("Box design must end with \"END boxname\"");
            YYABORT;
        }
        if (pflicht < 3) {
            yyerror ("Entries SAMPLE, SHAPES, and ELASTIC are mandatory");
            YYABORT;
        }
        
        designs[design_idx].name = (char *) strdup ($2);
        pflicht = 0;
        time_for_se_check = 0;
        
        ++design_idx;
        tmp = (design_t *) realloc (designs, (design_idx+1)*sizeof(design_t));
        if (tmp) {
            designs = tmp;
            memset (&(designs[design_idx]), 0, sizeof(design_t));
        }
        else {
            perror ("boxes");
            YYABORT;
        }
    }
;

layout: layout entry | layout block | entry | block ;

entry: KEYWORD STRING
    {
        #ifdef DEBUG
            fprintf (stderr, "entry rule fulfilled [%s = %s]\n", $1, $2);
        #endif
        if (strcasecmp ($1, "author") == 0) {
            designs[design_idx].author = (char *) strdup ($2);
        }
        else if (strcasecmp ($1, "revision") == 0) {
            designs[design_idx].revision = (char *) strdup ($2);
        }
        else if (strcasecmp ($1, "created") == 0) {
            designs[design_idx].created = (char *) strdup ($2);
        }
        else if (strcasecmp ($1, "revdate") == 0) {
            designs[design_idx].revdate = (char *) strdup ($2);
        }
        else {
            fprintf (stderr, "boxes: Internal parser error (unrecognized: %s)"
                    " in line %d of %s.\n", $1, __LINE__, __FILE__);
            YYABORT;
        }
    }
| WORD STRING
    {
        fprintf (stderr, "boxes: Discarding entry [%s = %s].\n", $1, $2);
    }
;


block: YSAMPLE '{' STRING '}'
    {
        #ifdef DEBUG
            fprintf (stderr, "SAMPLE block rule fulfilled\n");
        #endif

        designs[design_idx].sample = (char *) strdup ($3);
        ++pflicht;
    }

| YSHAPES  '{' the_shapes  '}'
    {
        int i,j;

        /*
         *  Check that at least one shape per side is specified
         *  (excluding corners)
         */
        if ((isempty (designs[design_idx].shape + NNW)
          && isempty (designs[design_idx].shape +   N)
          && isempty (designs[design_idx].shape + NNE))
         || (isempty (designs[design_idx].shape + ENE)
          && isempty (designs[design_idx].shape +   E)
          && isempty (designs[design_idx].shape + ESE))
         || (isempty (designs[design_idx].shape + SSW)
          && isempty (designs[design_idx].shape +   S)
          && isempty (designs[design_idx].shape + SSE))
         || (isempty (designs[design_idx].shape + WNW)
          && isempty (designs[design_idx].shape +   W)
          && isempty (designs[design_idx].shape + WSW)))
        {
            yyerror ("Must specify at least one shape per side");
            YYABORT;
        }

        if (check_sizes())
            YYABORT;

        ++pflicht;
        if (++time_for_se_check > 1) {
            if (perform_se_check() != 0)
                YYABORT;
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
        #ifdef DEBUG
            fprintf (stderr, "Minimum box dimensions: width %d height %d\n",
                    designs[design_idx].minwidth, designs[design_idx].minheight);
        #endif
    }

| YELASTIC elist
    {
        ++pflicht;
        if (++time_for_se_check > 1) {
            if (perform_se_check() != 0)
                YYABORT;
        }
    }

| YOFFSETS '{' the_offsets '}'
;


the_shapes: the_shapes SHAPE slist
    {
        #ifdef DEBUG
            fprintf (stderr, "Adding shape spec for \'%s\' (width %d "
                    "height %d)\n", shape_name[$2], $3.width, $3.height);
        #endif

        if (isempty (designs[design_idx].shape+$2)) {
            designs[design_idx].shape[$2] = $3;
        }
        else {
            yyerror ("Duplicate specification for %s shape", shape_name[$2]);
            YYABORT;
        }
    }

| SHAPE slist
    {
        #ifdef DEBUG
            fprintf (stderr, "Adding shape spec for \'%s\' (width %d "
                    "height %d)\n", shape_name[$1], $2.width, $2.height);
        #endif

        if (isempty (designs[design_idx].shape + $1)) {
            designs[design_idx].shape[$1] = $2;
        }
        else {
            yyerror ("Duplicate specification for %s shape", shape_name[$1]);
            YYABORT;
        }
    }
;


the_offsets: the_offsets OFFSET NUMBER
    {
        #ifdef DEBUG
            fprintf (stderr, "Setting offset of \'%s\' to %d\n",
                    ofs_name[$2], $3);
        #endif

        designs[design_idx].offset[$2] = $3;
    }

| OFFSET NUMBER
    {
        #ifdef DEBUG
            fprintf (stderr, "Setting offset of \'%s\' to %d\n",
                    ofs_name[$1], $2);
        #endif

        designs[design_idx].offset[$1] = $2;
    }
;


elist: '(' elist_entries ')' ;


elist_entries: elist_entries ',' SHAPE
    {
        #ifdef DEBUG
            fprintf (stderr, "Marked \'%s\' shape as elastic\n",
                    shape_name[(int)$3]);
        #endif
        designs[design_idx].shape[$3].elastic = 1;
    }

| SHAPE
    {
        #ifdef DEBUG
            fprintf (stderr, "Marked \'%s\' shape as elastic\n",
                    shape_name[(int)$1]);
        #endif
        designs[design_idx].shape[$1].elastic = 1;
    }
;


slist: '(' slist_entries ')'
    {
        sentry_t rval = $2;

        if (rval.width == 0 || rval.height == 0) {

            int i;

            for (i=0; i<rval.height; ++i)
                BFREE (rval.chars[i]);
            BFREE (rval.chars);

            yyerror ("warning: minimum shape dimension is 1x1 - clearing.");

            $$ = SENTRY_INITIALIZER;
        }
        else {
            $$ = $2;
            /* memcpy (&($$), &($2), sizeof(sentry_t)); */
        }
    }

| '(' ')'
    {
        $$ = SENTRY_INITIALIZER;
    }
;


slist_entries: slist_entries ',' STRING
    {
        sentry_t rval = $1;
        size_t slen = strlen ($3);
        char **tmp;

        #ifdef DEBUG
            fprintf (stderr, "Extending a shape entry\n");
        #endif

        if (slen != rval.width) {
            yyerror ("All elements of a shape spec must be of equal length");
            YYABORT;
        }

        rval.height++;
        tmp = (char **) realloc (rval.chars, rval.height*sizeof(char*));
        if (tmp == NULL) {
            perror ("boxes: slist_entries11");
            YYABORT;
        }
        rval.chars = tmp;
        rval.chars[rval.height-1] = (char *) strdup ($3);
        if (rval.chars[rval.height-1] == NULL) {
            perror ("boxes: slist_entries12");
            YYABORT;
        }
        $$ = rval;
    }

| STRING
    {
        sentry_t rval = SENTRY_INITIALIZER;

        #ifdef DEBUG
            fprintf (stderr, "Initializing a shape entry with first line\n");
        #endif

        rval.width = strlen ($1);
        rval.height = 1;
        rval.chars = (char **) malloc (sizeof(char*));
        if (rval.chars == NULL) {
            perror ("boxes: slist_entries21");
            YYABORT;
        }
        rval.chars[0] = (char *) strdup ($1);
        if (rval.chars[0] == NULL) {
            perror ("boxes: slist_entries22");
            YYABORT;
        }
        $$ = rval;
    }
;

%%


/*EOF*/                                          /* vim: set sw=4 cindent: */
