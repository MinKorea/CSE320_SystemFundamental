/*
 * Error handling routines
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "error.h"

int errors;
int warnings;
int dbflag = 1;

void fatal(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);

        fprintf(stderr, "\nFatal error: ");
        // fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);

        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fprintf(stderr, "\n");
        // exit(1);
}

void error(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);

        fprintf(stderr, "\nError: ");
        //fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        errors++;
}

void warning(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);

        fprintf(stderr, "\nWarning: ");
        //fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        warnings++;
}

void debug(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);

        if(!dbflag) return;
        fprintf(stderr, "\nDebug: ");
        //fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
}
