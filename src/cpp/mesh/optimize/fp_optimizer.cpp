#include "fp_optimizer.h"
#include <mesh/floorplan/floorplan.h>
#include <geometry/octree/octree.h>
#include <util/error_codes.h>
#include <iostream>
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
	return -1;// TODO
}
