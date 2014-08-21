#ifndef MATHLIB_H
#define MATHLIB_H

/* returns next largest integer of the form 2^n */
int next_largest_base_2(int n);

/* butterworth:
 *
 * 	Computes a butterworth function on the domain [0,1].
 *
 *	butterworth(0) = 1,
 *	butterworth(1) = 0, 
 *	butterworth(1/sqrt(2)) = 0.5
 */
double butterworth(double x);

#endif
