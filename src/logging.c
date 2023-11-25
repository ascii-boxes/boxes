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
 * Simple makeshift log facility for handling debug output.
 */

#include "config.h"

#include <stdarg.h>
#include <string.h>

#include "regulex.h"
#include "tools.h"
#include "logging.h"


char *log_area_names[] = { "RESERVED", "ALL", "MAIN", "REGEXP", "PARSER", "LEXER", "DISCOVERY" };

static int debug_logging_active = 0;

static int *areas_active = NULL;



static char *shorten_module(const char *module)
{
    if (module == NULL) {
        return strdup("NULL");
    }

    const char *s = strrchr(module, '/');
    if (s == NULL) {
        s = strrchr(module, '\\');
    }
    if (s != NULL) {
        s++;
    }
    else {
        s = module;
    }

    char *e = strrchr(module, '.');
    if (e != NULL && e > s && *s != '\0') {
        char *result = strdup(s);
        result[e-s] = '\0';
        return result;
    }
    return strdup(s);
}



void activate_debug_logging(int *log_areas)
{
    debug_logging_active = 0;

    if (areas_active != NULL) {
        BFREE(areas_active);
    }
    areas_active = (int *) calloc(NUM_LOG_AREAS, sizeof(int));
    if (log_areas != NULL) {
        memcpy(areas_active, log_areas, NUM_LOG_AREAS * sizeof(int));
        debug_logging_active = 1;
    }
}



static int is_area_active(log_area_t log_area)
{
    if (debug_logging_active && areas_active != NULL && log_area != RESERVED && log_area < NUM_LOG_AREAS + 2) {
        if (log_area == ALL) {
            for (size_t i = 0; i < NUM_LOG_AREAS; i++) {
                if (!areas_active[i]) {
                    return 0;
                }
            }
            return 1;
        }
        return areas_active[log_area - 2];
    }
    return 0;
}



int is_debug_logging(log_area_t log_area)
{
    return (debug_logging_active && is_area_active(log_area)) ? 1 : 0;
}



int is_debug_activated()
{
    if (debug_logging_active) {
        for (size_t i = 0; i < NUM_LOG_AREAS; i++) {
            if (areas_active[i]) {
                return 1;
            }
        }
    }
    return 0;
}



void log_debug(const char *module, log_area_t log_area, const char *format, ...)
{
    if (is_debug_logging(log_area)) {
        char *msg = (char *) malloc(1024);
        va_list va;
        va_start(va, format);
        vsprintf(msg, format, va);
        va_end(va);

        char *short_module = shorten_module(module);
        bx_fprintf(stderr, "[%-9s] %s", short_module, msg);
        BFREE(short_module);
        BFREE(msg);
    }
}



void log_debug_cont(log_area_t log_area, const char *format, ...)
{
    if (is_debug_logging(log_area)) {
        char *msg = (char *) malloc(1024);
        va_list va;
        va_start(va, format);
        vsprintf(msg, format, va);
        va_end(va);

        bx_fprintf(stderr, "%s", msg);
        BFREE(msg);
    }
}


/* vim: set cindent sw=4: */
