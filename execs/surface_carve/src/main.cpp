/* main.cpp:
 *
 * 	This is the main file of the surface carving
 * 	program.  This program will take a point-cloud
 * 	and generate a surface.
 */

#include <iostream>
#include <vector>

#include "io/config.h"
#include "structs/dgrid.h"
#include "structs/triangulation.h"
#include "triangulate/region_growing.h"
#include "process/makegrid.h"
#include "process/create_mesh.h"
#include "process/export_data.h"

using namespace std;

int main(int argc, char** argv)
{
	config_t conf;
	dgrid_t grid;
	triangulation_t tri;
	vector<planar_region_t> regions;
	int ret;

	/* read command-line args */
	if(parseargs(argc, argv, conf))
	{
		print_usage_short(argv[0]);
		return 1;
	}

	/* use the given files to carve a voxel grid
	 * from the laser scans */
	ret = make_grid(grid, conf);
	if(ret)
	{
		cerr << "Error in make-grid: " << ret << endl;
		return 1;
	}

	/* create a mesh from the surface of this grid */
	ret = create_mesh(tri, regions, grid, conf);
	if(ret)
	{
		cerr << "Error in creating mesh: " << ret << endl;
		return 1;
	}
	
	/* export surface */
	ret = export_data(tri, regions, conf);
	if(ret)
	{
		cerr << "Error in exporting data: " << ret << endl;
		return 1;
	}

	return 0;
}
