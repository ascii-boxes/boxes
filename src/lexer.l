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

    /** the currently active string delimiter character */
    char sdel;

    /** the currently active string escape character */
    char sesc;
} pass_to_flex;


/*
 *  Valid characters to be used as string delimiters. Note that the
 *  following list must correspond to the SDELIM definition below.
 */
#define LEX_SDELIM  "\"~'`!@%&*=:;<>?/|.\\"
#define LEX_SDELIM_RECOMMENDED  "\"~'!|"


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

static int change_string_delimiters(pass_to_flex *extra, char *delim_expr);

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


%x BOX
%x SAMPLE
%x SHAPES
%x ELASTIC
%x DELIMSPEC
%x PARENT


PWORD     [a-zA-ZäöüÄÖÜ][a-zA-Z0-9\-_üäöÜÄÖß]*
PWHITE    [ \t\r\n]
SDELIM    [\"~\'`!@\%\&\*=:;<>\?/|\.\\]
PPARENT   parent
PFILENAME [^\r\n]+


%%

    /*
     * Precedence of rules:
     * - The rule that matches the most text wins.
     * - If two rules match the same amount of text, the one defined first (further up) wins.
     */


<INITIAL,BOX,DELIMSPEC,ELASTIC,SHAPES>{PWHITE}  /* ignore whitespace */

<DELIMSPEC>[^ \t\r\n]+ {
    /*
     * String delimiter spec - like WORD, but allow any character
     */
    yylval->s = (char *) strdup (yytext);
    BEGIN(BOX);
    report_state("YDELIMS", yytext, "INITIAL");
    if (change_string_delimiters(yyextra, yylval->s) != 0) {
        return YUNREC;
    }
    return YDELIMSPEC;
}


<BOX,SHAPES>{SDELIM}.*$ {
    /*
     * Strings  --  first match everything starting from a potential string delimiter until the end of the line. We
     * will give back what we don't need and also detect unterminated strings. Strings always end on the same line.
     */
    char *p;
    int rest_len = yyleng - 1;           /* length of string pointed to by p */
    int qcnt = 0;                        /* esc char count in current string */

    if (yytext[0] != yyextra->sdel) {
        REJECT;                          /* that was not our delimiter */
    }

    yylval->s = (char *) strdup (yytext + 1);
    if (yylval->s == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    p = yylval->s;

    while (*p) {
        if (*p == yyextra->sesc) {
            memmove (p, p+1, rest_len);     /* incl. '\0' */
            ++qcnt;
            --rest_len;
            if (*p == '\0')
                break;
        }
        else if (*p == yyextra->sdel) {
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


<INITIAL>{PPARENT} {
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


<BOX>Sample {
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

<SAMPLE>ends[ \t\r]*$ {
    /* reached because the other rule pushes it back so a proper end token can be returned */
    BEGIN(BOX);
    report_state("YENDSAM", yytext, "BOX");
    return YENDSAMPLE;
}


<BOX>Tags {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "  YTAGS: %s\n", yytext);
    #endif
    return YTAGS;
}

<BOX>Elastic {
    BEGIN(ELASTIC);
    report_state("YELASTC", yytext, "ELASTIC");
    return YELASTIC;
}

<BOX>Shapes {
    BEGIN(SHAPES);
    report_state("YSHAPES", yytext, "SHAPES");
    return YSHAPES;
}

<INITIAL>Box {
    BEGIN(BOX);
    report_state("   YBOX", yytext, "BOX");
    yyextra->yyerrcnt = 0;
    change_string_delimiters(yyextra, "\\\"");
    return YBOX;
}

<BOX>Replace { return YREPLACE; }
<BOX>Reverse { return YREVERSE; }
<BOX>Padding { return YPADDING; }
<BOX>To      { return YTO;      }
<BOX>With    { return YWITH;    }
<BOX>Global  { yylval->c = 'g'; return YRXPFLAG; }
<BOX>Once    { yylval->c = 'o'; return YRXPFLAG; }
<BOX>End     {
    BEGIN(INITIAL);
    report_state("   YEND", yytext, "INITIAL");
    change_string_delimiters(yyextra, "\\\"");
    return YEND;
}


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
    BEGIN(BOX);
    report_state_char("SYMBOL", yytext[0], "BOX");
    return yytext[0];
}

<SHAPES>\} {
    BEGIN(BOX);
    report_state_char("SYMBOL", yytext[0], "BOX");
    return yytext[0];
}


<BOX>author|designer|created|revision|revdate|indent {
    /*
     * general key words
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


<BOX>Delimiter|Delim {
    /*
     * Change string delimiting characters
     */
    BEGIN(DELIMSPEC);
    report_state("YCHGDEL", yytext, "DELIMSPEC");
    return YCHGDEL;
}


<INITIAL,BOX>{PWORD} {
    /*
     * a free-floating word which is not a string, i.e. it does not have delimiting characters
     */
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


<BOX>[\+-]?[0-9]+ {
    #ifdef LEXER_DEBUG
        fprintf (stderr, "YNUMBER: %s\n", yytext);
    #endif
    yylval->num = atoi (yytext);
    return YNUMBER;
}


<BOX,SHAPES,ELASTIC>[,(){}] {
    #ifdef LEXER_DEBUG
        fprintf (stderr, " SYMBOL: \'%c\'\n", yytext[0]);
    #endif
    return yytext[0];
}


<INITIAL,BOX,SHAPES,ELASTIC>#.*$ {
    /* ignore comments */
    #ifdef LEXER_DEBUG
        fprintf (stderr, "COMMENT: %s\n", yytext+1);
    #endif
}


<INITIAL,BOX,SHAPES,ELASTIC>. {
    /* a character that made no sense where it was encountered. Let the parser handle it. */
    #ifdef LEXER_DEBUG
        fprintf (stderr, " YUNREC: \'%c\'\n", yytext[0]);
    #endif
    return YUNREC;
}


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



static void report_state_char(char *symbol, char c, char *expected_state_str)
{
    char *s = (char *) malloc(4);
    sprintf(s, "'%c'", c >= ' ' && c <= 126 ? c : '?');
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



static int change_string_delimiters(pass_to_flex *extra, char *delim_expr)
{
    if (strlen(delim_expr) != 2) {
        yyerror(NULL, "invalid string delimiter specification -- %s", delim_expr);
        return 1;
    }
    if (delim_expr[0] == delim_expr[1]) {
        yyerror(NULL, "string delimiter and escape char may not be the same");
        return 1;
    }
    if (strchr (LEX_SDELIM, delim_expr[1]) == NULL) {
        yyerror(NULL, "invalid string delimiter -- %c (try one of %s)", delim_expr[1], LEX_SDELIM_RECOMMENDED);
        return 1;
    }

    #ifdef LEXER_DEBUG
        fprintf(stderr, "YDELIMS: change_string_delimiters('%c', '%c')\n", delim_expr[0], delim_expr[1]);
    #endif
    extra->sesc = delim_expr[0];
    extra->sdel = delim_expr[1];

    return 0;
}


/*EOF*/                                         /* vim: set cindent sw=4: */
