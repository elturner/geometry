#include "fp_optimizer.h"
#include <mesh/floorplan/floorplan.h>
#include <mesh/optimize/shapes/fp_wall.h>
#include <geometry/octree/octree.h>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <float.h>

/**
 * @file fp_optimizer.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file contains functions to optmize floorplans from octrees
 *
 * @section DESCRIPTION
 *
 * This file contains the fp_optimizer_t class, which is used to modify
 * the geometry of a floorplan in order to align it with the geometry
 * described in an octree.
 */

using namespace std;
using namespace Eigen;

/* function implementations */

/*------------*/
/* processing */
/*------------*/

int fp_optimizer_t::process_all(const string& octfile,
				const vector<string>& infiles,
				const vector<string>& outfiles)
{
	size_t i, n;
	int ret;

	/* verify validity of input by checking the same
	 * number of input and output files were given */
	if(infiles.size() != outfiles.size())
	{
		/* inform user of issue */
		cerr << "[fp_optimizer_t::process_all]\tError!  Must "
		     << "provide same number of input and output files."
		     << endl;
		return -1;
	}

	/* load the octree */
	ret = this->load_oct(octfile);
	if(ret)
	{
		/* unable to load file */
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[fp_optimizer_t::process_all]\tError " << ret
		     << ":  Unable to load .oct file: " << octfile << endl;
		return ret;
	}

	/* iterate through the floorplans */
	n = infiles.size();
	for(i = 0; i < n; i++)
	{
		/* load the floorplan */
		ret = this->load_fp(infiles[i]);
		if(ret)
		{
			/* unable to load fp */
			ret = PROPEGATE_ERROR(-3, ret);
			cerr << "[fp_optimizer_t::process_all]\tError "
			     << ret << ":  Unable to load .fp file: "
			     << infiles[i] << endl;
			return ret;
		}

		/* process this floorplan */
		ret = this->optimize();
		if(ret)
		{
			/* unable to process */
			ret = PROPEGATE_ERROR(-4, ret);
			cerr << "[fp_optimizer_t::process_all]\tError "
			     << ret << ":  Unable to process floorplan #"
			     << i << endl;
			return ret;
		}
		
		/* export the results */
		ret = this->export_fp(outfiles[i]);
		if(ret)
		{
			/* unable to load fp */
			ret = PROPEGATE_ERROR(-5, ret);
			cerr << "[fp_optimizer_t::process_all]\tError "
			     << ret << ":  Unable to export .fp file: "
			     << outfiles[i] << endl;
			return ret;
		}
	}

	/* success */
	return 0;
}
		
int fp_optimizer_t::process(const string& octfile,
			const string& infile, const string& outfile)
{
	int ret;
	
	/* load the octree */
	ret = this->load_oct(octfile);
	if(ret)
	{
		/* unable to load file */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[fp_optimizer_t::process]\tError " << ret
		     << ":  Unable to load .oct file: " << octfile << endl;
		return ret;
	}
		
	/* load the floorplan */
	ret = this->load_fp(infile);
	if(ret)
	{
		/* unable to load fp */
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[fp_optimizer_t::process]\tError "
		     << ret << ":  Unable to load .fp file: "
		     << infile << endl;
		return ret;
	}

	/* process this floorplan */
	ret = this->optimize();
	if(ret)
	{
		/* unable to process */
		ret = PROPEGATE_ERROR(-3, ret);
		cerr << "[fp_optimizer_t::process]\tError "
		     << ret << ":  Unable to process floorplan: "
		     << infile << endl;
		return ret;
	}
		
	/* export the results */
	ret = this->export_fp(outfile);
	if(ret)
	{
		/* unable to load fp */
		ret = PROPEGATE_ERROR(-4, ret);
		cerr << "[fp_optimizer_t::process]\tError "
		     << ret << ":  Unable to export .fp file: "
		     << outfile << endl;
		return ret;
	}

	/* success */
	return 0;
}

/*-----*/
/* i/o */
/*-----*/

int fp_optimizer_t::load_oct(const string& filename)
{
	int ret;

	/* load the octree */
	ret = this->tree.parse(filename);
	if(ret)
	{
		/* an error occurred */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[fp_optimizer_t::load_oct]\tError " << ret
		     << ": Unable to load .oct file: " << filename
		     << endl << endl;
		return ret;
	}

	/* success */
	return 0;
}

int fp_optimizer_t::load_fp(const string& filename)
{
	int ret;

	/* load floorplan info */
	ret = this->floorplan.import_from_fp(filename);
	if(ret)
	{
		/* an error occurred */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[fp_optimizer_t::load_fp]\tError " << ret
		     << ": Unable to load .fp file: " << filename
		     << endl << endl;
		return ret;
	}

	/* success */
	return 0;
}

int fp_optimizer_t::export_fp(const string& filename) const
{
	int ret;

	/* export floorplan */
	ret = this->floorplan.export_to_fp(filename);
	if(ret)
	{
		/* an error occurred */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[fp_optimizer_t::export_fp]\tError " << ret
		     << ": Unable to export .fp file: " << filename
		     << endl << endl;
		return ret;
	}

	/* success */
	return 0;	
}

/*------------------*/
/* helper functions */
/*------------------*/

int fp_optimizer_t::optimize()
{
	unsigned int i, num_iters; 

	/* retrieve desired number of iterations */
	num_iters = 5;// TODO
	
	/* optimize positions of walls, floors, and ceilings */
	for(i = 0; i < num_iters; i++)
	{
		/* perform a single iteration of optimization
		 * on this floorplan */
		this->run_iteration_walls();
		// TODO do floors and ceilings
	}
	
	/* success */
	return 0; 
}

void fp_optimizer_t::run_iteration_walls()
{
	vector<Vector2d,aligned_allocator<Vector2d> > net_offset;
	vector<double> total_cost;
	vector<fp::edge_t> edges; /* walls in floorplan */
	fp_wall_t wall;
	Vector2d v_unit, curr_offset;
	double r, r_min, r_max, r_step, r_best, c, c_best;
	size_t i, j, n;

	/* prepare offset vectors for each wall */
	total_cost.resize(this->floorplan.verts.size(), 0);
	net_offset.resize(this->floorplan.verts.size());

	/* prepare parameters */
	r_max = 0.05; // TODO
	r_min = -r_max;
	r_step = 0.25 * this->tree.get_resolution(); // TODO

	/* iterate over the walls */
	this->floorplan.compute_edges(edges);
	n = edges.size();
	for(i = 0; i < n; i++)
	{
		/* compute geometry for the i'th wall */
		wall.init(this->floorplan, edges[i]);

		/* find the best offset for this wall */
		c_best = DBL_MAX; /* initialize cost */
		r_best = 0;
		for(r = r_min; r <= r_max; r += r_step)
		{
			/* set the current wall to this offset */
			wall.set_offset(r);

			/* compute the cost at this offset */
			this->tree.find(wall);

			/* compare to best cost so far */
			c = wall.get_offset_cost();
			if(c < c_best)
			{
				/* update values for best-so-far */
				c_best = c;
				r_best = r;
			}
		}

		/* compute the offset vector due to the best offset dist */
		curr_offset = r_best * wall.get_norm(); 

		/* with the best offset distance, update the offset
		 * vector for the vertices of this edge */
		for(j = 0; j < fp::NUM_VERTS_PER_EDGE; j++)
		{
			/* since multiple walls can affect the position
			 * of each vertex, the net offset will be a
			 * weighted average of the offsets from each wall,
			 * based on the cost given */
			total_cost[edges[i].verts[j]] += c_best;
			net_offset[edges[i].verts[j]] += c_best*curr_offset;
		}
	}

	/* perturb vertex positions */
	n = total_cost.size();
	for(i = 0; i < n; i++)
	{
		/* normalize the offsets */
		if(total_cost[i] > 0)
			net_offset[i] /= total_cost[i];
	
		/* add vector to vertex position for incremental update */
		this->floorplan.verts[i].x += net_offset[i](0);
		this->floorplan.verts[i].y += net_offset[i](1);
	}
}
