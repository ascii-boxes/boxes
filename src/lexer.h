/*
 *  File:             lexer.h
 *  Project Main:     boxes.c
 *  Date created:     July 01, 1999 (Thursday, 13:43h)
 *  Author:           Copyright (C) 1999 Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: lexer.h,v 1.2 1999/07/03 16:12:19 tsjensen Exp tsjensen $
 *  Language:         ANSI C
 *  Purpose:          Export symbols used by the parser files only
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
 *
 *  Revision History:
 *
 *    $Log: lexer.h,v $
 *    Revision 1.2  1999/07/03 16:12:19  tsjensen
 *    Renamed file from parser.h to lexer.h
 *
 *    Revision 1.1  1999/07/02 11:47:11  tsjensen
 *    Initial revision
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#ifndef LEXER_H
#define LEXER_H


int yylex();                             /* defined in lex.yy.c */

void begin_speedmode();

extern int speeding;


#endif /*LEXER_H*/

/*EOF*/                                                  /* vim: set sw=4: */
