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
 *
 * Debug output is printed on stderr. Each message is annotated with its module and "area".
 * The "area" defines certain sections of code as a possible filter criterion.
 */

#ifndef LOGGING_H
#define LOGGING_H


/** Enum of the log areas supported. `ALL` means all of them, `RESERVED` cannot be used. */
typedef enum _log_area_t {
    /** the 0 value is reserved and must not be used */
    RESERVED,

    /** all areas */
    ALL,

    /** the main areas of the code (activate this if unsure) */
    MAIN,

    /** regular expression handling */
    REGEXP,

    /** parser code */
    PARSER,

    /** lexer code */
    LEXER,

    /** config file discovery */
    DISCOVERY
} log_area_t;

/** names of the values of `log_area_t` as strings for parsing */
extern char *log_area_names[];

/** number of log areas in `log_area_t`, excluding `ALL` and `RESERVED` */
#define NUM_LOG_AREAS 5


/**
 * Activate debug logging.
 * @param log_areas an array of log areas to activate, zero-terminated, should not be empty. The value will by copied
 *      and can be freed after returning from the function. Passing NULL will turn off debug logging.
 */
void activate_debug_logging(int *log_areas);


/**
 * Determine if debug logging was activated for any debug area.
 * @return 1 if active, 0 if not
 */
int is_debug_activated();


/**
 * Determine if debug logging is active and the given area is active, too.
 * @param log_area the log area to check
 * @return 1 if active, 0 if not
 */
int is_debug_logging(log_area_t log_area);


/**
 * Print a debug log message if the log area is active. Debug log messages will be printed on `stderr`.
 * @param module the module name as per `__FILE__`
 * @param log_area_t log_area
 * @param format format string for the message, followed by the placeholder arguments
 */
void log_debug(const char *module, log_area_t log_area, const char *format, ...);


/**
 * Print a debug log message if the log area is active. Debug log messages will be printed on `stderr`.
 * The difference to `log_debug()` is that no module is printed.
 * @param log_area_t log_area
 * @param format format string for the message, followed by the placeholder arguments
 */
void log_debug_cont(log_area_t log_area, const char *format, ...);


#endif

/* vim: set cindent sw=4: */
