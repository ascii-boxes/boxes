/*
 *  File:             shape.h
 *  Project Main:     boxes.c
 *  Date created:     June 23, 1999 (Wednesday, 13:45h)
 *  Author:           Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id$
 *  Language:         ANSI C
 *  Purpose:          Shape handling and information functions
 *  Remarks:          ---
 *
 *  Revision History:
 *
 *    $Log$
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#ifndef SHAPE_H
#define SHAPE_H


typedef enum {
    NW, NNW, N, NNE, NE, ENE, E, ESE, SE, SSE, S, SSW, SW, WSW, W, WNW
} shape_t;

extern char *shape_name[];

#define ANZ_SHAPES 16

#define SHAPES_PER_SIDE 5
#define ANZ_SIDES       4
#define ANZ_CORNERS     4

extern shape_t north_side[SHAPES_PER_SIDE];  /* groups of shapes, clockwise */
extern shape_t  east_side[SHAPES_PER_SIDE];
extern shape_t south_side[SHAPES_PER_SIDE];
extern shape_t  west_side[SHAPES_PER_SIDE];
extern shape_t corners[ANZ_CORNERS];
extern shape_t *sides[ANZ_SIDES];


typedef struct {
    char **chars;
    size_t height;
    size_t width;
    int    elastic;          /* elastic is used only in orginial definition */
} sentry_t;

#define SENTRY_INITIALIZER (sentry_t) {NULL, 0, 0, 0}



int iscorner (const shape_t s);

shape_t *on_side (const shape_t s, const int idx);

int isempty (const sentry_t *shape);

int shapecmp (const sentry_t *shape1, const sentry_t *shape2);

shape_t *both_on_side (const shape_t shape1, const shape_t shape2);

int shape_distance (const shape_t s1, const shape_t s2);

size_t highest (const sentry_t *sarr, const int n, ...);
size_t widest (const sentry_t *sarr, const int n, ...);

shape_t leftmost (const int aside, const int cnt);



#endif /*SHAPE_H*/

/*EOF*/                                          /* vim: set cindent sw=4: */
