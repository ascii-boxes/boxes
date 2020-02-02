/*
 *  File:             regsub.c
 *  Date created:     Copyright (c) 1986 by University of Toronto.
 *  Author:           Henry Spencer.
 *                    Extensions and modifications by Thomas Jensen
 *  Language:         K&R C (traditional)
 *  Purpose:          Perform substitutions after a regexp match
 *  License:          - Not derived from licensed software.
 *                    - Permission is granted to anyone to use this
 *                      software for any purpose on any computer system,
 *                      and to redistribute it freely, subject to the
 *                      following restrictions:
 *                      1. The author is not responsible for the
 *                         consequences of use of this software, no matter
 *                         how awful, even if they arise from defects in it.
 *                      2. The origin of this software must not be
 *                         misrepresented, either by explicit claim or by
 *                         omission.
 *                      3. Altered versions must be plainly marked as such,
 *                         and must not be misrepresented as being the
 *                         original software.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#include <stdio.h>
#include <string.h>
#include "regexp.h"
#include "regmagic.h"


#ifndef CHARBITS
#define UCHARAT(p)      ((int)*(unsigned char *)(p))
#else
#define UCHARAT(p)      ((int)*(p)&CHARBITS)
#endif



/*
 - regsub - perform substitutions after a regexp match
 */
size_t                                   /* RETURNS length of dest str */
regsub (prog, source, dest, dest_size)
    regexp *prog;
    char *source;
    char *dest;
    size_t dest_size;                    /* size of destination buffer */
{
    register char *src;
    register char *dst;
    register char c;
    register int no;
    register int len;
    size_t fill;                         /* current number of chars in dest */

    if (prog == NULL || source == NULL || dest == NULL) {
        regerror("NULL parm to regsub");
        return 0;
    }
    if (UCHARAT(prog->program) != MAGIC) {
        regerror("damaged regexp fed to regsub");
        return 0;
    }

    src = source;
    dst = dest;
    fill = 0;

    while ((c = *src++) != '\0') {
        if (c == '&')
            no = 0;
        else if (c == '\\' && '0' <= *src && *src <= '9')
            no = *src++ - '0';
        else
            no = -1;

        if (no < 0) {   /* Ordinary character. */
            if (c == '\\' && (*src == '\\' || *src == '&'))
                c = *src++;
            *dst++ = c;
            ++fill;
        } else if (prog->startp[no] != NULL && prog->endp[no] != NULL) {
            len = prog->endp[no] - prog->startp[no];
            if (len < dest_size-fill) {
                (void) strncpy(dst, prog->startp[no], len);
                dst += len;
                fill += len;
                if (len != 0 && *(dst-1) == '\0') { /* strncpy hit NUL. */
                    regerror("damaged match string");
                    return fill;
                }
            }
            else {
                (void) strncpy (dst, prog->startp[no], dest_size-fill);
                dest[dest_size-1] = '\0';
                return dest_size-1;
            }
        }
        if (fill >= dest_size) {
            dest[dest_size-1] = '\0';
            return dest_size-1;
        }
    }
    *dst++ = '\0';

    return fill;
}



size_t                       /* RETURNS length of str in destination buffer */
myregsub (prog, orig, orig_len, repstr, dest, dest_size, mode)
    regexp *prog;            /* pointers for matched regexp to original text */
    char *orig;              /* original input line */
    size_t orig_len;         /* length of original input line */
    char *repstr;            /* source buffer for replaced parts */
    char *dest;              /* destination buffer */
    size_t dest_size;        /* size of destination buffer */
    char mode;               /* 'g' or 'o' */
{
    size_t fill;                         /* current number of chars in dest */
    char  *sp, *dp;                      /* source rover, destination rover */
    int rc;                              /* received return codes */
    size_t rest_size;                    /* remaining space in dest */
    size_t partlen;                      /* temp length of a piece handled */

    fill = 0;
    sp = orig;
    dp = dest;
    rest_size = dest_size;

    do {
        rc = regexec (prog, sp);
        if (!rc) break;

        partlen = prog->startp[0] - sp;
        if (partlen < rest_size) {
            strncpy (dp, sp, partlen);
            fill += partlen;
            sp = prog->startp[0];
            dp += partlen;
            rest_size -= partlen;
        }
        else {
            strncpy (dp, sp, rest_size);
            dest[dest_size-1] = '\0';
            return dest_size - 1;
        }

        /* fprintf (stderr, "regsub (%p, \"%s\", \"%s\", %d);\n", */
        /*         prog, repstr, dp, rest_size);                  */
        fill += regsub (prog, repstr, dp, rest_size);
        dp = dest + fill;
        sp = prog->endp[0];
        rest_size = dest_size - fill;

        if (fill >= dest_size) {
            dest[dest_size-1] = '\0';
            return dest_size - 1;
        }

        /* fprintf (stderr, "dest = \"%s\";\n", dest); */
        if (prog->startp[0] == prog->endp[0])
            break;                       /* match "^" or "$" only once */

    } while (mode == 'g');

    partlen = orig + orig_len - sp;
    if (partlen < rest_size) {
        strncpy (dp, sp, partlen);
        fill += partlen;
        dp[partlen] = '\0';
    }
    else {
        strncpy (dp, sp, rest_size);
        dest[dest_size-1] = '\0';
        fill = dest_size - 1;
    }

    return fill;
}




/*EOF*/                                                    /* vim: set sw=4: */
