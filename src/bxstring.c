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
 * The boxes-internal representation of strings.
 */

#include "config.h"

#include <string.h>
#include <unictype.h>
#include <unistr.h>
#include <uniwidth.h>

#include "bxstring.h"
#include "tools.h"
#include "unicode.h"



bxstr_t *bxs_from_ascii(char *pAscii)
{
    if (pAscii == NULL) {
        bx_fprintf(stderr, "%s: internal error: from_ascii() called with NULL\n", PROJECT);
        return NULL;
    }
    if (strchr(pAscii, '\t') != NULL) {
        bx_fprintf(stderr, "%s: internal error: from_ascii() called with tabs: \"%s\"\n", PROJECT, pAscii);
        return NULL;
    }

    bxstr_t *result = (bxstr_t *) calloc(1, sizeof(bxstr_t));
    result->memory = u32_strconv_from_arg(pAscii, "ASCII");
    if (result->memory == NULL) {
        BFREE(result);
        return NULL;
    }
    result->ascii = strdup(pAscii);

    size_t num_esc = 0;
    char *ascii_copy;
    size_t *map;
    result->num_chars_invisible = count_invisible_chars(result->memory, &num_esc, &ascii_copy, &(map));
    BFREE(ascii_copy);

    result->num_chars = strlen(pAscii);
    result->num_columns = result->num_chars;
    result->num_chars_visible = result->num_chars - result->num_chars_invisible;

    result->indent = strspn(pAscii, " ");
    result->trailing = my_strrspn(pAscii, " ");

    result->first_char = malloc(result->num_chars_visible * sizeof(size_t));
    result->visible_char = malloc(result->num_chars_visible * sizeof(size_t));
    for (size_t i = 0; i < result->num_chars_visible; i++) {
        result->first_char[i] = i;
        result->visible_char[i] = i;
    }

    return result;
}



bxstr_t *bxs_from_unicode(uint32_t *pInput)
{
    if (pInput == NULL) {
        bx_fprintf(stderr, "%s: internal error: from_unicode() called with NULL\n", PROJECT);
        return NULL;
    }

    bxstr_t *result = (bxstr_t *) calloc(1, sizeof(bxstr_t));
    result->memory = u32_strdup(pInput);
    result->num_chars = u32_strlen(pInput);
    size_t ascii_len = ((size_t) u32_strwidth(pInput, encoding)) + 1;   /* often generous, but always enough */
    result->ascii = (char *) calloc(ascii_len, sizeof(char));
    size_t map_size = 5;
    result->first_char = (size_t *) calloc(map_size, sizeof(size_t));
    result->visible_char = (size_t *) calloc(map_size, sizeof(size_t));
    char *ascii_ptr = result->ascii;

    const uint32_t *rest = pInput;
    size_t step_invis = 0;
    int indent_active = 1;
    size_t blank_streak = 0;
    int first_candidate = -1;
    size_t idx = 0;

    for (ucs4_t c = pInput[0]; c != char_nul; c = rest[0]) {
        if (result->num_chars_visible >= map_size - 2) {
            map_size = map_size * 2 + 1;
            result->first_char = (size_t *) realloc(result->first_char, map_size * sizeof(size_t));
            result->visible_char = (size_t *) realloc(result->visible_char, map_size * sizeof(size_t));
        }

        if (c == char_tab) {
            bx_fprintf(stderr, "%s: internal error: tab encountered in from_unicode()\n", PROJECT);
            bxs_free(result);
            return NULL;
        }
        else if (c == char_esc) {
            if (is_csi_reset(rest)) {
                first_candidate = -1;
            }
            else {
                first_candidate = idx;
            }
        }
        else {
            int cols = 1;
            if (is_ascii_printable(c)) {
                *ascii_ptr = c & 0xff;
                ++ascii_ptr;
            }
            else {
                cols = BMAX(0, uc_width(c, encoding));
                if (cols > 0) {
                    memset(ascii_ptr, (int) (uc_is_blank(c) ? ' ' : 'x'), cols);
                    ascii_ptr += cols;
                }
            }
            if (uc_is_blank(c)) {
                if (indent_active) {
                    result->indent += cols;
                }
                blank_streak++;
            }
            result->num_columns += BMAX(0, cols);
            result->visible_char[result->num_chars_visible] = idx;
            result->first_char[result->num_chars_visible] = first_candidate < 0 ? idx : (size_t) first_candidate;
            first_candidate = -1;
        }

        if (!uc_is_blank(c) && c != char_esc) {
            indent_active = 0;
            blank_streak = 0;
        }

        rest = advance_next32(rest, &step_invis);

        if (step_invis == 0) {
            result->num_chars_visible++;
            idx++;
        }
        else {
            result->num_chars_invisible += step_invis;
            idx += step_invis;
        }
    }

    *ascii_ptr = '\0';
    result->visible_char[result->num_chars_visible] = idx;  // both point to the terminator
    result->first_char[result->num_chars_visible] = idx;
    result->trailing = blank_streak;
    return result;
}



void bxs_free(bxstr_t *pString)
{
    if (pString != NULL) {
        BFREE(pString->memory);
        BFREE(pString->ascii);
        BFREE(pString->first_char);
        BFREE(pString->visible_char);
        BFREE(pString);
    }
}


/* vim: set cindent sw=4: */
