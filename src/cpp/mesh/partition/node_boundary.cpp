#include "node_boundary.h"
#include <geometry/octree/octnode.h>
#include <geometry/octree/octtopo.h>
#include <geometry/poly_intersect/poly2d.h>
#include <mesh/partition/node_set.h>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <string>
#include <vector>
#include <map>

/**
 * @file node_boundary.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Classes used to define boundary nodes in octrees
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to formulate the set of boundary
 * nodes in a given octree.  Boundary nodes are nodes that are labeled
 * interior, but are adjacent to exterior nodes.  The "is_interior()"
 * function of octdata objects is used to determine if the nodes
 * are interior or exterior.
 */

using namespace octtopo;
using namespace std;
using namespace Eigen;

/* the following constants are used for computations in this file */

#define APPROX_ZERO  0.0000001

/* function implementations */

int node_boundary_t::populate(const octtopo_t& topo)
{
	int ret;

	/* populate the boundary structure */
	ret = this->populate_boundary(topo);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* now populate the faces structure */
	ret = this->populate_faces(topo);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* success */
	return 0;
}
		
int node_boundary_t::get_boundary_neighbors(octnode_t* node,
					vector<octnode_t*>& neighs) const
{
	octneighbors_t edges;
	size_t fi;
	int ret;

	/* clear any existing values in neighs container */
	neighs.clear();

	/* find the node in our structure */
	ret = this->boundary.get(node, edges);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* store all neighbors */
	for(fi = 0; fi < NUM_FACES_PER_CUBE; fi++)
		edges.get(all_cube_faces[fi], neighs);

	/* success */
	return 0;
}
		
int node_boundary_t::writeobj(const string& filename) const
{
	int ret;

	/* use the stored topology to write the file */
	ret = this->writeobj_cliques(filename); // TODO this->boundary.writeobj(filename);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}

int node_boundary_t::writeobj_cliques(const std::string& filename) const
{
	map<node_face_t, node_face_info_t>::const_iterator fit, nfit;
	set<node_face_t>::const_iterator nit;
	map<node_face_t, size_t> index_map;
	ofstream outfile;
	vector<node_face_t> intersect;
	vector<node_face_t>::iterator eit;
	size_t num_verts;

	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1;

	/* write out the centers of all found faces */
	num_verts = 0;
	for(fit = this->faces.begin(); fit != this->faces.end(); fit++)
	{
		/* store index.  Note that OBJ files index
		 * starting at 1, so should we. */
		index_map[fit->first] = ++num_verts;
		outfile << "v " << fit->first.node->center.transpose()
		        << endl;
	}

	/* find all the cliques */
	for(fit = this->faces.begin(); fit != this->faces.end(); fit++)
	{
		/* iterate over the neighbors of this face */
		for(nit = fit->second.neighbors.begin();
				nit != fit->second.neighbors.end(); nit++)
		{
			/* ignore self-neighbors */
			if(*nit == fit->first)
			{
				cerr << "[node_boundary_t::"
				     << "writeobj_cliques]\tFound selfcycle"
				     << endl;
				continue;
			}

			/* get the neighbors of this neighbor */
			nfit = this->faces.find(*nit);

			/* check for the intersection of the two neighbor
			 * sets */
			intersect.resize(max(fit->second.neighbors.size(),
					nfit->second.neighbors.size()));
			eit = std::set_intersection(
					fit->second.neighbors.begin(),
					fit->second.neighbors.end(),
					nfit->second.neighbors.begin(),
					nfit->second.neighbors.end(),
					intersect.begin());
			intersect.resize(eit - intersect.begin());

			/* iterate over the intersection */
			for(eit = intersect.begin(); 
					eit != intersect.end(); eit++)
			{
				/* ignore degenerate triangles */
				if(*eit == *nit || *eit == fit->first)
					continue;

				/* export triangle between these
				 * three points */
				outfile << "f " << index_map[*eit] << " "
					<< index_map[nfit->first] << " "
					<< index_map[fit->first] << endl;
			}
		}
	}

	/* clean up */
	outfile.close();
	return 0;
}

/*----------------------------------*/
/* node_boundary_t helper functions */
/*----------------------------------*/


int node_boundary_t::populate_boundary(const octtopo::octtopo_t& topo)
{
	map<octnode_t*, octneighbors_t>::const_iterator it;
	vector<octnode_t*> neighs;
	size_t f, i, n;
	bool found_exterior;
	int ret;

	/* iterate through the nodes in this tree, storing
	 * the boundary nodes */
	for(it = topo.begin(); it != topo.end(); it++)
	{
		/* ignore exterior nodes */
		if(!(octtopo_t::node_is_interior(it->first)))
			continue;

		/* check all neighbors of this node */
		found_exterior = false;
		for(f = 0; f < NUM_FACES_PER_CUBE && !found_exterior; f++)
		{
			/* get the neighbors for this face */
			neighs.clear();
			it->second.get(octtopo::all_cube_faces[f], neighs);

			/* check if facing some null space */
			if(neighs.empty())
			{
				/* if a face has no neighbors,
				 * then this node is next to null
				 * space, which is exterior */
				found_exterior = true;
				break;
			}

			/* check if any of this node's neighbors
			 * are exterior.  If so, then this is a 
			 * boundary node.
			 *
			 * Alternatively, if this node has no neighbors
			 * at all, then is also a boundary, since null
			 * space is assumed to be exterior. */
			n = neighs.size();
			for(i = 0; i < n && !found_exterior; i++)
				if(!octtopo_t::node_is_interior(neighs[i]))
					found_exterior = true;
		}
	
		/* check if current node is a boundary */
		if(found_exterior)
		{
			/* it is a boundary node, copy its
			 * structure to this object */
			ret = this->boundary.add(it->first,
					it->second);
			if(ret)
				return PROPEGATE_ERROR(-1, ret);
		}
	}

	/* success */
	return 0;
}
		
int node_boundary_t::populate_faces(const octtopo_t& topo)
{
	map<octnode_t*, octneighbors_t>::const_iterator it;
	multimap<octnode_t*, node_face_t> node_face_map;
	pair<map<node_face_t, node_face_info_t>::iterator, bool> ins;
	node_face_t face;
	vector<octnode_t*> neighs;
	size_t f, i, n;
	bool found_exterior;
	int ret;

	/* now that we've stored the bounary nodes, we can determine the
	 * set of boundary faces.  Each boundary node can contribute one
	 * or more boundary faces. */
	for(it = this->boundary.begin(); it != this->boundary.end(); it++)
	{
		/* iterate over faces, looking for exterior neighbors */
		for(f = 0; f < NUM_FACES_PER_CUBE; f++)
		{
			/* prepare properties of the current face */
			face.node = it->first;
			face.f = octtopo::all_cube_faces[f];
			
			/* get nodes neighboring on this face */
			neighs.clear();
			it->second.get(face.f, neighs);
			
			/* if the neighs list is empty, that means that
			 * this node is abutting null space.  We want to
			 * count this as a boundary, since null space
			 * is considered exterior.  Thus, we'll add
			 * a null octnode to the list in this eventuality
			 * to account for it */
			if(neighs.empty())
				neighs.push_back(NULL);

			/* check for external nodes */
			found_exterior = false;
			n = neighs.size();
			for(i = 0; i < n; i++)
			{
				/* ignore if interior */
				if(octtopo_t::node_is_interior(neighs[i]))
					continue;
				found_exterior = true; /* i'th neighbor
				                        * exterior */

				/* store face info */
				ins = this->faces.insert(pair<node_face_t,
						node_face_info_t>(
						face, node_face_info_t()));
				if(ins.second)
				{
					/* newly inserted info, we need
					 * to initialize it to this face */
					ins.first->second.init(face);
				}

				/* add this opposing node to face info */
				ins.first->second.add(neighs[i]);

				/* keep track of all the faces generated
				 * for each node.
				 *
				 * We want to do this for both the current
				 * node and the opposing node, since both
				 * are connected to the face. */
				if(neighs[i] != NULL)
					node_face_map.insert(
						std::pair<octnode_t*,
							node_face_t>(
							neighs[i], face));
			}
			
			/* record current node in mapping */
			if(found_exterior)
				node_face_map.insert(
					std::pair<octnode_t*,node_face_t>(
						it->first, face));
		}
	}

	/* populate the linkage between nodes */
	ret = this->populate_face_linkages(topo, node_face_map);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}
		
int node_boundary_t::populate_face_linkages(const octtopo_t& topo,
				const multimap<octnode_t*,
				node_face_t>& node_face_map)
{
	map<node_face_t, node_face_info_t>::iterator fit;
	multimap<octnode_t*, node_face_t>::const_iterator it, nit;
	pair<multimap<octnode_t*, node_face_t>::const_iterator,
		multimap<octnode_t*, node_face_t>::const_iterator> range;
	octnode_t* node;
	node_face_t face;
	CUBE_FACE oppface, joiningface;
	octneighbors_t edges;
	vector<octnode_t*> neighs;
	size_t fi, i, n;
	int ret;

	/* populate linkages between faces using node_face_map
	 * faces should be neighbors iff one of the following:
	 *	- they share a node (and their f values are not same 
	 * 				or opposing)
	 * 	- they are on neighboring nodes and have same f value
	 */

	/* iterate all the nodes in question */
	for(it = node_face_map.begin(); it != node_face_map.end(); it++)
	{
		/* get the node and face at this entry */
		node = it->first;
		face = it->second;
		oppface = octtopo::get_opposing_face(face.f);

		/* verify that this face is a part of the mapping */
		fit = this->faces.find(face);
		if(fit == this->faces.end())
		{
			cerr << "[node_boundary_t::populate_face_linkages]"
			     << "\tUnable to find face: (" << face.node
			     << ", " << face.f << "), which is paired "
			     << "with node: " << node << endl;
			return -1;
		}

		/* get the other faces for this node */
		range = node_face_map.equal_range(node);
		for(nit = range.first; nit != range.second; nit++)
		{
			/* the face denoted by nit->second is a face on
			 * node.  If it is facing at a right angle from the
			 * current face, then it should be a neighbor */
			if(nit->second.f == face.f 
					|| nit->second.f == oppface)
				continue;

			/* check if the two faces share an edge */
			if(!(face.shares_edge_with(nit->second)))
				continue;

			/* these faces should be linked */
			fit->second.neighbors.insert(nit->second);
		}

		/* get the nodes that are neighbors to this node */
		ret = topo.get(node, edges);
		if(ret)
		{
			cerr << "[node_boundary_t::populate_face_linkages]"
			     << "\tCould not locate node: " << node << endl;
			return PROPEGATE_ERROR(-2, ret);
		}


		for(fi = 0; fi < NUM_FACES_PER_CUBE; fi++)
		{
			/* we only care about neighboring nodes if the
			 * touching face is orthogonal to the current face
			 */
			joiningface = octtopo::all_cube_faces[fi];
			if(joiningface == face.f || joiningface == oppface)
				continue;

			/* get nodes neighboring on this face */
			neighs.clear();
			edges.get(joiningface, neighs);
			n = neighs.size();

			/* iterate over the neighboring nodes */
			for(i = 0; i < n; i++)
			{
				/* get the boundary faces for this
				 * neighboring node */
				range = node_face_map.equal_range(
							neighs[i]);
				for(nit = range.first; 
						nit != range.second; nit++)
				{
					/* ignore this neighboring face
					 * if it is not facing the same
					 * direction as the current
					 * face */
					if(nit->second.f != face.f)
						continue;
			
					/* check if the two faces share 
					 * an edge */
					if(!(face.shares_edge_with(
							nit->second)))
						continue;


					/* add this face as a neighbor */
					fit->second.neighbors.insert(
							nit->second);
				}
			}
		}
	}

	/* success */
	return 0;
}

/*--------------------------------------*/
/* node_face_t function implementations */
/*--------------------------------------*/

bool node_face_t::shares_edge_with(const node_face_t& other) const
{
	octtopo::CUBE_FACE oppf;
	double ax[2];
	double ay[2];
	double bx[2];
	double by[2];
	double hw=0.0, ohw=0.0;
	Vector3d norm(0,0,0), othernorm(0,0,0);

	/* get the cube side that this face resides on */
	oppf = octtopo::get_opposing_face(this->f);
	if(oppf == other.f)
		return false; /* opposing faces cannot share an edge */
	
	/* the following initialize the variables that may
	 * be used */
	ax[0] = ax[1] = ay[0] = ay[1] = bx[0] = bx[1] = by[0] = by[1] = 0.0;

	/* check for case where they face the same direction */
	if(this->f == other.f)
	{
		/* face the same direction, problem is now 2D */
		hw  = this->node->halfwidth;
		ohw = other.node->halfwidth;
		switch(this->f)
		{
			case FACE_XMINUS:
			case FACE_XPLUS:
				ax[0] = this->node->center(1)-hw;
				ax[1] = this->node->center(1)+hw;
				ay[0] = this->node->center(2)-hw;
				ay[1] = this->node->center(2)+hw;
				bx[0] = other.node->center(1)-ohw;
				bx[1] = other.node->center(1)+ohw;
				by[0] = other.node->center(2)-ohw;
				by[1] = other.node->center(2)+ohw;
				break;

			case FACE_YMINUS:
			case FACE_YPLUS:
				ax[0] = this->node->center(2)-hw;
				ax[1] = this->node->center(2)+hw;
				ay[0] = this->node->center(0)-hw;
				ay[1] = this->node->center(0)+hw;
				bx[0] = other.node->center(2)-ohw;
				bx[1] = other.node->center(2)+ohw;
				by[0] = other.node->center(0)-ohw;
				by[1] = other.node->center(0)+ohw;
				break;

			case FACE_ZMINUS:
			case FACE_ZPLUS:
				ax[0] = this->node->center(0)-hw;
				ax[1] = this->node->center(0)+hw;
				ay[0] = this->node->center(1)-hw;
				ay[1] = this->node->center(1)+hw;
				bx[0] = other.node->center(0)-ohw;
				bx[1] = other.node->center(0)+ohw;
				by[0] = other.node->center(1)-ohw;
				by[1] = other.node->center(1)+ohw;
				break;
		}

		/* determine if edges touch */
		return poly2d::aabb_pair_abut(ax,ay,bx,by,APPROX_ZERO); 
	}
	
	/* the faces are orthogonal to each other.  We can determine
	 * if these touch based on analysis corresponding to each
	 * face's normal. */
	octtopo::cube_face_normals(this->f, norm);
	octtopo::cube_face_normals(other.f, othernorm);

	// TODO left off here ... this over-connects
	return true;
}

/*-------------------------------------------*/
/* node_face_info_t function implementations */
/*--------------------------------------------*/

void node_face_info_t::get_subface_center(size_t i,Eigen::Vector3d& p) const
{
	octnode_t* opp;
	CUBE_FACE f;
	double hw;

	/* get opposing node to this subface */
	opp = this->exterior_nodes[i];

	/* determine starting node */
	if(opp == NULL || opp->halfwidth > this->face.node->halfwidth)
	{
		/* the interior node dictates position */
		f = this->face.f;
		hw = this->face.node->halfwidth;
		p = this->face.node->center;
	}
	else
	{
		/* the exterior node dictates position */
		f = octtopo::get_opposing_face(this->face.f);
		hw = opp->halfwidth;
		p = opp->center;
	}

	/* get face center position based on node center position */
	switch(f)
	{
		case FACE_ZMINUS:
			p(2) -= hw;
			break;
		case FACE_YMINUS:
			p(1) -= hw;
			break;
		case FACE_XMINUS:
			p(0) -= hw;
			break;
		case FACE_XPLUS:
			p(0) += hw;
			break;
		case FACE_YPLUS:
			p(1) += hw;
			break;
		case FACE_ZPLUS:
			p(2) += hw;
			break;
	}
}

double node_face_info_t::get_subface_halfwidth(size_t i) const
{
	Vector3d p;
	octnode_t* opp;

	/* get opposing node to this subface */
	opp = this->exterior_nodes[i];

	/* whichever node (either the interor or exterior) that
	 * borders this face has the smaller halfwidth will
	 * dictate the halfwidth of this face */
	if(opp == NULL)
		return this->face.node->halfwidth;
	else if(this->face.node->halfwidth < opp->halfwidth)
		return this->face.node->halfwidth;
	return opp->halfwidth;
}

void node_face_info_t::writeobj(std::ostream& os) const
{
	Vector3d p;
	size_t i, n;

	/* iterate over subfaces */
	n = this->num_subfaces();
	for(i = 0; i < n; i++)
	{
		/* get the face geometry */
		this->get_subface_center(i, p);

		/* write it */
		os << "v " << p.transpose() << " 0 255 0" << endl;
	}
}	
