%{
/*
 *  File:             lexer.l
 *  Date created:     March 15, 1999 (Monday, 17:16h)
 *  Author:           Copyright (C) 1999 Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: lexer.l,v 1.17 1999/08/20 19:51:12 tsjensen Exp tsjensen $
 *  Language:         lex (ANSI C)
 *  Purpose:          flex lexical analyzer for boxes configuration files
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
 *           - We don't use the yylineno %option. It is not only inefficient,
 *             but also doesn't work. :-| *doh*
 *
 *  Revision History:
 *
 *    $Log: lexer.l,v $
 *    Revision 1.17  1999/08/20 19:51:12  tsjensen
 *    Moved contents of YY_USER_INIT definition into a separate function which
 *    is now called by YY_USER_INIT (better readability)
 *
 *    Revision 1.16  1999/08/18 18:23:41  tsjensen
 *    Declared yyerrcnt to be static
 *    Added YY_USER_INIT macro to set the input buffer size to the config file
 *    size (plus a bit). This is supposed to be a workaround for the REJECT
 *    problem.
 *
 *    Revision 1.15  1999/08/18 15:40:10  tsjensen
 *    Added %options never-interactive and caseless
 *    Added code for DELIMITER statements
 *    Rewrote string rules to deal with delimiter statements
 *
 *    Revision 1.14  1999/08/16 16:28:03  tsjensen
 *    Implemented new SAMPLE block syntax
 *    Replaced states SAMPLE1 and SAMPLE2 with new state SAMPLE - this is now
 *    much simpler code
 *
 *    Revision 1.13  1999/07/22 12:21:14  tsjensen
 *    Added GNU GPL disclaimer
 *    Renamed y.tab.h include to parser.h (same file)
 *    Renamed parser.h include to lexer.h (same file)
 *    Added config.h include
 *
 *    Revision 1.12  1999/07/02 11:58:15  tsjensen
 *    Added begin_speedmode() which is called by parser.y
 *    Added state SPEEDMODE for fast skipping of designs
 *    Introduced definitions for PWORD, PBOX, and PWHITE (whitespace)
 *    Added %options nounput and noyywrap for easier compilation/linking
 *
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
 *    Added "padding" keyword and recognition of numbers (YNUMBER)
 *
 *    Revision 1.7  1999/06/17 19:05:46  tsjensen
 *    Bugfix: Sample block analysis didn't handle empty blocks
 *
 *    Revision 1.6  1999/06/14 12:13:41  tsjensen
 *    Added Reverse pattern
 *
 *    Revision 1.4  1999/04/09 13:31:13  tsjensen
 *    Removed all code related to OFFSET blocks (obsolete)
 *
 *    Revision 1.3  1999/04/04 16:11:39  tsjensen
 *    Added indent keyword
 *    Added Replace token
 *    Some fiddling which will hopefully fix a line counting bug
 *
 *    Revision 1.1  1999/03/18 15:09:48  tsjensen
 *    Initial revision
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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


static const char rcsid_lexer_l[] =
    "$Id: lexer.l,v 1.17 1999/08/20 19:51:12 tsjensen Exp tsjensen $";


int yylineno = 1;
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

<DELWORD,SHAPES,ELASTIC,INITIAL>\n ++yylineno;


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
    ++yylineno;
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

<SPEEDMODE>\n ++yylineno;

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
    YY_CURRENT_BUFFER = yy_create_buffer (yyin, sinf.st_size+10);
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
