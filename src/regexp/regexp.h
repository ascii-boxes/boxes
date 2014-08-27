/*
 * Definitions etc. for regexp(3) routines.
 *
 * Version: $Id: regexp.h,v 1.5 1999/06/25 18:53:51 tsjensen Exp $
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */

#ifndef REGEXP_H
#define REGEXP_H


#define NSUBEXP  10
typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

extern regexp *regcomp();
/* extern int regexec();   */
/* extern size_t regsub(); */
extern size_t myregsub();
/* extern void regerror(); */


#endif /* REGEXP_H */
