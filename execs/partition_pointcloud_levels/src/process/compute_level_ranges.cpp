#include "compute_level_ranges.h"
#include <vector>
#include "../io/config.h"
#include "../structs/histogram.h"
#include "../util/error_codes.h"
#include "../util/tictoc.h"

using namespace std;

int compute_level_ranges(vector<double>& floor_heights, 
			vector<double>& ceil_heights, 
			histogram_t& floor_hist, 
			histogram_t& ceil_hist,
			config_t& conf)
{
	vector<double> floor_peaks, ceil_peaks;
	vector<int> floor_counts, ceil_counts;
	int i, fi, fi_next, ci, ci_next, fn, cn;
	tictoc_t clk;

	/* start timer */
	tic(clk);

	/* find locations peaks in the histograms */
	floor_hist.find_peaks(floor_peaks, floor_counts, 
				conf.min_floor_height); 
	ceil_hist.find_peaks(ceil_peaks, ceil_counts, 
				conf.min_floor_height); 

	/* clear output */
	floor_heights.clear();
	ceil_heights.clear();

	/* find matching floor/ceiling pairs */
	fn = floor_peaks.size();
	cn = ceil_peaks.size();
	fi = fi_next = ci = ci_next = 0;
	while(fi < fn && ci < cn)
	{
		/* find the floor with the highest count that is
		 * still below the current ceiling */
		for(i = fi+1; i<fn && floor_peaks[i] < ceil_peaks[ci]; i++)
			if(floor_counts[i] > floor_counts[fi])
				fi = i;

		/* figure out what the next floor above the current ceiling
		 * is */
		fi_next = fi;
		while(fi_next < fn && floor_peaks[fi_next] < ceil_peaks[ci])
			fi_next++;

		/* find the ceiling with the highest count that is below
		 * the next floor position */
		for(i = ci+1; i<cn && (fi_next >= fn 
				|| ceil_peaks[i] < floor_peaks[fi_next]); 
				i++)
			if(ceil_counts[i] > ceil_counts[ci])
				ci = i;
	
		/* we know have the optimum floor and ceiling positions
		 * for this level, so export those to the output */
		floor_heights.push_back(floor_peaks[fi]);
		ceil_heights.push_back(ceil_peaks[ci]);

		/* find the next ceiling */
		ci_next = ci;
		while(ci_next < cn && ceil_peaks[ci_next] 
					< floor_peaks[fi_next])
			ci_next++;

		/* move to next floor */
		fi = fi_next;
		ci = ci_next;
	}

	/* success */
	toc(clk, "Computing level ranges");
	return 0;
}
