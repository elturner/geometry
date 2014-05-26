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
	size_t i, num_iters;

	num_iters = 200; // TODO get convergence parameter
	for(i = 0; i < num_iters; i++)
	{
		this->run_iteration_walls();
		stringstream ss;
		ss << "/home/elturner/Desktop/test/fp_iter_" << i << ".fp";
		this->export_fp(ss.str());
	}
	
	// TODO run walls and heights

	/* success */
	return 0; 
}

int fp_optimizer_t::run_iteration_walls()
{
	vector<Vector2d,aligned_allocator<Vector2d> > net_forces;
	vector<fp::edge_t> edges; /* walls in floorplan */
	fp_wall_t wall;
	Vector2d f0, f1;
	double r, stepsize, norm;
	size_t i, n;

	/* prepare force vectors for each wall */
	net_forces.resize(this->floorplan.verts.size());

	/* iterate over the walls */
	this->floorplan.compute_edges(edges);
	n = edges.size();
	r = sqrt(2) * this->tree.get_resolution(); // TODO
	for(i = 0; i < n; i++)
	{
		/* compute geometry for the i'th wall */
		wall.init(r, this->floorplan, edges[i]);

		/* get the scalar field intersected by the i'th wall */
		wall.toggle_offset(false);
		this->tree.find(wall);
		wall.toggle_offset(true);
		this->tree.find(wall);

		/* add the computed forces to our net sum */
		wall.compute_forces(f0, f1);
		net_forces[edges[i].verts[0]] += f0;
		net_forces[edges[i].verts[1]] += f1;
	}

	/* normalize the force vectors */
	norm = 0;
	stepsize = 0.5 * this->tree.get_resolution(); // TODO
	n = net_forces.size();
	for(i = 0; i < n; i++)
		norm += net_forces[i].squaredNorm();
	norm = stepsize / sqrt(norm); /* L2 norm across all forces */
	for(i = 0; i < n; i++)
		net_forces[i] *= norm;

	/* perturb vertex positions */
	for(i = 0; i < n; i++)
	{
		/* add vector to vertex position for incremental update */
		this->floorplan.verts[i].x += net_forces[i](0);
		this->floorplan.verts[i].y += net_forces[i](1);
	}
	
	/* success */
	return 0;
}
		
int fp_optimizer_t::run_iteration_heights()
{
	return -1; // TODO
}
