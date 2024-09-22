%top{
/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2024 Thomas Jensen and the boxes contributors
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License, version 3, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/*
 * flex lexical analyzer for boxes configuration files
 */

#include "config.h"
#include "bxstring.h"

typedef struct {
    int yyerrcnt;

    /** the currently active string delimiter character */
    char sdel;

    /** the currently active string escape character */
    char sesc;
} pass_to_flex;


/*
 *  Valid characters to be used as string delimiters.
 *  The following list must correspond to the SDELIM definition below.
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
void inflate_inbuf(void *yyscanner, const bxstr_t *configfile);

}

%{
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unitypes.h>

#include "boxes.h"
#include "logging.h"
#include "shape.h"
#include "tools.h"
#include "parsing.h"
#include "parser.h"
#include "unicode.h"


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

/*
 * The following paragraph contains patterns to recognize UTF-8 characters from a byte stream, based on
 * - https://stackoverflow.com/a/10253320/1005481 by Zack Weinberg (under CC-BY-SA 3.0 license)
 * - https://www.w3.org/2005/03/23-lex-U by Eric Prud'hommeaux, W3C (under the W3C Document License)
 */
PBOM      \xEF\xBB\xBF
U2A       [\xC2-\xDF][\x80-\xBF]
U2B       \xE0[\xA0-\xBF][\x80-\xBF]
U3A       [\xE1-\xEC\xEE\xEF][\x80-\xBF]{2}
U3B       \xED[\x80-\x9F][\x80-\xBF]
U4A       \xF0[\x90-\xBF][\x80-\xBF]{2}
U4B       [\xF1-\xF3][\x80-\xBF]{3}
U4C       \xF4[\x80-\x8F][\x80-\xBF]{2}
UTF_8     {U2A}|{U2B}|{U3A}|{U3B}|{U4A}|{U4B}|{U4C}

PWORD     (?:[a-zA-Z]|{UTF_8})(?:[a-zA-Z0-9_-]|{UTF_8})*
PASCII_ID [a-zA-Z][a-zA-Z0-9_-]*
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


<INITIAL,BOX,DELIMSPEC,ELASTIC,SHAPES>{PWHITE}|{PBOM}  /* ignore whitespace and a byte order mark */

<DELIMSPEC>[^ \t\r\n]+ {
    /*
     * String delimiter spec - like BXWORD, but allow any character
     */
    yylval->s = bxs_from_ascii("IGNORED");
    char *str = (char *) strdup(yytext);
    BEGIN(BOX);
    report_state("YDELIMS", str, "INITIAL");
    if (change_string_delimiters(yyextra, str) != 0) {
        return YUNREC;
    }
    return YDELIMSPEC;
}


<BOX,SHAPES>{SDELIM}.*$ {
    /*
     * Strings  --  first match everything starting from a potential string delimiter until the end of the line. We
     * will give back what we don't need and also detect unterminated strings. Strings always end on the same line.
     */
    int rest_len = yyleng - 1;           /* length of string pointed to by p */
    int qcnt = 0;                        /* esc char count in current string */

    if (yytext[0] != yyextra->sdel) {
        REJECT;                          /* that was not our delimiter */
    }

    char *str = (char *) strdup(yytext + 1);
    if (str == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    char *p = str;

    while (*p) {
        if (*p == yyextra->sesc) {
            memmove (p, p+1, rest_len);     /* incl. '\0' */
            ++qcnt;
            --rest_len;
            if (*p == '\0') {
                break;
            }
        }
        else if (*p == yyextra->sdel) {
            *p = '\0';
            yyless ((p - str) + 2 + qcnt);   /* string plus quotes */
            log_debug(__FILE__, LEXER, " STRING: \"%s\"\n", str);

            uint32_t *utf8 = u32_strconv_from_arg(str, CONFIG_FILE_ENCODING);
            yylval->s = bxs_from_unicode(utf8);
            BFREE(utf8);
            BFREE(str);
            return STRING;
        }
        --rest_len;
        ++p;
    }
    if ((yyextra->yyerrcnt)++ < 5) {
        yyerror(NULL, "Unterminated String -- %s", yytext);
    }
    BFREE(str);
    return YUNREC;
}


<INITIAL>{PPARENT} {
    BEGIN(PARENT);
    report_state("YPARENT", yytext, "PARENT");
    return YPARENT;
}

<PARENT>{PFILENAME} {
    uint32_t *utf8 = u32_strconv_from_arg(yytext, CONFIG_FILE_ENCODING);
    bxstr_t *bxstr = bxs_from_unicode(utf8);
    yylval->s = bxs_trim(bxstr);

    BFREE(utf8);
    bxs_free(bxstr);

    BEGIN(INITIAL);
    report_state("FILENAM", bxs_to_output(yylval->s), "INITIAL");
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
    char *sample = (char *) strdup(yytext);
    if (sample == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    *p-- = 'n';

    len = p - yytext;                    /* yyless(n): push back all but the first n */
    yyless (len);                        /* allow the lexer to return YENDSAMPLE */

    sample[len] = '\n';                  /* replace 'e' with newline */
    btrim(sample, &len);
    if (len > 0) {
        uint32_t *utf8 = u32_strconv_from_arg(sample, CONFIG_FILE_ENCODING);
        uint32_t *nl = u32_strconv_from_arg("\n", CONFIG_FILE_ENCODING);
        bxstr_t *bxstr = bxs_from_unicode(utf8);
        bxstr_t *bxstr2 = bxs_rtrim(bxstr);
        bxs_free(bxstr);
        bxstr = bxs_strcat(bxstr2, nl);
        BFREE(nl);
        BFREE(utf8);
        BFREE(sample);
        bxs_free(bxstr2);
        yylval->s = bxstr;
        return STRING;
    }
    else {
        if ((yyextra->yyerrcnt)++ < 5) {
            yyerror(NULL, "SAMPLE block must not be empty");
        }
        BFREE(sample);
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
    log_debug(__FILE__, LEXER, "  YTAGS: %s\n", yytext);
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


<BOX>author|designer|indent {
    /*
     * general key words
     */
    log_debug(__FILE__, LEXER, "KEYWORD: %s\n", yytext);
    yylval->ascii = strdup(yytext);
    if (yylval->ascii == NULL) {
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

<INITIAL,BOX>{PASCII_ID} {
    /*
     * a free-floating word which is not a string, i.e. it does not have delimiting characters (ASCII version)
     */
    yylval->ascii = strdup(yytext);
    if (yylval->ascii == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    log_debug(__FILE__, LEXER, "ASCIIID: %s\n", yylval->ascii);
    return ASCII_ID;
}

<INITIAL,BOX>{PWORD} {
    /*
     * a free-floating word which is not a string, i.e. it does not have delimiting characters
     */
    uint32_t *utf8 = u32_strconv_from_arg(yytext, CONFIG_FILE_ENCODING);
    yylval->s = bxs_from_unicode(utf8);
    if (yylval->s == NULL) {
        perror (PROJECT);
        exit (EXIT_FAILURE);
    }
    if (is_debug_logging(LEXER)) {
        char *out_utf8 = u32_strconv_to_output(utf8);
        log_debug(__FILE__, LEXER, " BXWORD: %s\n", out_utf8);
        BFREE(out_utf8);
    }
    BFREE(utf8);
    return BXWORD;
}


<BOX>[\+-]?[0-9]+ {
    log_debug(__FILE__, LEXER, "YNUMBER: %s\n", yytext);
    yylval->num = atoi (yytext);
    return YNUMBER;
}


<BOX,SHAPES,ELASTIC>[,(){}] {
    log_debug(__FILE__, LEXER, " SYMBOL: \'%c\'\n", yytext[0]);
    return yytext[0];
}


<INITIAL,BOX,SHAPES,ELASTIC>#.*$ {
    /* ignore comments */
    log_debug(__FILE__, LEXER, "COMMENT: %s\n", yytext + 1);
}


<INITIAL,BOX,SHAPES,ELASTIC>. {
    /* a character that made no sense where it was encountered. Let the parser handle it. */
    log_debug(__FILE__, LEXER, " YUNREC: \'%c\'\n", yytext[0]);
    return YUNREC;
}


%%


void inflate_inbuf(void *yyscanner, const bxstr_t *configfile)
{
    struct stat sinf;

    char *utf8 = u32_strconv_to_arg(configfile->memory, "UTF-8");
    if (stat(utf8, &sinf)) {
        perror (PROJECT);
        BFREE(utf8);
        exit (EXIT_FAILURE);
    }
    BFREE(utf8);
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
    if (is_debug_logging(LEXER)) {
        size_t text_len = 0;
        if (text != NULL) {
            text_len = strlen(text);
            if (text_len > 0 && (text[text_len - 1] == '\r' || text[text_len - 1] == '\t')) {
                text[text_len - 1] = '\0';
            }
        }
        log_debug(__FILE__, LEXER, "%7s: %s (%d) -- STATE %s\n", symbol, text, text_len, expected_state_str);
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

    log_debug(__FILE__, LEXER, "YDELIMS: change_string_delimiters('%c', '%c')\n", delim_expr[0], delim_expr[1]);
    extra->sesc = delim_expr[0];
    extra->sdel = delim_expr[1];

    return 0;
}


/* vim: set cindent sw=4: */
