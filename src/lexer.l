%{
/*
 *  File:             lexer.l
 *  Date created:     March 15, 1999 (Monday, 17:16h)
 *  Author:           Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: lexer.l,v 1.11 1999/06/28 18:37:38 tsjensen Exp tsjensen $
 *  Language:         lex (ANSI C)
 *  Purpose:          flex lexical analyzer for boxes configuration files
 *  Remarks:          ---
 *
 *  Revision History:
 *
 *    $Log: lexer.l,v $
 *    Revision 1.11  1999/06/28 18:37:38  tsjensen
 *    Replaced DEBUG macro with LEXER_DEBUG, which is now activated in boxes.h
 *    New tokens to, with, global, once
 *    Added LEX_MAX_WARN macro to limit number of lex errors printed per design
 *    Replaced exit()s with return YUNREC where errors are not fatal
 *
 *    Revision 1.10  1999/06/28 12:17:46  tsjensen
 *    Added tokens YBOX and YEND (thus, BOX and END are no longer YKEYWORDs)
 *    Added #define FILE_LEXER_L around #include boxes.h to please compiler
 *
 *    Revision 1.9  1999/06/22 12:00:05  tsjensen
 *    Added #undef DEBUG, because DEBUGging is now activated in boxes.h
 *    Added #include tools.h
 *
 *    Revision 1.8  1999/06/20 14:17:58  tsjensen
 *    Added "padding" keyword and recogintion of numbers (YNUMBER)
 *
 *    Revision 1.7  1999/06/17 19:05:46  tsjensen
 *    Bugfix: Sample block analysis didn't handle empty blocks
 *
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

#include <string.h>
#include "shape.h"
#define FILE_LEXER_L
#include "boxes.h"
#undef FILE_LEXER_L
#include "tools.h"
#include "y.tab.h"
#include "parser.h"



#define LEX_MAX_WARN 3                   /* number of lex errors per design */


static const char rcsid_lexer_l[] =
    "$Id: lexer.l,v 1.11 1999/06/28 18:37:38 tsjensen Exp tsjensen $";


int yylineno = 1;
int yyerrcnt = 0;

%}

%option nounput
%option noyywrap

%x SAMPLE1
%x SAMPLE2
%x SPEEDMODE
%s SHAPES
%s ELASTIC

PWORD  [a-zA-ZäöüÄÖÜ][a-zA-Z0-9\-_üäöÜÄÖß]*
PWHITE [\n \r\t]
PBOX   Box

%%

<SAMPLE1,SHAPES,ELASTIC,INITIAL>[ \r\t] /* ignore whitespace */

<SAMPLE1,SHAPES,ELASTIC,INITIAL>\n ++yylineno;

\"[^"\n]*$ {
    if (yyerrcnt++ < 5)
        yyerror ("Unterminated String at %s", yytext);
    return YUNREC;
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

        #ifdef LEXER_DEBUG
            fprintf (stderr, "\n STRING: \"%s\"", yytext);
        #endif
        yylval.s = (char *) strdup (yytext);
        if (yylval.s == NULL) {
            perror (PROJECT);
            exit (EXIT_FAILURE);
        }
        return STRING;
    }
}

Sample {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\n   WORD: %s -- STATE SAMPLE1", yytext);
    #endif
    BEGIN SAMPLE1;
    return YSAMPLE;
}

<SAMPLE1>\{ {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\n SYMBOL: \'%c\' -- STATE SAMPLE2", yytext[0]);
    #endif
    BEGIN SAMPLE2;
    return yytext[0];
}

<SAMPLE1>[^\{\n\r\t ]+ {
    if (yyerrcnt++ < 5)
        yyerror ("Syntax Error at \"%s\"", yytext);
    return YUNREC;
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
            #ifdef LEXER_DEBUG
                fprintf (stderr, "\n SAMPLE: %s -- STATE INITIAL", yytext);
            #endif
            yylval.s = (char *) strdup (yytext);
            if (yylval.s == NULL) {
                perror (PROJECT);
                exit (EXIT_FAILURE);
            }
            return STRING;
        }
    }
    else {
        if (yyerrcnt++ < 5)
            yyerror ("SAMPLE entry must not be empty");
        return YUNREC;
    }
}

<SAMPLE2>. {
    yymore();
}

Elastic {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\nYELASTC: %s -- STATE ELASTIC", yytext);
    #endif
    BEGIN ELASTIC;
    return YELASTIC;
}

Shapes {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\nYSHAPES: %s -- STATE SHAPES", yytext);
    #endif
    BEGIN SHAPES;
    return YSHAPES;
}

{PBOX} {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\n   YBOX: %s", yytext);
    #endif
    yyerrcnt = 0;
    return YBOX;
}

Replace { return YREPLACE; }
Reverse { return YREVERSE; }
Padding { return YPADDING; }
End     { return YEND;     }
To      { return YTO;      }
With    { return YWITH;    }
Global  { yylval.c = 'g'; return YRXPFLAG; }
Once    { yylval.c = 'o'; return YRXPFLAG; }


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
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\n SYMBOL: \'%c\' -- STATE INITIAL", yytext[0]);
    #endif
    BEGIN INITIAL;
    return yytext[0];
}

<SHAPES>\} {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\n SYMBOL: \'%c\' -- STATE INITIAL", yytext[0]);
    #endif
    BEGIN INITIAL;
    return yytext[0];
}


author|created|revision|revdate|indent {
    /*
     *  general key words
     */
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\nKEYWORD: %s", yytext);
    #endif
    yylval.s = (char *) strdup (yytext);
    if (yylval.s == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    return KEYWORD;
}


{PWORD} {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\n   WORD: %s", yytext);
    #endif
    yylval.s = (char *) strdup (yytext);
    if (yylval.s == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    return WORD;
}


[\+-]?[0-9]+ {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\nYNUMBER: %s", yytext);
    #endif
    yylval.num = atoi (yytext);
    return YNUMBER;
}


[,(){}] {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\n SYMBOL: \'%c\'", yytext[0]);
    #endif
    return yytext[0];
}


#.*$ {
    /* ignore comments */
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\nCOMMENT: %s", yytext+1);
    #endif
}


. {
    if (yyerrcnt++ < LEX_MAX_WARN)
        yyerror ("Unrecognized input char \'%s\'", yytext);
    return YUNREC;
}


<SPEEDMODE>{PBOX}{PWHITE}+{PWORD} {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\n STATUS: %s -- BEGIN INITIAL", yytext);
    #endif
    yyless (0);
    speeding = 0;
    BEGIN INITIAL;
}

<SPEEDMODE>\n ++yylineno;

<SPEEDMODE>. /* ignore anything else */


%%


void begin_speedmode()
{
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\n STATUS: begin_speedmode() -- STATE SPEEDMODE");
    #endif
    BEGIN SPEEDMODE;
}


/*EOF*/                                         /* vim: set cindent sw=4: */
