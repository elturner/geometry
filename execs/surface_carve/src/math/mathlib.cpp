#include "mathlib.h"

#define SQRT_2 1.4142145

int next_largest_base_2(int n)
{
	n |= (n >> 1);
	n |= (n >> 2);
	n |= (n >> 4);
	n |= (n >> 8);
	n |= (n >> 16);

	return n+1;
}

double butterworth(double x)
{
	double d;
	
	d = (x * SQRT_2);
	return 1 / (1 + d*d*d*d);
}
