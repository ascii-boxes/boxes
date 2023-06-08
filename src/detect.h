/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2023 Thomas Jensen and the boxes contributors
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
 * Autodetect design used by box in input.
 */

#ifndef BOXES_DETECT_H
#define BOXES_DETECT_H

#include "boxes.h"


/**
 * Autodetect design used by box in input.
 * This requires knowledge about ALL designs, so the entire config file had to be parsed at some earlier time.
 * @return != NULL: success, pointer to detected design;
 *         == NULL: on error
 */
design_t *autodetect_design();


#endif /* BOXES_DETECT_H */


/* vim: set cindent sw=4: */
