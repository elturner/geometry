#include "vox_writer.h"
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octdata.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <Eigen/Dense>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>

/**
 * @file vox_writer.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file defines classes used to export .vox files from octrees
 *
 * @section DESCRIPTION
 *
 * This file contains the vox_writer_t class, which is used to take an
 * octree, and export its contents to a .vox file.  This file format was
 * originally used for the voxel carving program specified in the following
 * paper:
 *
 * Eric Turner and Avideh Zakhor, "Watertight Planar Surface Meshing of 
 * Indoor Point-Clouds with Voxel Carving," Third Joint 3DIM/3DPVT 
 * Conference, Seattle, WA, June 29-July 1, 2013.
 *
 * This file format specifies the location of occupied voxels by explicitly
 * storing the voxel positions that occur on the boundary between connected
 * components of interior and exterior voxels.  Each voxel defined in the
 * .vox file is an exterior voxel that borders one or more interior voxels.
 *
 * This file requires the Eigen framework
 */

using namespace std;
using namespace Eigen;

/* basic geometry */
#define NUM_FACES_PER_VOXEL 6

/* the following macros will determine if a particular face
 * is boundary face within a boundary voxel, based on the voxel_state_t.
 *
 * arguments:
 *
 * 	v -	The voxel state to analyze
 * 	i -	The face to analyze
 */
#define VOXEL_IS_FACE_BIT_INWARD(v, i)   ( ((v) >> (i)) & 1 )
#define VOXEL_GET_FACE_BIT(i)            ( 1 << (i) )
#define VOXEL_SET_FACE_BIT_INWARD(v, i)  ( (v) |=   VOXEL_GET_FACE_BIT(i)  )
#define VOXEL_SET_FACE_BIT_OUTWARD(v, i) ( (v) &= ~(VOXEL_GET_FACE_BIT(i)) )

/* The following values define the enumeration of each face value
 *
 *         7 ________ 6           _____6__      ^      ________
 *         /|       /|         7/|       /|     |    /|       /|
 *       /  |     /  |        /  |     /5 |     |  /  5     /  |
 *   4 /_______ /    |      /__4____ /    10    |/_______2/    |
 *    |     |  |5    |     |    11  |     |     |     |  |   1 |
 *    |    3|__|_____|2    |     |__|__2__|     | 3   |__|_____|
 *    |    /   |    /      8   3/   9    /      |    /   |    /
 *    |  /     |  /        |  /     |  /1       |  /     4  /
 *    |/_______|/          |/___0___|/          |/_0_____|/________> x
 *   0          1                 
 *
 */
#define VOXEL_FACE_YMINUS 0
#define VOXEL_FACE_XPLUS  1
#define VOXEL_FACE_YPLUS  2
#define VOXEL_FACE_XMINUS 3
#define VOXEL_FACE_ZMINUS 4
#define VOXEL_FACE_ZPLUS  5

/* The following denotes the state of a voxel that is not at the boundary */
#define VOXEL_STATE_NONBOUNDARY ((voxel_state_t) 0)

/*--------------------------*/
/* function implementations */
/*--------------------------*/

/* The following table converts voxel face indices to directions */
Vector3d get_face_direction(int face_ind)
{
	Vector3d d;

	/* get direction of the face based on the index provided */
	switch(face_ind)
	{
		case VOXEL_FACE_YMINUS:
			d << 0,-1,0;
			break;
		case VOXEL_FACE_XPLUS:
			d << 1,0,0;
			break;
		case VOXEL_FACE_YPLUS:
			d << 0,1,0;
			break;
		case VOXEL_FACE_XMINUS:
			d << -1,0,0;
			break;
		case VOXEL_FACE_ZMINUS:
			d << 0,0,-1;
			break;
		case VOXEL_FACE_ZPLUS:
			d << 0,0,1;
			break;
		default:
			/* not a valid face index */
			break;
	}

	/* return the result */
	return d;
}

int vox_writer_t::write(const string& voxfile, const octree_t& tree)
{
	ofstream outfile;
	tictoc_t clk;
	Vector3d c;
	voxel_state_t s;
	int xi, yi, zi, min_x, max_x, min_y, max_y, min_z, max_z;
	double res, hw;

	/* retrieve important characteristics of the tree to analyze */
	res = tree.get_resolution();
	if(res <= 0)
		return -1; /* bad tree */

	/* get bounds of tree in units of voxels */
	c = tree.get_root()->center;
	hw = tree.get_root()->halfwidth;
	min_x = (int) floor((c(0) - hw)/res);
	max_x = (int) ceil( (c(0) + hw)/res);
	min_y = (int) floor((c(1) - hw)/res);
	max_y = (int) ceil( (c(1) + hw)/res);
	min_z = (int) floor((c(2) - hw)/res);
	max_z = (int) ceil( (c(2) + hw)/res);

	/* prepare file for writing */
	outfile.open(voxfile.c_str());
	if(!(outfile.is_open()))
		return -2; /* could not open file for writing */

	/* write header information */
	outfile << res << endl;

	/* iterate through potential voxel positions */
	tic(clk);
	for(xi = min_x; xi <= max_x; xi++)
		for(yi = min_y; yi <= max_y; yi++)
			for(zi = min_z; zi <= max_z; zi++)
			{
				/* get the center of this potential voxel */
				c << xi,yi,zi;
				c *= res;

				/* check if this voxel is a boundary */
				s = vox_writer_t::retrieve_state(tree,
							c, res);
				if(s == VOXEL_STATE_NONBOUNDARY)
					continue;

				/* export this boundary voxel info */
				outfile << xi << " " 
				        << yi << " "
					<< zi << " "
					<< ((int)s) << endl;
			}

	/* clean up */
	outfile.close();
	toc(clk, "Exporting vox file");
	return 0;
}

voxel_state_t vox_writer_t::retrieve_state(const octree_t& tree,
					const Vector3d& p, double r)
{
	octnode_t* root, *leaf, *neigh;
	Vector3d neigh_p;
	unsigned int i;
	voxel_state_t s;

	/* retrieve the root of this tree */
	root = tree.get_root();

	/* get the leaf node at the specified position in the tree */
	leaf = root->retrieve(p);
	
	/* check if this location is interior or exterior */
	if(leaf != NULL && leaf->data != NULL && leaf->data->is_interior())
	{
		/* this point resides in interior volume, which means that
		 * it cannot be a boundary voxel */
		return VOXEL_STATE_NONBOUNDARY;
	}

	/* iterate over the neighbors of the recovered voxel */
	s = VOXEL_STATE_NONBOUNDARY;
	for(i = 0; i < NUM_FACES_PER_VOXEL; i++)
	{
		/* get position of neighboring voxel */
		neigh_p = p + r*get_face_direction(i);

		/* check if this neighbor is interior */
		neigh = root->retrieve(neigh_p);
		if(neigh == NULL)
		{
			/* outside of bounds, which means neighbor
			 * is exterior */
			VOXEL_SET_FACE_BIT_OUTWARD(s,i);
		}
		else if(neigh->data == NULL)
		{
			/* neighbor has no data, which implies that this
			 * voxel is exterior */
			VOXEL_SET_FACE_BIT_OUTWARD(s,i);
		}
		else if(neigh->data->is_interior())
		{
			/* point is labeled as interior */
			VOXEL_SET_FACE_BIT_INWARD(s,i);
		}
		else
		{
			/* the neighbor is an exterior voxel */
			VOXEL_SET_FACE_BIT_OUTWARD(s,i);
		}
	}

	/* return the computed result */
	return s;
}
