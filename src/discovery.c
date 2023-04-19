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
#include "unicode.h"
#include "discovery.h"



static int can_read_file(char *filename_utf8)
{
    #ifdef DISCOVERY_DEBUG
        fprintf(stderr, "can_read_file(%s) - enter\n", filename_utf8);
    #endif

    struct stat statbuf;
    int result = 1;
    if (filename_utf8 == NULL || filename_utf8[0] == '\0') {
        #ifdef DISCOVERY_DEBUG
            fprintf(stderr, "%s: can_read_file(): argument was NULL\n", PROJECT);
        #endif
        result = 0;
    }
    else {
        FILE *f = bx_fopen(filename_utf8, "r");
        if (f == NULL) {
            #ifdef DISCOVERY_DEBUG
                fprintf(stderr, "%s: can_read_file(): File \"%s\" could not be opened for reading - %s\n",
                    PROJECT, filename_utf8, strerror(errno));
            #endif
            result = 0;
        }
        else {
            fclose(f);

            if (stat(filename_utf8, &statbuf) != 0) {
                #ifdef DISCOVERY_DEBUG
                    fprintf(stderr, "%s: can_read_file(): File \"%s\" not statable - %s\n",
                        PROJECT, filename_utf8, strerror(errno));
                #endif
                result = 0;
            }
            else if (S_ISDIR(statbuf.st_mode)) {
                #ifdef DISCOVERY_DEBUG
                    fprintf(stderr, "%s: can_read_file(): File \"%s\" is in fact a directory\n", PROJECT,
                            filename_utf8);
                #endif
                result = 0;
            }
        }
    }
    #ifdef DISCOVERY_DEBUG
        fprintf(stderr, "can_read_file() - exit -> %s\n", result ? "true" : "false");
    #endif
    return result;
}



static int can_read_dir(const char *dirname_utf8)
{
    #ifdef DISCOVERY_DEBUG
        fprintf(stderr, "can_read_dir(%s) - enter\n", dirname_utf8);
    #endif

    int result = 1;
    if (dirname_utf8 == NULL || dirname_utf8[0] == '\0') {
        #ifdef DISCOVERY_DEBUG
            fprintf(stderr, "%s: can_read_dir(): argument was NULL\n", PROJECT);
        #endif
        result = 0;
    }
    else {
        DIR *dir = opendir(dirname_utf8);
        if (dir == NULL) {
            #ifdef DISCOVERY_DEBUG
                fprintf(stderr, "%s: can_read_dir(): Directory \"%s\" could not be opened for reading - %s\n",
                    PROJECT, dirname_utf8, strerror(errno));
            #endif
            result = 0;
        }
        else {
            closedir(dir);
        }
    }
    #ifdef DISCOVERY_DEBUG
        fprintf(stderr, "can_read_dir() - exit -> %s\n", result ? "true" : "false");
    #endif
    return result;
}



static char *combine(const char *dirname_utf8, const char *filename_utf8)
{
    const size_t dirname_len = strlen(dirname_utf8);

    if (dirname_utf8[dirname_len - 1] == '/') {
        return concat_strings_alloc(2, dirname_utf8, filename_utf8);
    }
    return concat_strings_alloc(3, dirname_utf8, "/", filename_utf8);
}



static char *locate_config_in_dir(const char *dirname_utf8)
{
    #ifdef __MINGW32__
    static const char *filenames[] = {"boxes.cfg", "box-designs.cfg", "boxes-config"};
    #else
    static const char *filenames[] = {".boxes", "box-designs", "boxes-config", "boxes"};
    #endif
    for (size_t i = 0; i < (sizeof(filenames) / sizeof(const char *)); i++) {
        char *f = combine(dirname_utf8, filenames[i]);
        if (can_read_file(f)) {
            return f;
        }
        BFREE(f);
    }
    return NULL;
}



static char *locate_config_file_or_dir(char *path_utf8, const char *ext_msg)
{
    char *result = NULL;
    if (can_read_file(path_utf8)) {
        result = strdup(path_utf8);
    }
    else if (can_read_dir(path_utf8)) {
        result = locate_config_in_dir(path_utf8);
        if (result == NULL) {
            fprintf(stderr, "%s: Couldn\'t find config file in directory \'%s\'%s\n", PROJECT, path_utf8, ext_msg);
        }
    }
    else {
        fprintf(stderr, "%s: Couldn\'t find config file at \'%s\'%s\n", PROJECT, path_utf8, ext_msg);
    }
    return result;
}



static char *from_env_var(const char *env_var, const char *postfix)
{
    char *result = getenv(env_var);
    #ifdef DISCOVERY_DEBUG
        fprintf(stderr, "%s: from_env_var(): getenv(\"%s\") --> %s\n", PROJECT, env_var, result);
    #endif
    if (result != NULL) {
        result = concat_strings_alloc(2, result, postfix);
    }
    return result;
}



static char *locate_config_common(int *error_printed)
{
    #ifdef DISCOVERY_DEBUG
        fprintf(stderr, "locate_config_common() - enter\n");
    #endif

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

    #ifdef DISCOVERY_DEBUG
        fprintf(stderr, "locate_config_common() - exit -> [%s]\n", result);
    #endif
    return result;
}


#ifdef __MINGW32__

static char *exe_to_cfg()
{
    #ifdef DISCOVERY_DEBUG
        fprintf(stderr, "exe_to_cfg() - enter\n");
    #endif

    const char *fallback = "C:\\boxes.cfg";
    char *exepath = (char *) malloc(256);   /* for constructing config file path */
    if (GetModuleFileName(NULL, exepath, 255) != 0) {
        #ifdef DISCOVERY_DEBUG
            fprintf(stderr, "exe_to_cfg() - Found executable at %s\n", exepath);
        #endif
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
    #ifdef DISCOVERY_DEBUG
        fprintf(stderr, "exe_to_cfg() - exit -> [%s]\n", exepath);
    #endif
    return exepath;
}

#endif



static bxstr_t *utf8_to_bxs(char *utf8)
{
    bxstr_t *result = NULL;
    if (utf8 != NULL) {
        uint32_t *utf32 = u32_strconv_from_arg(utf8, "UTF-8");
        result = bxs_from_unicode(utf32);
        BFREE(utf32);
    }
    return result;
}



bxstr_t *discover_config_file(const int global_only)
{
    #ifdef DISCOVERY_DEBUG
        fprintf(stderr, "discover_config_file(%s) - enter\n", global_only ? "true" : "false");
    #endif
    int error_printed = 0;
    bxstr_t *result = NULL;
    if (!global_only) {
        char *common_config = locate_config_common(&error_printed);
        result = utf8_to_bxs(common_config);
        BFREE(common_config);
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
                        result = utf8_to_bxs(exepath);
                    } else {
                        fprintf(stderr, "%s: Couldn\'t find config file at \'%s\'\n", PROJECT, exepath);
                        error_printed = 1;
                    }
                    BFREE(exepath);
                #else
                    if (can_read_file(GLOBALCONF)) {
                        result = utf8_to_bxs(GLOBALCONF);
                    }
                #endif
            }
            else if (can_read_dir(dir)) {
                result = utf8_to_bxs(locate_config_in_dir(dir));
            }
            if (result != NULL) {
                break;
            }
        }
    }

    if (result == NULL && !error_printed) {
        fprintf(stderr, "%s: Can't find config file.\n", PROJECT);
    }
    #ifdef DISCOVERY_DEBUG
        fprintf(stderr, "discover_config_file() - exit -> [%s]\n", bxs_to_output(result));
    #endif
    return result;
}


/*EOF*/                                          /* vim: set cindent sw=4: */
