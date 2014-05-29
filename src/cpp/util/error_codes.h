#ifndef ERROR_CODES_H
#define ERROR_CODES_H

/**
 * @file error_codes.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains macros and functions that
 * are used to log errors and warnings.
 */

#include <stdio.h>

/* the following macro will propegate error codes up functions,
 * so that the source of an error is easy to trace. */
#define PROPEGATE_ERROR(curr, prev) ( (curr) + 100*(prev) )

/* Will suppress "unused parameter" warning */
#define MARK_USED(X)  ((void)(&(X)))

#endif
