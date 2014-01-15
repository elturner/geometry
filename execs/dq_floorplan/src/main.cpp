/* main.cpp:
 *
 *	This program generates a floor plan using the
 *	output of ncorso's particle filter, which is
 *	represented as a populated grid-map stored in
 *	a dynamic quadtree.
 *
 */

#include <iostream>
#include <fstream>

#include "process/generate_boundary.h"
#include "process/simplify_graph.h"
#include "process/export_data.h"
#include "structs/quadtree.h"
#include "structs/path.h"
#include "structs/cell_graph.h"
#include "rooms/tri_rep.h"
#include "io/dq_io.h"
#include "io/config.h"
#include "util/tictoc.h"

using namespace std;

int main(int argc, char** argv)
{
	config_t conf;
	quadtree_t tree;
	path_t path;
	cell_graph_t graph;
	tri_rep_t trirep;
	tictoc_t clk;
	int ret;

	/* read command-line args */
	if(parseargs(argc, argv, conf))
	{
		print_usage_short(argv[0]);
		return 1;
	}

	/* read in files */
	tic(clk);
	ret = read_dq(conf.dq_infile, tree);
	if(ret)
	{
		cerr << "Unable to read DQ file: " 
		     << conf.dq_infile << endl
		     << "\tError: " << ret << endl;
		return 1;
	}
	ret = path.readmad(conf.mad_infile);
	if(ret)
	{
		cerr << "Unable to read mad file: "
		     << conf.mad_infile << endl
		     << "\tError: " << ret << endl;
		return 1;
	}
	toc(clk, "Importing data");

	/* optionally limit path to number of poses specified */
	if(conf.num_poses > 0)
		path.pl.resize(conf.num_poses);

	/* create graph from tree, forming watertight boundary */
	ret = generate_boundary(graph, trirep, tree, path, 
	                        conf.carve_through);
	if(ret)
	{
		cerr << "Unable to generate boundary.  Error "
		     << ret << endl;
		return 1;
	}

	/* simplify walls */
	ret = simplify_graph(graph, trirep, conf.simplify_threshold);
	if(ret)
	{
		cerr << "Error: " << ret
		     << " unable to simplify graph." << endl;
		return 1;
	}

	/* export the result */
	tic(clk);
	ret = export_data(graph, trirep, conf);
	if(ret)
	{
		cerr << "Error: " << ret
		     << " unable to export floorplan data." << endl;
		return 1;
	}
	toc(clk, "Exporting data");

	/* success */
	return 0;
}
