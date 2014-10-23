#include "tree_exporter.h"
#include <io/mesh/mesh_io.h>
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octdata.h>
#include <geometry/octree/octtopo.h>
#include <mesh/partition/node_partitioner.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region_graph.h>
#include <mesh/surface/node_corner.h>
#include <mesh/surface/face_mesher.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <Eigen/Dense>

/**
 * @file tree_exporter.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains function implementations that are used
 * to export information stored in an octree_t to various
 * formats for visualization purposes.
 */

using namespace std;
using namespace Eigen;

/*--------------------------*/
/* function implementations */
/*--------------------------*/
	
int tree_exporter::export_dense_mesh(const std::string& filename,
	                      const octree_t& tree)
{
	octtopo::octtopo_t top;
	node_boundary_t boundary;
	face_mesher_t mesher;
	tictoc_t clk;
	int ret;

	/* initialize the octree topology */
	tic(clk);
	ret = top.init(tree);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	toc(clk, "Initializing topology");

	/* extract the boundary nodes using the generated topology */
	ret = boundary.populate(top);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* generate mesh from this geometry */
	tic(clk);
	ret = mesher.add(tree, boundary);
	if(ret)
		return PROPEGATE_ERROR(-3, ret);
	toc(clk, "Generating mesh");

	/* export the mesh */
	tic(clk);
	ret = mesher.get_mesh().write(filename);
	if(ret)
		return PROPEGATE_ERROR(-4, ret);
	toc(clk, "Exporting mesh");

	/* success */
	return 0;
}
	
int tree_exporter::export_node_faces(const string& filename,
                                     const octree_t& tree,
                                     node_boundary_t::SEG_SCHEME scheme)
{
	octtopo::octtopo_t top;
	node_boundary_t boundary;
	facemap_t::const_iterator it;
	node_corner::corner_t corner;
	Vector3d p;
	mesh_io::mesh_t mesh;
	mesh_io::vertex_t vertex;
	mesh_io::polygon_t poly;
	map<node_corner::corner_t, size_t> corner_index_map;
	pair<map<node_corner::corner_t, size_t>::iterator, bool> ins;
	tictoc_t clk;
	size_t ci;
	int ret;

	/* initialize the octree topology */
	tic(clk);
	ret = top.init(tree);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	toc(clk, "Initializing topology");

	/* extract the boundary nodes using the generated topology */
	ret = boundary.populate(top, scheme);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* iterate through the faces, recording corner positions */
	tic(clk);
	for(it = boundary.begin(); it != boundary.end(); it++)
	{
		/* clear polygon info for this face */
		poly.clear();

		/* get the corners for this face */
		for(ci = 0; ci < node_corner::NUM_CORNERS_PER_SQUARE; ci++)
		{
			/* add this corner as a vertex to the mesh */ 
			corner.set(tree, it->first, ci);
			corner.get_position(tree, p);
			vertex.x = p(0);
			vertex.y = p(1);
			vertex.z = p(2);

			/* record the index of this corner in the mesh.
			 *
			 * If the corner already exists in the mesh, then
			 * this add will fail (which is what we want). */
			ins = corner_index_map.insert(
					pair<node_corner::corner_t,
					size_t>(corner, mesh.num_verts()));

			/* add this corner to the polygon */
			mesh.add(vertex);
			poly.vertices.push_back(ins.first->second);
		}

		/* we want the normal of the polygon to face into
		 * the interior of the model, so we may need to
		 * flip the ordering based on the face in question */
		if(it->first.exterior == NULL 
				|| it->first.interior->halfwidth
				<= it->first.exterior->halfwidth)
			/* orient to face inwards */
			std::reverse(poly.vertices.begin(), 
					poly.vertices.end());

		/* now that we've added the corners to the mesh for this
		 * face, we can add the polygon of this face to the mesh */
		mesh.add(poly);
	}
	toc(clk, "Preparing mesh");

	/* write the mesh to disk */
	tic(clk);
	ret = mesh.write(filename);
	if(ret)
		return PROPEGATE_ERROR(-3, ret);
	toc(clk, "Exporting mesh");

	/* success */
	return 0;
}
	
int tree_exporter::export_regions(const std::string& filename,
					const octree_t& tree,
					node_boundary_t::SEG_SCHEME scheme)
{
	octtopo::octtopo_t top;
	node_boundary_t boundary;
	planar_region_graph_t region_graph;
	tictoc_t clk;
	int ret;

	/* initialize the octree topology */
	tic(clk);
	ret = top.init(tree);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	toc(clk, "Initializing topology");

	/* extract the boundary nodes using the generated topology */
	ret = boundary.populate(top, scheme);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* form planar regions from these boundary faces */
	tic(clk);
	ret = region_graph.populate(boundary);
	if(ret)
		return PROPEGATE_ERROR(-3, ret);
	toc(clk, "Forming regions");

	/* coalesce regions */
	tic(clk);
	region_graph.init(0.5, 2.0, false); // TODO debugging
	ret = region_graph.coalesce_regions();
	if(ret)
		return PROPEGATE_ERROR(-4, ret);
	toc(clk, "Coalesce regions");

	/* export regions to file */
	tic(clk);
	ret = region_graph.writeobj(filename);
	if(ret)
		return PROPEGATE_ERROR(-5, ret);
	toc(clk, "Writing OBJ");

	/* success */
	return 0;
}

/**
 * Helper function used to export leaf center to OBJ file
 *
 * This helper function is recursive, and is called by
 * tree_exporter::export_leafs_to_obj().
 *
 * @param os     The output file stream to write to
 * @param node   The node to analyze and export recursively
 */
void export_leafs_to_obj_recur(ostream& os, const octnode_t* node)
{
	unsigned int i, red, green, blue;
	double p;

	/* check if this node is a leaf (i.e. it has data) */
	if(node->data != NULL)
	{
		/* get characteristic to visualize */
		p = node->data->get_probability();
		if(p > 1)
			p = 1;
		if(p < 0)
			p = 0;

		/* assign colors */
		green = (unsigned int) (100 * (1 - (2*fabs(p-0.5))));
		if(p > 0.5)
		{
			/* color as interior */
			red = 0;
			blue = (unsigned int) (255 * p);
		}
		else
		{
			/* color as exterior */
			red = (unsigned int) (255 * (1-p));
			blue = 0;
		}

		/* export this leaf */
		os << "v " << node->center(0)
		   <<  " " << node->center(1)
		   <<  " " << node->center(2)
		   <<  " " << red
		   <<  " " << green
		   <<  " " << blue
		   <<  " # probability: " << p
		   << endl;
	}

	/* check if leaf */
	if(node->isleaf())
		return;

	/* recurse through the node's children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(node->children[i] != NULL)
			export_leafs_to_obj_recur(os, node->children[i]);
		else if(node->data != NULL)
		{
			/* export temp node to where child would have 
			 * been */
			Vector3d cp = relative_child_pos(i)
					*node->halfwidth/2
					+ node->center;
			os << "v " << cp(0) << " " << cp(1) << " " << cp(2)
			   << " 255 255 0" << endl;
		}
}

int tree_exporter::export_leafs_to_obj(const string& filename,
                                       const octree_t& tree)
{
	ofstream outfile;
	tictoc_t clk;
	
	/* time this action */
	tic(clk);

	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1; /* could not open file */

	/* add some header information */
	outfile << "# This file generated by tree_exporter" << endl
	        << "#" << endl
	        << "# The contents are a list of vertices, which" << endl
		<< "# denote the 3D positions of centers of leaf" << endl
		<< "# nodes of an octree, colored based on the" << endl
		<< "# data stored in that tree." << endl << endl;

	/* export to file */
	export_leafs_to_obj_recur(outfile, tree.get_root());

	/* clean up */
	outfile.close();
	toc(clk, "Exporting octree leafs to OBJ");
	return 0;
}

int tree_exporter::export_corners_to_obj(const std::string& filename,
	                          const octree_t& tree)
{	
	ofstream outfile;
	stringstream ss;
	octtopo::octtopo_t top;
	node_boundary_t boundary;
	facemap_t::const_iterator it;
	node_corner::corner_t corner;
	tictoc_t clk;
	size_t i;
	int ret;
	
	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1; /* could not open file */

	/* add some header information */
	outfile << "# This file generated by tree_exporter" << endl
	        << "#" << endl
	        << "# The contents are a list of vertices, which" << endl
		<< "# denote the 3D positions of corners of leaf" << endl
		<< "# nodes of an octree, colored based on the" << endl
		<< "# corner index." << endl << endl;

	/* initialize the octree topology */
	tic(clk);
	ret = top.init(tree);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	toc(clk, "Initializing topology");

	/* extract the boundary nodes using the generated topology */
	ret = boundary.populate(top);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);
	
	/* export corners to file */
	tic(clk);
	ss.str("");

	/* iterate over faces */
	for(it = boundary.begin(); it != boundary.end(); it++)
	{
		/* export the corners of this face */
		for(i = 0; i < node_corner::NUM_CORNERS_PER_SQUARE; i++)
		{
			/* get this corner */
			corner.set(tree, it->first, i);
			corner.writeobj(ss, tree);
		}
	}

	/* clean up */
	outfile << ss.str();
	outfile.close();
	toc(clk, "Exporting octree corners to OBJ");
	return 0;
}
	
/**
 * Helper function used to export exterior cubes to OBJ file
 *
 * This helper function is recursive, and is called by
 * tree_exporter::export_exterior_cubes_to_obj().
 *
 * @param os     The output file stream to write to
 * @param node   The node to analyze and export recursively
 */
void export_exterior_cubes_to_obj_recur(ostream& os, const octnode_t* node)
{
	unsigned int i, r, g, b;
	double hw;
	int cc[8][3] = { /* this list indicates corner corner pos */
			{ 1, 1, 1},
			{ 1,-1, 1},
			{-1,-1, 1},
			{-1, 1, 1},
			{ 1, 1,-1},
			{ 1,-1,-1},
			{-1,-1,-1},
			{-1, 1,-1}	};

	/* check if this node is a leaf (i.e. it has data) */
	if(node->data != NULL && node->data->is_object())
	{
		/* export cube of this leaf */

		/* color appropriately by the data count */
		r = (node->data->get_count() == 0 ? 255 : 0);
		g = (node->data->get_count() == 0 ? 0 : 255);
		b = (int) (255.0*node->data->get_probability());

		/* vertices of cube */
		hw = node->halfwidth;
		for(i = 0; i < 8; i++)
			os << "v " << (node->center(0)+cc[i][0]*hw)
			   <<  " " << (node->center(1)+cc[i][1]*hw)
			   <<  " " << (node->center(2)+cc[i][2]*hw)
			   <<  " " << r << " " << g << " " << b
			   << endl;
		
		/* faces of the cube */
		os << "f -1 -4 -3 -2" << endl
		   << "f -5 -6 -7 -8" << endl
		   << "f -2 -3 -7 -6" << endl
		   << "f -1 -5 -8 -4" << endl
		   << "f -3 -4 -8 -7" << endl
		   << "f -6 -5 -1 -2" << endl;
	}

	/* recurse through the node's children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(node->children[i] != NULL)
			export_exterior_cubes_to_obj_recur(os,
					node->children[i]);
}

int tree_exporter::export_exterior_cubes_to_obj(const string& filename,
                                                const octree_t& tree)
{
	ofstream outfile;
	tictoc_t clk;

	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1; /* could not open file */

	/* add some header information */
	outfile << "# This file generated by tree_exporter" << endl
	        << "#" << endl
	        << "# The contents are a set of cubes, which" << endl
		<< "# denote the 3D positions of exterior leaf" << endl
		<< "# nodes of an octree, colored based on the" << endl
		<< "# data stored in that tree." << endl << endl;

	/* export to file */
	tic(clk);
	export_exterior_cubes_to_obj_recur(outfile, tree.get_root());
	toc(clk, "Exporting exterior cubes");

	/* clean up */
	outfile.close();
	return 0;
}

/**
 * This helper function is used to recursively go through the tree
 *
 * This function is called by the export_stats_to_txt() function,
 * and is in charge of writing each leaf to the stream.
 */
void export_stats_to_txt_recur(ostream& os, const octnode_t* node)
{
	unsigned int i;
	double p, uc;

	/* check if this node is a leaf (i.e. it has data) */
	if(node->data != NULL)
	{
		/* get characteristic to visualize */
		p = node->data->get_probability();
		if(p > 1)
			p = 1;
		if(p < 0)
			p = 0;
		
		/* get uncertainty */
		uc = node->data->get_uncertainty();

		/* export this leaf */
		os << p << " " << uc << endl;
	}

	/* recurse through the node's children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(node->children[i] != NULL)
			export_stats_to_txt_recur(os, node->children[i]);
}

int tree_exporter::export_stats_to_txt(const std::string& filename,
                                       const octree_t& tree)
{
	ofstream outfile;
	
	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1; /* could not open file */
	
	/* recursively export nodes */
	export_stats_to_txt_recur(outfile, tree.get_root());

	/* clean up */
	outfile.close();
	return 0;
}
