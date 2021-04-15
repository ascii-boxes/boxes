%top{
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
 * flex lexical analyzer for boxes configuration files
 */

#include "config.h"


typedef struct {
    int yyerrcnt;

    /** pointer to the currently active string delimiter character in bison_args */
    char *sdel_ptr;

    /** pointer to the currently active string escape character in bison_args */
    char *sesc_ptr;
} pass_to_flex;



void begin_speedmode(void *yyscanner);


/**
 * User-defined initializations for the lexer.
 *
 * Since this scanner must use REJECT in order to be able to process the string delimiter commands, it cannot
 * dynamically enlarge its input buffer to accomodate larger tokens. Thus, we simply set the buffer size to the
 * input file size plus 10 bytes margin-of-error.
 *
 * @param yyscanner pointer to the scanner data block
 * @param configfile the path to the config file we are reading
 */
void inflate_inbuf(void *yyscanner, const char *configfile);

}

%{
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "boxes.h"
#include "shape.h"
#include "tools.h"
#include "parsing.h"
#include "parser.h"


#define LEX_MAX_WARN 3                   /* number of lex errors per design */

static void report_state_char(char *symbol, char c, char *expected_state_str);

static void report_state(char *symbol, char *text, char *expected_state_str);

%}


%option 8bit
%option bison-bridge
%option case-insensitive
%option ecs
%option extra-type="pass_to_flex *"
%option never-interactive
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option reentrant
%option warn
%option yylineno


%x DELWORD
%x PARENT
%x SAMPLE
%x SPEEDMODE
%s ELASTIC
%s SHAPES


PWORD     [a-zA-ZäöüÄÖÜ][a-zA-Z0-9\-_üäöÜÄÖß]*
PWHITE    [\n \r\t]
PBOX      Box
SDELIM    [\"~\'`!@\%\&\*=:;<>\?/|\.\\]
PPARENT   parent
PFILENAME [^\r\n]+


%%


<INITIAL,DELWORD,ELASTIC,SHAPES>[ \r\t] /* ignore whitespace */

<INITIAL,DELWORD,ELASTIC,SHAPES>\n {}


<DELWORD>[^ \t\r\n]+ {
    /*
     *  String delimiter spec - like WORD, but allow any character
     */
    yylval->s = (char *) strdup (yytext);
    if (yylval->s == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    BEGIN(INITIAL);
    report_state("YDELWOR", yytext, "INITIAL");
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

    if (yytext[0] != *(yyextra->sdel_ptr)) {
        REJECT;                          /* that was not our delimiter */
    }

    yylval->s = (char *) strdup (yytext + 1);
    if (yylval->s == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    p = yylval->s;

    while (*p) {
        if (*p == *(yyextra->sesc_ptr)) {
            memmove (p, p+1, rest_len);     /* incl. '\0' */
            ++qcnt;
            --rest_len;
            if (*p == '\0')
                break;
        }
        else if (*p == *(yyextra->sdel_ptr)) {
            *p = '\0';
            yyless ((p-yylval->s)+2+qcnt);   /* string plus quotes */
            #ifdef LEXER_DEBUG
                fprintf (stderr, " STRING: \"%s\"\n", yylval->s);
            #endif
            return STRING;
        }
        --rest_len;
        ++p;
    }
    if ((yyextra->yyerrcnt)++ < 5) {
        yyerror(NULL, "Unterminated String -- %s", yytext);
    }
    return YUNREC;
}


{PPARENT} {
    BEGIN(PARENT);
    report_state("YPARENT", yytext, "PARENT");
    return YPARENT;
}

<PARENT>{PFILENAME} {
    char *p = yytext;
    while (*p == ' ' || *p == '\t') {
        ++p;
    }
    yylval->s = (char *) strdup (p);
    p = yylval->s + strlen(yylval->s) - 1;
    while ((*p == ' ' || *p == '\t') && p >= yylval->s) {
        *p-- = '\0';
    }
    BEGIN(INITIAL);
    report_state("FILENAM", yylval->s, "INITIAL");
    return FILENAME;
}

<PARENT>\r?\n {
    /* This is triggered only when no parent filename was specified. */
    BEGIN(INITIAL);
    report_state("     NL", "", "INITIAL");
}

Sample {
    BEGIN(SAMPLE);
    report_state("YSAMPLE", yytext, "SAMPLE");
    return YSAMPLE;
}


<SAMPLE>\n {
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
    yylval->s = (char *) strdup (yytext);
    if (yylval->s == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    *p-- = 'n';

    len = p - yytext;                    /* yyless(n): push back all but the first n */
    yyless (len);                        /* allow him to return YENDSAMPLE */

    yylval->s[len] = '\n';               /* replace 'e' with newline */
    btrim (yylval->s, &len);
    if (len > 0) {
        strcat (yylval->s, "\n");        /* memory was allocated with strdup */
        BEGIN(INITIAL);
        report_state(" STRING", yylval->s, "INITIAL");
        return STRING;
    }
    else {
        if ((yyextra->yyerrcnt)++ < 5) {
            yyerror(NULL, "SAMPLE block must not be empty");
        }
        BFREE (yylval->s);
        return YUNREC;
    }
}


<SAMPLE>. yymore();


ends[ \t\r]*$ {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "YENDSAM: %s\n", yytext);
    #endif
    return YENDSAMPLE;
}


Tags {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "  YTAGS: %s\n", yytext);
    #endif
    return YTAGS;
}

Elastic {
    BEGIN(ELASTIC);
    report_state("YELASTC", yytext, "ELASTIC");
    return YELASTIC;
}

Shapes {
    BEGIN(SHAPES);
    report_state("YSHAPES", yytext, "SHAPES");
    return YSHAPES;
}

{PBOX} {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "   YBOX: %s\n", yytext);
    #endif
    yyextra->yyerrcnt = 0;
    return YBOX;
}

Replace { return YREPLACE; }
Reverse { return YREVERSE; }
Padding { return YPADDING; }
End     { return YEND;     }
To      { return YTO;      }
With    { return YWITH;    }
Global  { yylval->c = 'g'; return YRXPFLAG; }
Once    { yylval->c = 'o'; return YRXPFLAG; }


<SHAPES,ELASTIC>nw  { yylval->shape = NW;  return SHAPE; }
<SHAPES,ELASTIC>nnw { yylval->shape = NNW; return SHAPE; }
<SHAPES,ELASTIC>n   { yylval->shape = N;   return SHAPE; }
<SHAPES,ELASTIC>nne { yylval->shape = NNE; return SHAPE; }
<SHAPES,ELASTIC>ne  { yylval->shape = NE;  return SHAPE; }
<SHAPES,ELASTIC>ene { yylval->shape = ENE; return SHAPE; }
<SHAPES,ELASTIC>e   { yylval->shape = E;   return SHAPE; }
<SHAPES,ELASTIC>ese { yylval->shape = ESE; return SHAPE; }
<SHAPES,ELASTIC>se  { yylval->shape = SE;  return SHAPE; }
<SHAPES,ELASTIC>sse { yylval->shape = SSE; return SHAPE; }
<SHAPES,ELASTIC>s   { yylval->shape = S;   return SHAPE; }
<SHAPES,ELASTIC>ssw { yylval->shape = SSW; return SHAPE; }
<SHAPES,ELASTIC>sw  { yylval->shape = SW;  return SHAPE; }
<SHAPES,ELASTIC>wsw { yylval->shape = WSW; return SHAPE; }
<SHAPES,ELASTIC>w   { yylval->shape = W;   return SHAPE; }
<SHAPES,ELASTIC>wnw { yylval->shape = WNW; return SHAPE; }

<ELASTIC>\) {
    BEGIN(INITIAL);
    report_state_char("SYMBOL", yytext[0], "INITIAL");
    return yytext[0];
}

<SHAPES>\} {
    BEGIN(INITIAL);
    report_state_char("SYMBOL", yytext[0], "INITIAL");
    return yytext[0];
}


author|designer|created|revision|revdate|indent {
    /*
     *  general key words
     */
    #ifdef LEXER_DEBUG
        fprintf (stderr, "KEYWORD: %s\n", yytext);
    #endif
    yylval->s = (char *) strdup (yytext);
    if (yylval->s == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    return KEYWORD;
}


Delimiter|Delim {
    /*
     *  Change string delimiting characters
     */
    BEGIN(DELWORD);
    report_state("YCHGDEL", yytext, "DELWORD");
    return YCHGDEL;
}


{PWORD} {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "   WORD: %s\n", yytext);
    #endif
    yylval->s = (char *) strdup (yytext);
    if (yylval->s == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    return WORD;
}


[\+-]?[0-9]+ {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "YNUMBER: %s\n", yytext);
    #endif
    yylval->num = atoi (yytext);
    return YNUMBER;
}


[,(){}] {
    #ifdef LEXER_DEBUG
        fprintf (stderr, " SYMBOL: \'%c\'\n", yytext[0]);
    #endif
    return yytext[0];
}


#.*$ {
    /* ignore comments */
    #ifdef LEXER_DEBUG
        fprintf (stderr, "COMMENT: %s\n", yytext+1);
    #endif
}


. {
    if ((yyextra->yyerrcnt)++ < LEX_MAX_WARN)
        yyerror(NULL, "Unrecognized input char \'%s\'", yytext);
    return YUNREC;
}


<SPEEDMODE>{PBOX}{PWHITE}+{PWORD}|{PPARENT} {
    /* end speedmode, but then give back the whole match so BOX or PARENT can be started properly */
    yyless(0);
    BEGIN(INITIAL);
    report_state(" STATUS", "", "INITIAL");
}

<SPEEDMODE>\n {}

<SPEEDMODE>. /* ignore anything else */


%%


void inflate_inbuf(void *yyscanner, const char *configfile)
{
    struct stat sinf;

    if (stat(configfile, &sinf)) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    struct yyguts_t *yyg = (struct yyguts_t *) yyscanner;
    yy_delete_buffer(YY_CURRENT_BUFFER, yyscanner);
    yy_switch_to_buffer (yy_create_buffer(yyin, sinf.st_size+10, yyscanner), yyscanner);
    BEGIN(INITIAL);
}



void begin_speedmode(void *yyscanner)
{
    struct yyguts_t *yyg = (struct yyguts_t *) yyscanner;
    BEGIN(SPEEDMODE);
    report_state(" STATUS", "begin_speedmode()", "SPEEDMODE");
}



static void report_state_char(char *symbol, char c, char *expected_state_str)
{
    char *s = (char *) malloc(4);
    sprintf(s, "'%c'", c);
    report_state(symbol, s, expected_state_str);
    BFREE(s);
}



static void report_state(char *symbol, char *text, char *expected_state_str)
{
    int lexerDebug = 0;
    #ifdef LEXER_DEBUG
        lexerDebug = 1;
    #endif
    if (lexerDebug) {
        fprintf(stderr, "%7s: %s -- STATE %s\n", symbol, text, expected_state_str);
    }
}


/*EOF*/                                         /* vim: set cindent sw=4: */
