%{
/*
 *  parser.y
 *
 *  yacc parser for boxes configuration files
 *
 *  Thomas Jensen <tsjensen@stud.informatik.uni-erlangen.de>
 *  Date created: March 16, 1999 (Tuesday, 17:17h)
 */

#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#include "boxes.h"


static int pflicht = 0;
static int time_for_corner_check = 0;

int corner_ambiguity()
/*
 *  Checks for ambiguity in corner specification using elastic and shape
 *  data. It must be clear which shape to use for each corner of the box.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    /* TODO */

    return 0;                            /* no ambiguity */
}

%}

%union {
    int num;
    char *s;
    shape_t shape;
    offset_t offset;
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
        ++time_for_corner_check;
        if (time_for_corner_check > 1 && corner_ambiguity()) {
            yyerror ("Ambiguous Shape/Elastic specification for corners");
            YYABORT;
        }
    }

| YELASTIC elist
    {
        int i;
        int ok[4] = {0, 0, 0, 0};

        /*
         *  Check that at least one shape per side is elastic
         */
        for (i=0; i<designs[design_idx].anz_elastic; ++i) {
            if (designs[design_idx].elastic[i] == NW
             || designs[design_idx].elastic[i] == NNW
             || designs[design_idx].elastic[i] == N
             || designs[design_idx].elastic[i] == NNE
             || designs[design_idx].elastic[i] == NE)
                ++ok[0];
            if (designs[design_idx].elastic[i] == NE
             || designs[design_idx].elastic[i] == ENE
             || designs[design_idx].elastic[i] == E
             || designs[design_idx].elastic[i] == ESE
             || designs[design_idx].elastic[i] == SE)
                ++ok[1];
            if (designs[design_idx].elastic[i] == SW
             || designs[design_idx].elastic[i] == SSW
             || designs[design_idx].elastic[i] == S
             || designs[design_idx].elastic[i] == SSE
             || designs[design_idx].elastic[i] == SE)
                ++ok[2];
            if (designs[design_idx].elastic[i] == NW
             || designs[design_idx].elastic[i] == WNW
             || designs[design_idx].elastic[i] == W
             || designs[design_idx].elastic[i] == WSW
             || designs[design_idx].elastic[i] == SW)
                ++ok[3];
        }
        for (i=0; i<4; ++i) {
            if (!ok[i]) {
                yyerror ("At least one shape per side must be elastic");
                YYABORT;
            }   
        }
        ++pflicht;
        ++time_for_corner_check;
        if (time_for_corner_check > 1 && corner_ambiguity()) {
            yyerror ("Ambiguous Shape/Elastic specification for corners");
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
        designs[design_idx].elastic[designs[design_idx].anz_elastic] = $3;
        ++(designs[design_idx].anz_elastic);
    }
| SHAPE
    {
        #ifdef DEBUG
            fprintf (stderr, "Marked \'%s\' shape as elastic\n",
                    shape_name[(int)$1]);
        #endif
        designs[design_idx].elastic[designs[design_idx].anz_elastic] = $1;
        ++(designs[design_idx].anz_elastic);
    }
;

slist: '(' slist_entries ')'
{
    $$ = $2;
}
| '(' ')'
{
    $$ = (sentry_t) { NULL, 0, 0 };
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
