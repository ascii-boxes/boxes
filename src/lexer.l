%{
/*
 *  File:             lexer.l
 *  Date created:     March 15, 1999 (Monday, 17:16h)
 *  Author:           Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: lexer.l,v 1.6 1999/06/14 12:13:41 tsjensen Exp tsjensen $
 *  Language:         lex (ANSI C)
 *  Purpose:          flex lexical analyzer for boxes configuration files
 *  Remarks:          ---
 *
 *  Revision History:
 *
 *    $Log: lexer.l,v $
 *    Revision 1.6  1999/06/14 12:13:41  tsjensen
 *    Added Reverse pattern
 *
 *    Revision 1.5  1999/06/03 18:54:55  tsjensen
 *    *** empty log message ***
 *
 *    Revision 1.4  1999/04/09 13:31:13  tsjensen
 *    Removed all code related to OFFSET blocks (obsolete)
 *
 *    Revision 1.3  1999/04/04 16:11:39  tsjensen
 *    Added indent keyword
 *    Added Replace token
 *    Some fiddling which will hopefully fix a line counting bug
 *
 *    Revision 1.2  1999/03/30 09:42:51  tsjensen
 *    Added rcs keywords and standard file header.
 *
 *    Revision 1.1  1999/03/18 15:09:48  tsjensen
 *    Initial revision
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/* #define DEBUG */

#include <string.h>
#include "boxes.h"
#include "y.tab.h"

const char rcsid_lexer_l[] =
    "$Id: lexer.l,v 1.6 1999/06/14 12:13:41 tsjensen Exp tsjensen $";

int yylineno = 1;

%}

%x SAMPLE1
%x SAMPLE2
%s SHAPES
%s ELASTIC

%%

<SAMPLE1,SHAPES,ELASTIC,INITIAL>[\r\t ]+ /* ignore whitespace */

<SAMPLE1,SHAPES,ELASTIC,INITIAL>\n yylineno++;

\"[^"\n]*$ {
    yyerror ("Unterminated String at %s", yytext);
    exit (EXIT_FAILURE);
}

\"[^"\n]*\" {
    int bidx = yyleng-2;                 /* backslash index */

    while (yytext[bidx] == '\\') bidx--;

    if ((yyleng-2-bidx) % 2) {
        yyless (yyleng-1);               /* give back last quote */
        yymore();                        /* append next string */
    }
    else {
        for (bidx=0; bidx<yyleng-1; ++bidx) {
            if (yytext[bidx] == '\\') {
                memmove (yytext+bidx, yytext+bidx+1, yyleng-bidx-1);
                yytext[yyleng-1] = '\0';
                --yyleng;                /* inefficient */
            }
        }

        memmove (yytext, yytext+1, yyleng-1);     /* cut quotes */
        --yyleng;
        yytext[yyleng-1] = '\0';
        --yyleng;

        #ifdef DEBUG
            fprintf (stderr, "\n STRING: \"%s\"", yytext);
        #endif
        yylval.s = (char *) strdup (yytext);
        return STRING;
    }
}

Sample {
    #ifdef DEBUG
        fprintf (stderr, "\n   WORD: %s -- STATE SAMPLE1", yytext);
    #endif
    BEGIN SAMPLE1;
    return YSAMPLE;
}

<SAMPLE1>\{ {
    #ifdef DEBUG
        fprintf (stderr, "\n SYMBOL: \'%c\' -- STATE SAMPLE2", yytext[0]);
    #endif
    BEGIN SAMPLE2;
    return yytext[0];
}

<SAMPLE1>[^{\n\t\r ]+ {
    yyerror ("Syntax Error at \"%s\"", yytext);
    exit (EXIT_FAILURE);
}

<SAMPLE2>\n {
    ++yylineno;
    if (yyleng > 1)
        yymore();
}

<SAMPLE2>\} {
    int bidx = yyleng-2;                 /* backslash index */

    if (bidx >= 0) {
        while (bidx >= 0 && yytext[bidx] == '\\') bidx--;

        if ((yyleng-2-bidx) % 2) {       /* odd number of backslashes */
            yymore();                    /* append next string */
        }
        else {
            yyless (yyleng-1);           /* have him recognize '}' symbol */

            for (bidx=yyleng-1; yytext[bidx]=='\n'; --bidx) {
                yytext[bidx] = '\0';    /* remove trailing newlines */
                --yyleng;
            }
            for (bidx=0; bidx<yyleng-1; ++bidx) {
                if (yytext[bidx] == '\\') {
                    memmove (yytext+bidx, yytext+bidx+1, yyleng-bidx-1);
                    yytext[yyleng-1] = '\0';
                    --yyleng;                   /* inefficient */
                }
            }
            BEGIN INITIAL;
            #ifdef DEBUG
                fprintf (stderr, "\n SAMPLE: %s -- STATE INITIAL", yytext);
            #endif
            yylval.s = (char *) strdup (yytext);
            return STRING;
        }
    }
    else {
        yyerror ("SAMPLE entry must not be empty");
        exit (EXIT_FAILURE);
    }
}

<SAMPLE2>. {
    yymore();
}

elastic {
    #ifdef DEBUG
        fprintf (stderr, "\nYELASTC: %s -- STATE ELASTIC", yytext);
    #endif
    BEGIN ELASTIC;
    return YELASTIC;
}

Shapes {
    #ifdef DEBUG
        fprintf (stderr, "\nYSHAPES: %s -- STATE SHAPES", yytext);
    #endif
    BEGIN SHAPES;
    return YSHAPES;
}

Replace {
    #ifdef DEBUG
        fprintf (stderr, "\nYREPLACE: %s", yytext);
    #endif
    return YREPLACE;
}

Reverse {
    #ifdef DEBUG
        fprintf (stderr, "\nYREVERSE: %s", yytext);
    #endif
    return YREVERSE;
}


<SHAPES,ELASTIC>nw  { yylval.shape = NW;  return SHAPE; }
<SHAPES,ELASTIC>nnw { yylval.shape = NNW; return SHAPE; }
<SHAPES,ELASTIC>n   { yylval.shape = N;   return SHAPE; }
<SHAPES,ELASTIC>nne { yylval.shape = NNE; return SHAPE; }
<SHAPES,ELASTIC>ne  { yylval.shape = NE;  return SHAPE; }
<SHAPES,ELASTIC>ene { yylval.shape = ENE; return SHAPE; }
<SHAPES,ELASTIC>e   { yylval.shape = E;   return SHAPE; }
<SHAPES,ELASTIC>ese { yylval.shape = ESE; return SHAPE; }
<SHAPES,ELASTIC>se  { yylval.shape = SE;  return SHAPE; }
<SHAPES,ELASTIC>sse { yylval.shape = SSE; return SHAPE; }
<SHAPES,ELASTIC>s   { yylval.shape = S;   return SHAPE; }
<SHAPES,ELASTIC>ssw { yylval.shape = SSW; return SHAPE; }
<SHAPES,ELASTIC>sw  { yylval.shape = SW;  return SHAPE; }
<SHAPES,ELASTIC>wsw { yylval.shape = WSW; return SHAPE; }
<SHAPES,ELASTIC>w   { yylval.shape = W;   return SHAPE; }
<SHAPES,ELASTIC>wnw { yylval.shape = WNW; return SHAPE; }

<ELASTIC>\) {
    #ifdef DEBUG
        fprintf (stderr, "\n SYMBOL: \'%c\' -- STATE INITIAL", yytext[0]);
    #endif
    BEGIN INITIAL;
    return yytext[0];
}

<SHAPES>\} {
    #ifdef DEBUG
        fprintf (stderr, "\n SYMBOL: \'%c\' -- STATE INITIAL", yytext[0]);
    #endif
    BEGIN INITIAL;
    return yytext[0];
}


BOX|revision|author|created|indent|revdate|END {
    /*
     *  general key words
     */
    #ifdef DEBUG
        fprintf (stderr, "\nKEYWORD: %s", yytext);
    #endif
    yylval.s = (char *) strdup (yytext);
    return KEYWORD;
}


[a-zA-ZäöüÄÖÜ][a-zA-Z0-9\-_üäöÜÄÖß]*  {
    #ifdef DEBUG
        fprintf (stderr, "\n   WORD: %s", yytext);
    #endif
    yylval.s = (char *) strdup (yytext);
    return WORD;
}


[,(){}] {
    #ifdef DEBUG
        fprintf (stderr, "\n SYMBOL: \'%c\'", yytext[0]);
    #endif
    return yytext[0];
}


#.*$ {
    /* ignore comments */
    #ifdef DEBUG
        fprintf (stderr, "\nCOMMENT: %s", yytext+1);
    #endif
}


. {
    yyerror ("Unrecognized input at %s", yytext);
    exit (EXIT_FAILURE);
}


%%


/*EOF*/                                         /* vim: set cindent sw=4: */
