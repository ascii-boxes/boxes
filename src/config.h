/*
 *  File:             config.h
 *  Project Main:     boxes.c
 *  Date created:     July 4, 1999 (Sunday, 21:04h)
 *  Author:           Copyright (C) 1999 Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: config.h,v 1.2 1999/08/18 18:23:06 tsjensen Exp tsjensen $
 *  Language:         ANSI C
 *  Purpose:          Please compiler and ease porting
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
 *    $Log: config.h,v $
 *    Revision 1.2  1999/08/18 18:23:06  tsjensen
 *    Added Linux defines. Now compiles nicely on (my) Linux.
 *
 *    Revision 1.1  1999/07/22 12:26:10  tsjensen
 *    Initial revision
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#ifndef CONFIG_H
#define CONFIG_H


#if defined(__linux__)

#define _GNU_SOURCE

#elif defined(__sun__)

#define _POSIX_SOURCE
#define __EXTENSIONS__

#elif defined(__FreeBSD__)
    /* do nothing */
#else

#define _POSIX_SOURCE

#endif


#endif /*CONFIG_H*/


/*EOF*/
