%{
/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (C) 1999  Thomas Jensen and the boxes contributors
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
 * flex lexical analyzer for boxes configuration files
 */

#include "config.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "shape.h"
#define FILE_LEXER_L
#include "boxes.h"
#undef FILE_LEXER_L
#include "tools.h"
#include "parser.h"
#include "lexer.h"



#define LEX_MAX_WARN 3                   /* number of lex errors per design */


int tjlineno = 1;

static int yyerrcnt = 0;

static char sdel = '\"';
static char sesc = '\\';


/*
 *  User-defined initializations for the lexer
 */
static void inflate_inbuf();
#define YY_USER_INIT  inflate_inbuf()

%}


%option nounput
%option noyywrap
%option never-interactive
%option caseless
%option noinput
%option noyylineno


%x SAMPLE
%x SPEEDMODE
%x DELWORD
%s SHAPES
%s ELASTIC


PWORD  [a-zA-ZäöüÄÖÜ][a-zA-Z0-9\-_üäöÜÄÖß]*
PWHITE [\n \r\t]
PBOX   Box
SDELIM [\"~\'`!@\%\&\*=:;<>\?/|\.\\]


%%



<DELWORD,SHAPES,ELASTIC,INITIAL>[ \r\t] /* ignore whitespace */

<DELWORD,SHAPES,ELASTIC,INITIAL>\n ++tjlineno;


<DELWORD>[^ \t\r\n]+ {
    /*
     *  String delimiter spec - like WORD, but allow any character
     */
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\nYDELWOR: %s -- STATE INITIAL", yytext);
    #endif
    yylval.s = (char *) strdup (yytext);
    if (yylval.s == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    BEGIN INITIAL;
    return YDELWORD;
}


{SDELIM}.*$ {
    /*
     *  Strings  --  first match everything starting from a potential
     *  string delimiter until the end of the line. We will give back what
     *  we don't need and also detect unterminated strings.
     */
    char *p;
    int rest_len = yyleng - 1;           /* length of string pointed to by p */
    int qcnt = 0;                        /* esc char count in current string */

    if (yytext[0] != sdel) {
        REJECT;                          /* that was not our delimiter */
    }

    yylval.s = (char *) strdup (yytext + 1);
    if (yylval.s == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    p = yylval.s;

    while (*p) {
        if (*p == sesc) {
            memmove (p, p+1, rest_len);     /* incl. '\0' */
            ++qcnt;
            --rest_len;
            if (*p == '\0')
                break;
        }
        else if (*p == sdel) {
            *p = '\0';
            yyless ((p-yylval.s)+2+qcnt);   /* string plus quotes */
            #ifdef LEXER_DEBUG
                fprintf (stderr, "\n STRING: \"%s\"", yylval.s);
            #endif
            return STRING;
        }
        --rest_len;
        ++p;
    }
    if (yyerrcnt++ < 5)
        yyerror ("Unterminated String -- %s", yytext);
    return YUNREC;
}



Sample {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\nYSAMPLE: %s -- STATE SAMPLE", yytext);
    #endif
    BEGIN SAMPLE;
    return YSAMPLE;
}


<SAMPLE>\n {
    ++tjlineno;
    if (yyleng > 1)
        yymore();
}


<SAMPLE>^[ \t]*ends[ \t\r]*$ {
    char *p = yytext + yyleng -1;
    size_t len;                          /* length of sample */

    while (*p == ' ' || *p == '\t' || *p == '\r')
        --p;                             /* skip trailing whitespace */
    p -= 2;                              /* almost skip "ends" statement */
    *p = '\0';                           /* p now points to 'n' */
    yylval.s = (char *) strdup (yytext);
    if (yylval.s == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    *p-- = 'n';

    len = p - yytext;                    /* yyless(n): push back all but the first n */
    yyless (len);                        /* allow him to return YENDSAMPLE */

    yylval.s[len] = '\n';                /* replace 'e' with newline */
    btrim (yylval.s, &len);
    if (len > 0) {
        strcat (yylval.s, "\n");         /* memory was allocated with strdup */
        #ifdef LEXER_DEBUG
            fprintf (stderr, "\n STRING: \"%s\" -- STATE INITIAL", yylval.s);
        #endif
        BEGIN INITIAL;
        return STRING;
    }
    else {
        if (yyerrcnt++ < 5)
            yyerror ("SAMPLE block must not be empty");
        BFREE (yylval.s);
        return YUNREC;
    }
}


<SAMPLE>. yymore();


ends[ \t\r]*$ {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\nYENDSAM: %s", yytext);
    #endif
    return YENDSAMPLE;
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


author|designer|tags|created|revision|revdate|indent {
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


Delimiter|Delim {
    /*
     *  Change string delimiting characters
     */
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\nYCHGDEL: %s -- STATE DELWORD", yytext);
    #endif
    BEGIN DELWORD;
    return YCHGDEL;
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
        fprintf (stderr, "\n STATUS: %s -- STATE INITIAL", yytext);
    #endif
    yyless (0);
    speeding = 0;
    BEGIN INITIAL;
}

<SPEEDMODE>\n ++tjlineno;

<SPEEDMODE>. /* ignore anything else */


%%


static void inflate_inbuf()
/*
 *  User-defined initializations for the lexer.
 *
 *  Since this scanner must use REJECT in order to be able to process the
 *  string delimiter commands, it cannot dynamically enlarge its input
 *  buffer to accomodate larger tokens. Thus, we simply set the buffer size
 *  to the input file size plus 10 bytes margin-of-error.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    struct stat sinf;

    if (stat(yyfilename, &sinf)) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    yy_delete_buffer (YY_CURRENT_BUFFER);
    yy_switch_to_buffer (yy_create_buffer (yyin, sinf.st_size+10));
}



void begin_speedmode()
{
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\n STATUS: begin_speedmode() -- STATE SPEEDMODE");
    #endif
    BEGIN SPEEDMODE;
}



void chg_strdelims (const char asesc, const char asdel)
{
    #ifdef LEXER_DEBUG
        fprintf (stderr, "\n STATUS: chg_strdelims ('%c', '%c')", asesc, asdel);
    #endif
    sesc = asesc;
    sdel = asdel;
}



/*EOF*/                                         /* vim: set cindent sw=4: */
