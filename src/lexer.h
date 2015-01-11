/*
 *  File:             lexer.h
 *  Project Main:     boxes.c
 *  Date created:     July 01, 1999 (Thursday, 13:43h)
 *  Author:           Copyright (C) 1999 Thomas Jensen <boxes@thomasjensen.com>
 *  Language:         ANSI C
 *  Web Site:         http://boxes.thomasjensen.com/
 *  Purpose:          Export symbols used by the parser files only
 *
 *  License: o This program is free software; you can redistribute it and/or
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
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#ifndef LEXER_H
#define LEXER_H


/*
 *  Valid characters to be used as string delimiters. Note that the
 *  following list must correspond to the DELIM definition in lexer.l.
 */
#define LEX_SDELIM  "\"~'`!@%&*=:;<>?/|.\\"


int yylex();                             /* defined in lex.yy.c */

void begin_speedmode();

void chg_strdelims (const char asdel, const char asesc);

extern int speeding;


#endif /*LEXER_H*/

/*EOF*/                                                  /* vim: set sw=4: */
