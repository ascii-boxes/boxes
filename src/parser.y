%{
/*
 *  File:             parser.y
 *  Date created:     March 16, 1999 (Tuesday, 17:17h)
 *  Author:           Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: parser.y,v 1.2 1999/03/19 17:57:20 tsjensen Exp tsjensen $
 *  Language:         yacc (ANSI C)
 *  Purpose:          Yacc parser for boxes configuration files
 *  Remarks:          ---
 *
 *  Revision History:
 *
 *    $Log: parser.y,v $
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

#ident "$Id: parser.y,v 1.2 1999/03/19 17:57:20 tsjensen Exp tsjensen $";


static int pflicht = 0;
static int time_for_se_check = 0;



shape_t corner_ambiguity()
/*
 *  Checks for ambiguity in corner specification using elastic and shape
 *  data. It must be clear which shape to use for each corner of the box.
 *
 *  A corner specification is ambiguous if the distance between the corner
 *  and the nearest specified shape in horizontal and vertical direction is
 *  equal and both shapes have the same elasticity.
 *
 *  RETURNS:  ANZ_SHAPES   no ambiguity
 *            a corner     spec is ambiguous
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int hdist, vdist;                    /* distances to corners */
    shape_t hcand, vcand;                /* candidate shapes */
    int c;

    for (c=0; c<ANZ_CORNERS; ++c) {

        hdist = vdist = 0;
        hcand = vcand = 0;

        switch (corners[c])
        {
            case NW:
                if (designs[design_idx].shape[NW].chars == NULL) {
                    if      (designs[design_idx].shape[NNW].chars) { hdist = 1; hcand = NNW; }
                    else if (designs[design_idx].shape[N].chars)   { hdist = 2; hcand = N;   }
                    else if (designs[design_idx].shape[NNE].chars) { hdist = 3; hcand = NNE; }
                    else if (designs[design_idx].shape[NE].chars)  { hdist = 4; hcand = NE;  }
                    if      (designs[design_idx].shape[WNW].chars) { vdist = 1; vcand = WNW; }
                    else if (designs[design_idx].shape[W].chars)   { vdist = 2; vcand = W;   }
                    else if (designs[design_idx].shape[WSW].chars) { vdist = 3; vcand = WSW; }
                    else if (designs[design_idx].shape[SW].chars)  { vdist = 4; vcand = SW;  }
                }
                else
                    continue;
                break;

            case NE:
                if (designs[design_idx].shape[NE].chars == NULL) {
                    if      (designs[design_idx].shape[NNE].chars) { hdist = 1; hcand = NNE; }
                    else if (designs[design_idx].shape[N].chars)   { hdist = 2; hcand = N;   }
                    else if (designs[design_idx].shape[NNW].chars) { hdist = 3; hcand = NNW; }
                    else if (designs[design_idx].shape[NW].chars)  { hdist = 4; hcand = NW;  }
                    if      (designs[design_idx].shape[ENE].chars) { vdist = 1; vcand = ENE; }
                    else if (designs[design_idx].shape[E].chars)   { vdist = 2; vcand = E;   }
                    else if (designs[design_idx].shape[ESE].chars) { vdist = 3; vcand = ESE; }
                    else if (designs[design_idx].shape[SE].chars)  { vdist = 4; vcand = SE;  }
                }
                else
                    continue;
                break;

            case SE:
                if (designs[design_idx].shape[SE].chars == NULL) {
                    if      (designs[design_idx].shape[SSE].chars) { hdist = 1; hcand = SSE; }
                    else if (designs[design_idx].shape[S].chars)   { hdist = 2; hcand = S;   }
                    else if (designs[design_idx].shape[SSW].chars) { hdist = 3; hcand = SSW; }
                    else if (designs[design_idx].shape[SW].chars)  { hdist = 4; hcand = SW;  }
                    if      (designs[design_idx].shape[ESE].chars) { vdist = 1; vcand = ESE; }
                    else if (designs[design_idx].shape[E].chars)   { vdist = 2; vcand = E;   }
                    else if (designs[design_idx].shape[ENE].chars) { vdist = 3; vcand = ENE; }
                    else if (designs[design_idx].shape[NE].chars)  { vdist = 4; vcand = NE;  }
                }
                else
                    continue;
                break;

            case SW:
                if (designs[design_idx].shape[SW].chars == NULL) {
                    if      (designs[design_idx].shape[SSW].chars) { hdist = 1; hcand = SSW; }
                    else if (designs[design_idx].shape[S].chars)   { hdist = 2; hcand = S;   }
                    else if (designs[design_idx].shape[SSE].chars) { hdist = 3; hcand = SSE; }
                    else if (designs[design_idx].shape[SE].chars)  { hdist = 4; hcand = SE;  }
                    if      (designs[design_idx].shape[WSW].chars) { vdist = 1; vcand = WSW; }
                    else if (designs[design_idx].shape[W].chars)   { vdist = 2; vcand = W;   }
                    else if (designs[design_idx].shape[WNW].chars) { vdist = 3; vcand = WNW; }
                    else if (designs[design_idx].shape[NW].chars)  { vdist = 4; vcand = NW;  }
                }
                else
                    continue;
                break;

            default:
                fprintf (stderr, "%s: internal error. Sorry.\n", PROJECT);
                return corners[c];
        }

        if ((hdist == vdist) &&
         designs[design_idx].shape[hcand].elastic ==
         designs[design_idx].shape[vcand].elastic)
            return corners[c];
    }

    return (shape_t) ANZ_SHAPES;         /* no ambiguity */
}



shape_t non_existent_elastics()
{
    shape_t i;

    for (i=0; i<ANZ_SHAPES; ++i) {
        if (designs[design_idx].shape[i].elastic
         && (designs[design_idx].shape[i].chars == NULL))
            return i;
    }

    return (shape_t) ANZ_SHAPES;         /* all elastic shapes exist */
}



int insufficient_elasticity()
{
    int i;
    shape_t s;
    int ok[4] = {0, 0, 0, 0};            /* N, E, S, W */

    for (s=0; s<ANZ_SHAPES; ++s) {
        if (designs[design_idx].shape[s].elastic) {
            switch (s) {
                case NW: ok[0] = 1; ok[3] = 1; break;
                case SW: ok[2] = 1; ok[3] = 1; break;
                case NE: ok[0] = 1; ok[1] = 1; break;
                case SE: ok[2] = 1; ok[1] = 1; break;
                case N: case NNE: case NNW: ok[0] = 1; break;
                case E: case ESE: case ENE: ok[1] = 1; break;
                case S: case SSE: case SSW: ok[2] = 1; break;
                case W: case WSW: case WNW: ok[3] = 1; break;
                default:
                    yyerror ("Internal parser error");
                    return 1;
            }
        }
    }

    for (i=0; i<4; ++i) {
        if (!ok[i]) return 1;
    }

    return 0;                            /* no problem detected */
}



int perform_se_check()
{
    shape_t s_rc;

    s_rc = non_existent_elastics();
    if (s_rc != ANZ_SHAPES) {
        yyerror ("Shape %s has been specified as elastic but doesn't exist",
                shape_name[s_rc]);
        return 1;
    }

    if (insufficient_elasticity()) {
        yyerror ("At least one shape per side must be elastic");
        return 1;
    }

    s_rc = corner_ambiguity();
    if (s_rc != ANZ_SHAPES) {
        yyerror ("Ambiguous Shape/Elastic specification for %s corner",
                shape_name[s_rc]);
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
        /*
         *  Check that at least one shape per side is specified
         */
        if (!((designs[design_idx].shape[NW].chars
            || designs[design_idx].shape[NNW].chars
            || designs[design_idx].shape[N].chars
            || designs[design_idx].shape[NNE].chars
            || designs[design_idx].shape[NE].chars)
           && (designs[design_idx].shape[NE].chars
            || designs[design_idx].shape[ENE].chars
            || designs[design_idx].shape[E].chars
            || designs[design_idx].shape[ESE].chars
            || designs[design_idx].shape[SE].chars)
           && (designs[design_idx].shape[SW].chars
            || designs[design_idx].shape[SSW].chars
            || designs[design_idx].shape[S].chars
            || designs[design_idx].shape[SSE].chars
            || designs[design_idx].shape[SE].chars)
           && (designs[design_idx].shape[NW].chars
            || designs[design_idx].shape[WNW].chars
            || designs[design_idx].shape[W].chars
            || designs[design_idx].shape[WSW].chars
            || designs[design_idx].shape[SW].chars)))
        {
            yyerror ("Must specify at least one shape per side");
            YYABORT;
        }

        ++pflicht;
        if (++time_for_se_check > 1) {
            if (perform_se_check() != 0)
                YYABORT;
        }
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

        designs[design_idx].shape[$2] = $3;
    }

| SHAPE slist
    {
        #ifdef DEBUG
            fprintf (stderr, "Adding shape spec for \'%s\' (width %d "
                    "height %d)\n", shape_name[$1], $2.width, $2.height);
        #endif

        designs[design_idx].shape[$1] = $2;
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

            $$ = (sentry_t) { NULL, 0, 0, 0 };
        }
        else {
            $$ = $2;
        }
    }

| '(' ')'
    {
        $$ = (sentry_t) { NULL, 0, 0, 0 };
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
        sentry_t rval;

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
