/*
 *  File:             tools.h
 *  Project Main:     boxes.c
 *  Date created:     June 20, 1999 (Sunday, 16:57h)
 *  Author:           Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id$
 *  Language:         ANSI C
 *  Purpose:          Tool functions for error reporting and some string
 *                    handling and other needful things
 *  Remarks:          ---
 *
 *  Revision History:
 *
 *    $Log$
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#ifndef TOOLS_H
#define TOOLS_H

#include "boxes.h"


#define BMAX(a,b) ((a)>(b)? (a):(b))     /* return the larger value */

#define BFREE(p) {                       /* free memory and clear pointer */ \
   if (p) {           \
      free (p);       \
      (p) = NULL;     \
   }                  \
}


int    yyerror     (const char *fmt, ...);
void   regerror    (char *msg);
int    empty_line  (const line_t *line);
size_t expand_tabs_into (const char *input_buffer, const int in_len,
                    const int tabstop, char **text);
void   btrim       (char *text, size_t *len);
char*  my_strnrstr (const char *s1, const char *s2, const size_t s2_len,
                    int skip);

#endif

/*EOF*/                                          /* vim: set cindent sw=4: */
