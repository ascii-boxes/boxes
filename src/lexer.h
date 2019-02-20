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
 * Export symbols used by the parser files only
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
