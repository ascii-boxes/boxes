/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2024 Thomas Jensen and the boxes contributors
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
 * Please compiler and ease porting
 */

#ifndef CONFIG_H
#define CONFIG_H


#if defined(__linux__) || defined(__GLIBC__) || defined(__GNU__)

#define _GNU_SOURCE

#elif defined(__APPLE__)

#define _DARWIN_C_SOURCE

#elif defined(__sun__)

#define _POSIX_SOURCE
#define __EXTENSIONS__

#elif defined(__FreeBSD__)
    /* do nothing */

#elif defined(__MINGW32__)

#define __MISC_VISIBLE 1

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

#else

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

#endif


#endif /*CONFIG_H*/
