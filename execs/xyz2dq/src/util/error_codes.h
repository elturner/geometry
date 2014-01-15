#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#include <stdio.h>

/* The following will append a current error code of a function
 * to the error code that was just returned.  If there are several
 * nested functions, then a failure can be easily traced back
 * to the source with the resulting value */
#define PROPEGATE_ERROR(curr, prev) ( (curr) + 100*(prev) )

/* the following is used for debugging purposes */

#define PRINT_STATISTICS
#define VERBOSE

#ifdef VERBOSE
#define LOG(str) printf(str)
#define LOGI(str,i) printf(str,i)
#else
#define LOG(str)
#define LOGI(str,i)
#endif

/* PRINT_ERROR:
 *
 * 	Prints an error message in red to stderr, with new-line.
 */
#define PRINT_ERROR(str) ( fprintf(stderr, "%c[0;31m%s%c[0m\n", \
					0x1b, str, 0x1b) )

/* PRINT_WARNING:
 *
 * 	Prints a warning message in yellow to stdout, with new-line.
 */
#define PRINT_WARNING(str) ( fprintf(stdout, "%c[0;33m%s%c[0m\n", \
					0x1b, str, 0x1b) )


#endif
