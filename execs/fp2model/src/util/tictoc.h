#ifndef TIC_TOC_H
#define TIC_TOC_H

/* tictoc.h
 *
 * This file houses functions
 * meant to emulate tic() and toc()
 * in matlab.  They will time the
 * duration of a program using the
 * system clock() function.
 */

#include <time.h>

typedef clock_t tictoc_t;

/* If true, will print out results
 * when toc() is called */
#define PRINT_TIMING 1

/* tic:
 *
 *	Records the current time in t.
 */
void tic(tictoc_t& t);

/* toc:
 *
 * 	Prints out how long has passed
 * 	since tic(t) was called.
 *
 * 	The program since tic(t) will be
 * 	printed to screen with description
 * 	given.
 *
 * return value:
 *
 * 	Returns number of seconds since tic()
 */
double toc(tictoc_t& t, const char* description);

#endif
