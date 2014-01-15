#include "tictoc.h"
#include <time.h>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>

void tic(tictoc_t& t)
{
	t = clock();
}

double toc(tictoc_t& t, const char* description)
{
	tictoc_t stop, diff;
	double dd;

	/* get time now */
	stop = clock();
	
	/* print difference */
	diff = stop - t;
	dd = ((double)diff)/CLOCKS_PER_SEC;
	if(PRINT_TIMING && description != NULL)
		printf("%32s took %.3f sec\n", description, dd);

	return dd;
}
