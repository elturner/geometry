/* main.cpp:
 *
 * 	This program will generate separate pointclouds for each floor
 * 	of a scanned building based on histogram analysis of the
 * 	input scans.
 */

#include <iostream>
#include <vector>
#include "io/config.h"
#include "structs/histogram.h"
#include "process/populate_histogram.h"
#include "process/compute_level_ranges.h"
#include "process/export_data.h"

using namespace std;

int main(int argc, char** argv)
{
	config_t conf;
	histogram_t floors, ceilings;
	vector<double> floor_heights, ceiling_heights;
	int ret;

	/* read command-line args */
	if(conf.parseargs(argc, argv))
	{
		conf.print_usage_short();
		return 1;
	}

	/* generate histogram from scans */
	ret = populate_histogram(floors, ceilings, conf);
	if(ret)
	{
		cerr << "Unable to generate histogram, error: " 
		     << ret << endl;
		return 1;
	}

	/* determine partitioning */
	ret = compute_level_ranges(floor_heights, ceiling_heights,
					floors, ceilings, conf);
	if(ret)
	{
		cerr << "Unable to compute level ranges, error: "
		     << ret << endl;
		return 1;
	}

	/* generate pointclouds for each floor */
	ret = export_data(floor_heights, ceiling_heights,
					floors, ceilings, conf);
	if(ret)
	{
		cerr << "Unable to export data, error: "
		     << ret << endl;
		return 1;
	}

	/* success */
	return 0;
}
