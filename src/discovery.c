/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2021 Thomas Jensen and the boxes contributors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/*
 * Discovery of the configuration file.
 */

#include "config.h"
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __MINGW32__
    #include <windows.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "boxes.h"
#include "tools.h"
#include "discovery.h"



static int can_read_file(const char *filename)
{
    struct stat statbuf;
    int result = 1;
    if (filename == NULL || filename[0] == '\0') {
        #ifdef DEBUG
            fprintf(stderr, "%s: can_read_file(): argument was NULL\n", PROJECT);
        #endif
        result = 0;
    }
    else {
        FILE *f = fopen(filename, "r");
        if (f == NULL) {
            #ifdef DEBUG
                fprintf(stderr, "%s: can_read_file(): File \"%s\" could not be opened for reading - %s\n",
                    PROJECT, filename, strerror(errno));
            #endif
            result = 0;
        }
        else {
            fclose(f);

            if (stat(filename, &statbuf) != 0) {
                #ifdef DEBUG
                    fprintf(stderr, "%s: can_read_file(): File \"%s\" not statable - %s\n",
                        PROJECT, filename, strerror(errno));
                #endif
                result = 0;
            }
            else if (S_ISDIR(statbuf.st_mode)) {
                #ifdef DEBUG
                    fprintf(stderr, "%s: can_read_file(): File \"%s\" is in fact a directory\n", PROJECT, filename);
                #endif
                result = 0;
            }
        }
    }
    return result;
}



static int can_read_dir(const char *dirname)
{
    int result = 1;
    if (dirname == NULL || dirname[0] == '\0') {
        #ifdef DEBUG
            fprintf(stderr, "%s: can_read_dir(): argument was NULL\n", PROJECT);
        #endif
        result = 0;
    }
    else {
        DIR *dir = opendir(dirname);
        if (dir == NULL) {
            #ifdef DEBUG
                fprintf(stderr, "%s: can_read_dir(): Directory \"%s\" could not be opened for reading - %s\n",
                    PROJECT, dirname, strerror(errno));
            #endif
            result = 0;
        }
        else {
            closedir(dir);
        }
    }
    return result;
}



static char *combine(const char *dirname, const char *filename)
{
    const size_t dirname_len = strlen(dirname);

    if (dirname[dirname_len - 1] == '/') {
        return concat_strings_alloc(2, dirname, filename);
    }
    return concat_strings_alloc(3, dirname, "/", filename);
}



static char *locate_config_in_dir(const char *dirname)
{
    #ifdef __MINGW32__
    static const char *filenames[] = {"boxes.cfg", "box-designs.cfg", "boxes-config"};
    #else
    static const char *filenames[] = {".boxes", "box-designs", "boxes-config", "boxes"};
    #endif
    for (size_t i = 0; i < (sizeof(filenames) / sizeof(const char *)); i++) {
        char *f = combine(dirname, filenames[i]);
        if (can_read_file(f)) {
            return f;
        }
        BFREE(f);
    }
    return NULL;
}



static char *locate_config_file_or_dir(const char *path, const char *ext_msg)
{
    char *result = NULL;
    if (can_read_file(path)) {
        result = strdup(path);
    }
    else if (can_read_dir(path)) {
        result = locate_config_in_dir(path);
        if (result == NULL) {
            fprintf(stderr, "%s: Couldn\'t find config file in directory \'%s\'%s\n", PROJECT, path, ext_msg);
        }
    }
    else {
        fprintf(stderr, "%s: Couldn\'t find config file at \'%s\'%s\n", PROJECT, path, ext_msg);
    }
    return result;
}



static char *from_env_var(const char *env_var, const char *postfix)
{
    char *result = getenv(env_var);
    #ifdef DEBUG
        fprintf(stderr, "%s: from_env_var(): getenv(\"%s\") --> %s\n", PROJECT, env_var, result);
    #endif
    if (result != NULL) {
        result = concat_strings_alloc(2, result, postfix);
    }
    return result;
}



static char *locate_config_common(int *error_printed)
{
    char *result = NULL;
    if (opt.f) {
        result = locate_config_file_or_dir(opt.f, "");
        if (result == NULL) {
            *error_printed = 1;
        }
    }
    else if (getenv("BOXES")) {
        result = locate_config_file_or_dir(getenv("BOXES"), " from BOXES environment variable");
        if (result == NULL) {
            *error_printed = 1;
        }
    }
    return result;
}


#ifdef __MINGW32__

static char *exe_to_cfg()
{
    const char *fallback = "C:\\boxes.cfg";
    char *exepath = (char *) malloc(256);   /* for constructing config file path */
    if (GetModuleFileName(NULL, exepath, 255) != 0) {
        char *p = strrchr(exepath, '.') + 1;
        if (p) {
            /* p is always != NULL, because we get the full path */
            *p = '\0';
            if (strlen(exepath) < 253) {
                strcat(exepath, "cfg");       /* c:\blah\boxes.cfg */
            }
            else {
                fprintf(stderr, "%s: path too long. Using %s.\n", PROJECT, fallback);
                strcpy(exepath, fallback);
            }
        }
    }
    else {
        strcpy(exepath, fallback);
    }
    return exepath;
}

#endif


char *discover_config_file(const int global_only)
{
    int error_printed = global_only;
    char *result = NULL;
    if (!global_only) {
        result = locate_config_common(&error_printed);
    }

    if (result == NULL && !error_printed) {
        const char *globalconf_marker = "::GLOBALCONF::";
        const char *user_dirs[] = {
                from_env_var("HOME", ""),
                from_env_var("XDG_CONFIG_HOME", "/boxes"),
                from_env_var("HOME", "/.config/boxes")
        };
        const char *global_dirs[] = {
                globalconf_marker,
                "/etc/xdg/boxes",
                "/usr/local/share/boxes",
                "/usr/share/boxes"
        };
        const char *dirs[global_only ? 4 : 7];
        if (global_only) {
            memcpy(dirs, global_dirs, 4 * sizeof(char *));
        } else {
            memcpy(dirs, user_dirs, 3 * sizeof(char *));
            memcpy(dirs + 3, global_dirs, 4 * sizeof(char *));
        }
        for (size_t i = 0; i < (sizeof(dirs) / sizeof(const char *)); i++) {
            const char *dir = dirs[i];
            if (dir == globalconf_marker) {
                #ifdef __MINGW32__
                    char *exepath = exe_to_cfg();
                    if (can_read_file(exepath)) {
                        result = exepath;
                    } else {
                        fprintf(stderr, "%s: Couldn\'t find config file at \'%s\'\n", PROJECT, exepath);
                        error_printed = 1;
                    }
                #else
                    if (can_read_file(GLOBALCONF)) {
                        result = strdup(GLOBALCONF);
                    }
                #endif
            }
            else if (can_read_dir(dir)) {
                result = locate_config_in_dir(dir);
            }
            if (result != NULL) {
                break;
            }
        }
    }

    if (result == NULL && !error_printed) {
        fprintf(stderr, "%s: Can't find config file.\n", PROJECT);
    }
    return result;
}


/*EOF*/                                          /* vim: set cindent sw=4: */
