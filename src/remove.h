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
 * Box removal, i.e. the deletion of boxes
 */

#ifndef REMOVE_H
#define REMOVE_H


/**
 * Remove box from input.
 * @return == 0: success;
 *         \!= 0:  error
 */
int remove_box();


/**
 * Output contents of input line list "as is" to standard output, except for removal of trailing spaces (trimming).
 * The trimming is performed on the actual input lines, modifying them.
 * @param trim_only only perform the trimming of trailing spaces, but do not output anything
 */
void output_input(const int trim_only);


#endif /*REMOVE_H*/

/*EOF*/                                          /* vim: set cindent sw=4: */
