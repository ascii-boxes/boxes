/*
 *  File:             generate.h
 *  Project Main:     boxes.c
 *  Date created:     June 23, 1999 (Wednesday, 20:12h)
 *  Author:           Copyright (C) 1999 Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: generate.h,v 1.2 1999/06/25 18:45:12 tsjensen Exp tsjensen $
 *  Language:         ANSI C
 *  Purpose:          Box generation, i.e. the drawing of boxes
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
 *    $Log: generate.h,v $
 *    Revision 1.2  1999/06/25 18:45:12  tsjensen
 *    Added output_box() function
 *
 *    Revision 1.1  1999/06/23 18:53:03  tsjensen
 *    Initial revision
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#ifndef GENERATE_H
#define GENERATE_H


int generate_box (sentry_t *thebox);
int output_box (const sentry_t *thebox);


#endif /*GENERATE_H*/

/*EOF*/                                          /* vim: set cindent sw=4: */
