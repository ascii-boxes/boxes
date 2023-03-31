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

    bxstr_t *result = (bxstr_t *) calloc(1, sizeof(bxstr_t));
    result->memory = u32_strconv_from_arg(pAscii, "ASCII");
    if (result->memory == NULL) {
        BFREE(result);
        return NULL;
    }
    result->ascii = strdup(pAscii);

    size_t error_pos = 0;
    if (!bxs_valid_anywhere(result, &error_pos)) {
        ucs4_t c = result->memory[error_pos];
        bx_fprintf(stderr, "%s: illegal character '%lc' (%#010x) encountered in string\n", PROJECT, c, (int) c);
        bxs_free(result);
        return NULL;
    }

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

    result->first_char = calloc(result->num_chars_visible + 1, sizeof(size_t));
    result->visible_char = calloc(result->num_chars_visible + 1, sizeof(size_t));
    for (size_t i = 0; i <= result->num_chars_visible; i++) {
        result->first_char[i] = i;
        result->visible_char[i] = i;
    }

    return result;
}



bxstr_t *bxs_from_unicode(uint32_t *pInput)
{
    if (pInput == NULL) {
        bx_fprintf(stderr, "%s: internal error: bxs_from_unicode() called with NULL\n", PROJECT);
        return NULL;
    }

    bxstr_t *result = (bxstr_t *) calloc(1, sizeof(bxstr_t));
    result->memory = u32_strdup(pInput);
    result->num_chars = u32_strlen(pInput);
    size_t ascii_len = ((size_t) u32_strwidth(pInput, encoding)) + 1;
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
    int non_blank_encountered = 0;
    size_t idx = 0;

    for (ucs4_t c = pInput[0]; c != char_nul; c = rest[0]) {
        if (result->num_chars_visible >= map_size - 2) {
            map_size = map_size * 2 + 1;
            result->first_char = (size_t *) realloc(result->first_char, map_size * sizeof(size_t));
            result->visible_char = (size_t *) realloc(result->visible_char, map_size * sizeof(size_t));
        }

        if (!is_allowed_anywhere(c)) { /* CHECK currently used for config only, reconsider when using on input data */
            bx_fprintf(stderr, "%s: illegal character '%lc' (%#010x) encountered in string\n", PROJECT, c, (int) c);
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
            else if (c == char_tab) {
                *ascii_ptr = ' ';
                ++ascii_ptr;
            }
            else {
                cols = BMAX(0, uc_width(c, encoding));
                if (cols > 0) {
                    memset(ascii_ptr, (int) (uc_is_blank(c) ? ' ' : 'x'), cols);
                    ascii_ptr += cols;
                }
            }
            if (is_blank(c)) {
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

        if (!is_blank(c) && c != char_esc) {
            indent_active = 0;
            blank_streak = 0;
            non_blank_encountered = 1;
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
    result->trailing = non_blank_encountered ? blank_streak : 0;
    return result;
}



bxstr_t *bxs_strdup(bxstr_t *pString)
{
    if (pString == NULL) {
        return NULL;
    }
    bxstr_t *result = (bxstr_t *) calloc(1, sizeof(bxstr_t));
    if (result != NULL) {
        result->memory = u32_strdup(pString->memory);
        result->ascii = strdup(pString->ascii);
        result->indent = pString->indent;
        result->num_columns = pString->num_columns;
        result->num_chars = pString->num_chars;
        result->num_chars_visible = pString->num_chars_visible;
        result->num_chars_invisible = pString->num_chars_invisible;
        result->trailing = pString->trailing;
        result->first_char = malloc((pString->num_chars_visible + 1) * sizeof(size_t));
        memcpy(result->first_char, pString->first_char, (pString->num_chars_visible + 1) * sizeof(size_t));
        result->visible_char = malloc((pString->num_chars_visible + 1) * sizeof(size_t));
        memcpy(result->visible_char, pString->visible_char, (pString->num_chars_visible + 1) * sizeof(size_t));
    }
    return result;
}



bxstr_t *bxs_trimdup(bxstr_t *pString, size_t start_idx, size_t end_idx)
{
    if (pString == NULL) {
        return NULL;
    }
    if (start_idx > pString->num_chars_visible) {
        /* a start_idx on the terminating NUL is a valid input */
        bx_fprintf(stderr, "%s: internal error: start_idx out of bounds in bxs_trimdup()\n", PROJECT);
        return NULL;
    }
    if (end_idx > pString->num_chars_visible) {
        bx_fprintf(stderr, "%s: internal error: end_idx out of bounds in bxs_trimdup()\n", PROJECT);
        return NULL;
    }
    if (end_idx < start_idx) {
        bx_fprintf(stderr, "%s: internal error: end_idx before start_idx in bxs_trimdup()\n", PROJECT);
        return NULL;
    }

    while (start_idx < end_idx && uc_is_blank(pString->memory[pString->visible_char[start_idx]])) {
        start_idx++;
    }
    while (start_idx < end_idx && uc_is_blank(pString->memory[pString->visible_char[end_idx - 1]])) {
        end_idx--;
    }

    ucs4_t save = char_nul;
    if (end_idx < pString->num_chars_visible) {
        save = pString->memory[pString->first_char[end_idx]];
        set_char_at(pString->memory, pString->first_char[end_idx], char_nul);
    }

    bxstr_t *result = bxs_from_unicode(pString->memory + pString->first_char[start_idx]);
    if (end_idx < pString->num_chars_visible) {
        set_char_at(pString->memory, pString->first_char[end_idx], save);
    }
    return result;
}



bxstr_t *bxs_strcat(bxstr_t *pString, uint32_t *pToAppend)
{
    if (pToAppend == NULL) {
        return bxs_strdup(pString);
    }
    size_t appened_num_chars = u32_strlen(pToAppend);
    if (appened_num_chars == 0) {
        return bxs_strdup(pString);
    }
    if (pString == NULL || pString->num_chars == 0) {
        return bxs_from_unicode(pToAppend);
    }

    size_t combined_num_chars = pString->num_chars + appened_num_chars;
    uint32_t *s = (uint32_t *) malloc((combined_num_chars + 1) * sizeof(uint32_t));
    memcpy(s, pString->memory, pString->num_chars * sizeof(uint32_t));
    memcpy(s + pString->num_chars, pToAppend, appened_num_chars * sizeof(uint32_t));
    set_char_at(s, combined_num_chars, char_nul);

    bxstr_t *result = bxs_from_unicode(s);
    BFREE(s);
    return result;
}



uint32_t *bxs_strchr(bxstr_t *pString, ucs4_t c, int *cursor)
{
    uint32_t *result = NULL;
    if (pString != NULL && pString->num_chars_visible > 0) {
        size_t start_idx = cursor != NULL ? *cursor + 1 : 0;
        for (size_t i = start_idx; i < pString->num_chars_visible; i++) {
            if (pString->memory[pString->visible_char[i]] == c) {
                result = pString->memory + pString->visible_char[i];
                if (cursor != NULL) {
                    *cursor = (int) i;
                }
                break;
            }
        }
    }
    return result;
}



bxstr_t *bxs_trim(bxstr_t *pString)
{
    if (pString == NULL) {
        return NULL;
    }
    if (pString->indent == 0 && pString->trailing == 0) {
        return bxs_strdup(pString);
    }
    if (pString->indent + pString->trailing == pString->num_chars_visible) {
        return bxs_from_ascii("");
    }

    uint32_t *e = u32_strdup(pString->memory);
    set_char_at(e, pString->first_char[pString->num_chars_visible - pString->trailing], char_nul);
    uint32_t *s = e + pString->first_char[pString->indent];
    bxstr_t *result = bxs_from_unicode(s);
    BFREE(e);
    return result;
}



bxstr_t *bxs_rtrim(bxstr_t *pString)
{
    if (pString == NULL) {
        return NULL;
    }
    if (pString->trailing == 0) {
        return bxs_strdup(pString);
    }

    uint32_t *s = u32_strdup(pString->memory);
    set_char_at(s, pString->first_char[pString->num_chars_visible - pString->trailing], char_nul);
    bxstr_t *result = bxs_from_unicode(s);
    BFREE(s);
    return result;
}



char *bxs_to_output(bxstr_t *pString)
{
    if (pString == NULL) {
        return strdup("NULL");
    }
    return u32_strconv_to_output(pString->memory);
}



int bxs_is_empty(bxstr_t *pString)
{
    if (pString == NULL) {
        return 1;
    }
    return pString->num_chars > 0 ? 0 : 1;
}



int bxs_strcmp(bxstr_t *s1, bxstr_t *s2)
{
    if (s1 == NULL) {
        if (s2 == NULL) {
            return 0;
        }
        else {
            return 1;
        }
    }
    if (s2 == NULL) {
        return -1;
    }
    return u32_strcmp(s1->memory, s2->memory);
}



static int bxs_valid_in_context(bxstr_t *pString, size_t *error_pos, int (*predicate)(const ucs4_t))
{
    if (pString == NULL) {
        if (error_pos != NULL) {
            *error_pos = 0;
        }
        return 0;   /* invalid */
    }

    for (size_t i = 0; pString->memory[i] != char_nul; i++) {
        if ((*predicate)(pString->memory[i]) == 0) {
            if (error_pos != NULL) {
                *error_pos = i;
            }
            return 0;   /* invalid */
        }
    }

    return 1;   /* valid */
}



int bxs_valid_anywhere(bxstr_t *pString, size_t *error_pos)
{
    return bxs_valid_in_context(pString, error_pos, &is_allowed_anywhere);
}



int bxs_valid_in_shape(bxstr_t *pString, size_t *error_pos)
{
    return pString->num_chars_visible > 0 && bxs_valid_in_context(pString, error_pos, &is_allowed_in_shape);
}



int bxs_valid_in_sample(bxstr_t *pString, size_t *error_pos)
{
    return pString->num_chars_visible > 0 && bxs_valid_in_context(pString, error_pos, &is_allowed_in_sample);
}



int bxs_valid_in_filename(bxstr_t *pString, size_t *error_pos)
{
    return pString->num_chars_visible > 0 && pString->num_chars_invisible == 0
            && bxs_valid_in_context(pString, error_pos, &is_allowed_in_filename);
}



int bxs_valid_in_kv_string(bxstr_t *pString, size_t *error_pos)
{
    return bxs_valid_in_context(pString, error_pos, &is_allowed_in_kv_string);
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
