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
 * Provide tool functions for error reporting and some string handling
 */

#include "config.h"
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "shape.h"
#include "boxes.h"
#include "tools.h"



int yyerror (const char *fmt, ...)
/*
 *  Print configuration file parser errors.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    va_list ap;

    va_start (ap, fmt);

    fprintf  (stderr, "%s: %s: line %d: ", PROJECT,
              yyfilename? yyfilename: "(null)", tjlineno);
    vfprintf (stderr, fmt, ap);
    fputc    ('\n', stderr);

    va_end (ap);

    return 0;
}



void regerror (char *msg)
/*
 *  Print regular expression andling error messages
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    fprintf (stderr, "%s: %s: line %d: %s\n",
            PROJECT, yyfilename? yyfilename: "(null)",
            opt.design->current_rule? opt.design->current_rule->line: 0,
            msg);
    errno = EINVAL;
}



int strisyes (const char *s)
/*
 *  Determine if the string s has a contents indicating "yes".
 *
 *     s   string to examine
 *
 *  RETURNS:  == 0  string does NOT indicate yes (including errors)
 *            != 0  string indicates yes
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    if (s == NULL)
        return 0;

    if (!strncasecmp ("on", s, 3))
        return 1;
    else if (!strncasecmp ("yes", s, 4))
        return 1;
    else if (!strncasecmp ("true", s, 5))
        return 1;
    else if (!strncmp ("1", s, 2))
        return 1;
    else if (!strncasecmp ("t", s, 2))
        return 1;
    else
        return 0;
}



int strisno (const char *s)
/*
 *  Determine if the string s has a contents indicating "no".
 *
 *     s   string to examine
 *
 *  RETURNS:  == 0  string does NOT indicate no (including errors)
 *            != 0  string indicates no
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    if (s == NULL)
        return 0;

    if (!strncasecmp ("off", s, 4))
        return 1;
    else if (!strncasecmp ("no", s, 3))
        return 1;
    else if (!strncasecmp ("false", s, 6))
        return 1;
    else if (!strncmp ("0", s, 2))
        return 1;
    else if (!strncasecmp ("f", s, 2))
        return 1;
    else
        return 0;
}



void concat_strings (char *dst, int max_len, int count, ...)
/*
 *  Concatenate a variable number of strings into a fixed-length buffer.
 *
 *      dst     Destination array
 *      max_len Maximum resulting string length (including terminating NULL).
 *      count   Number of source strings.
 * 
 *  The concatenation process terminates when either the destination 
 *  buffer is full or all 'count' strings are processed.  Null string
 *  pointers are treated as empty strings.
 * 
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    va_list     va;
    const char *src;

    va_start (va, count);

    /*
     *  Sanity check. 
     */
    if (max_len < 1)
        return;

    if (max_len == 1 || count < 1) {
        *dst = '\0';
        return;
    }

    /*
     *  Loop over all input strings.
     */
    while (count-->0 && max_len > 1) {

        /*
         * Grab an input string pointer.  If it's NULL, skip it (eg. treat
         * it as empty.
         */
        src = va_arg (va, const char *);

        if (src == NULL) 
            continue;

        /* 
         * Concatenate 'src' onto 'dst', as long as we have room.
         */
        while (*src && max_len > 1) {
            *dst++ = *src++;
            max_len--;
        }
    }

    va_end (va);

    /*
     * Terminate the string with an ASCII NUL.
     */
    *dst = '\0';
}



int empty_line (const line_t *line)
/*
 *  Return true if line is empty.
 *
 *  Empty lines either consist entirely of whitespace or don't exist.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char *p;
    size_t j;

    if (!line)
        return 1;
    if (line->text == NULL || line->len <= 0)
        return 1;

    for (p=line->text, j=0; *p && j<line->len; ++j, ++p) {
        if (*p != ' ' && *p != '\t' && *p != '\r')
            return 0;
    }
    return 1;
}



size_t expand_tabs_into (const char *input_buffer, const size_t in_len,
      const int tabstop, char **text, size_t **tabpos, size_t *tabpos_len)
/*
 *  Expand tab chars in input_buffer and store result in text.
 *
 *  input_buffer   Line of text with tab chars
 *  in_len         length of the string in input_buffer
 *  tabstop        tab stop distance
 *  text           address of the pointer that will take the result
 *  tabpos         array of ints giving the positions of the first
 *                 space of an expanded tab in the text result buffer
 *  tabpos_len     number of tabs recorded in tabpos
 *
 *  Memory will be allocated for text and tabpos.
 *  Should only be called for lines of length > 0;
 *
 *  RETURNS:  Success: Length of the result line in characters (> 0)
 *            Error:   0       (e.g. out of memory)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
   static char temp [LINE_MAX*MAX_TABSTOP+1];  /* work string */
   size_t ii;                            /* position in input string */
   size_t io;                            /* position in work string */
   size_t jp;                            /* tab expansion jump point */
   size_t tabnum;                        /* number of tabs in input */

   *text = NULL;

   for (ii=0, *tabpos_len=0; ii<in_len; ++ii) {
      if (input_buffer[ii] == '\t')
         (*tabpos_len)++;
   }
   if (opt.tabexp != 'k')
      *tabpos_len = 0;
   if (*tabpos_len > 0) {
      *tabpos = (size_t *) calloc ((*tabpos_len) + 1, sizeof(size_t));
      if (*tabpos == NULL) {
          return 0;       /* out of memory */
      }
   }

   for (ii=0, io=0, tabnum=0; ii < in_len && ((int) io) < (LINE_MAX*tabstop-1); ++ii) {
      if (input_buffer[ii] == '\t') {
         if (*tabpos_len > 0) {
            (*tabpos)[tabnum++] = io;
         }
         for (jp=io+tabstop-(io%tabstop); io<jp; ++io)
            temp[io] = ' ';
      }
      else {
         temp[io] = input_buffer[ii];
         ++io;
      }
   }
   temp[io] = '\0';

   *text = (char *) strdup (temp);
   if (*text == NULL) return 0;

   return io;
}



void btrim (char *text, size_t *len)
/*
 *  Remove trailing whitespace from line.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    long idx = (long) (*len) - 1;

    while (idx >= 0 && (text[idx] == '\n' || text[idx] == '\r'
                     || text[idx] == '\t' || text[idx] == ' '))
    {
        text[idx--] = '\0';
    }

    *len = idx + 1;
}



char *my_strnrstr (const char *s1, const char *s2, const size_t s2_len, int skip)
/*
 *  Return pointer to last occurrence of string s2 in string s1.
 *
 *      s1       string to search
 *      s2       string to search for in s1
 *      s2_len   length in characters of s2
 *      skip     number of finds to ignore before returning anything
 *
 *  RETURNS: pointer to last occurrence of string s2 in string s1
 *           NULL if not found or error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char *p;
    int comp;

    if (!s2 || *s2 == '\0')
        return (char *) s1;
    if (!s1 || *s1 == '\0')
        return NULL;
    if (skip < 0)
        skip = 0;

    p = strrchr (s1, s2[0]);
    if (!p)
        return NULL;

    while (p >= s1) {
        comp = strncmp (p, s2, s2_len);
        if (comp == 0) {
            if (skip--)
                --p;
            else
                return p;
        }
        else {
            --p;
        }
    }

    return NULL;
}



char *tabbify_indent (const size_t lineno, char *indentspc, const size_t indentspc_len)
/*
 *  Checks if tab expansion mode is "keep", and if so, calculates a new
 *  indentation string based on the one given. The new string contains
 *  tabs in their original positions.
 *
 *    lineno         index of the input line we are referring to
 *    indentspc      previously calculated "space-only" indentation string
 *                   (may be NULL). This is only used when opt.tabexp != 'k',
 *                   in which case it will be used as the function result.
 *    indentspc_len  desired result length, measured in spaces only
 *
 *  RETURNS:  if successful and opt.tabexp == 'k': new string
 *            on error (invalid input or out of memory): NULL
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t i;
    char  *result;
    size_t result_len;

    if (opt.tabexp != 'k') {
        return indentspc;
    }
    if (lineno >= input.anz_lines) {
        return NULL;
    }
    if (indentspc_len == 0) {
        return (char *) strdup ("");
    }

    result = (char *) malloc (indentspc_len + 1);
    if (result == NULL) {
        perror (PROJECT);
        return NULL;
    }
    memset (result, (int)' ', indentspc_len);
    result[indentspc_len] = '\0';
    result_len = indentspc_len;
    
    for (i=0; i < input.lines[lineno].tabpos_len
            && input.lines[lineno].tabpos[i] < indentspc_len; ++i)
    {
        size_t tpos = input.lines[lineno].tabpos[i];
        size_t nspc = opt.tabstop - (tpos % opt.tabstop);   /* no of spcs covered by tab */
        if (tpos + nspc > input.indent) {
            break;
        }
        result[tpos] = '\t';
        result_len -= nspc - 1;
        result[result_len] = '\0';
    }

    return result;
}



/*EOF*/                                                  /* vim: set sw=4: */
