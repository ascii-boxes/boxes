/*
 *  File:             generate.h
 *  Project Main:     boxes.c
 *  Date created:     June 23, 1999 (Wednesday, 20:12h)
 *  Author:           Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: generate.h,v 1.1 1999/06/23 18:53:03 tsjensen Exp tsjensen $
 *  Language:         ANSI C
 *  Purpose:          Box generation, i.e. the drawing of boxes
 *  Remarks:          ---
 *
 *  Revision History:
 *
 *    $Log: generate.h,v $
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
