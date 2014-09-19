#ifndef SOF_IO_H
#define SOF_IO_H

/**
 * @file    sof_io.h
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Functionality to export octrees to SOF files
 *
 * @section DESCRIPTION
 *
 * This file contains functions used to export octree information
 * to a SOF (Signed Octree Format) file.  This file is used for Tao
 * Ju's Dual Contour and Poly Mender code.
 *
 * @section FORMAT
 *
 * The following is his documentation on this format:
 *
 * IV SOF format
 *
 * SOF (Signed Octree Format) records an octree grid with signes attached
 * to the 8 corners of each leaf node. All leaf nodes appear at the same
 * depth that is specified by the <octree_depth> argument to the program.
 * The tree is recorded in SOF file using pre-order traversal. Here is 
 * the structure of a SOF file (binary):
 *
 * <header>
 *
 * <node>
 *
 * <header> is a 4-bytes integer that equals 2 ^ octree_depth. The first
 * byte of a <node> is either 0 (denoting an intermediate node) or 1 
 * (denoting an empty node) or 2 (denoting a leaf node). After the first 
 * byte, an intermediate node <node> contains eight <node> structures for 
 * its eight children; an empty node <node> contains one byte of value 0 
 * or 1 denoting if it is inside or outside; and a leaf node contains one
 * byte whose eight bits correspond to the signs at its eight corners (0
 * for inside and 1 for outside). The order of enumeration of the eight 
 * children nodes in an intermediate nodeis the following (expressed in 
 * coordinates <x,y,z> ): <0,0,0>,<0,0,1>,<0,1,0>,<0,1,1>,<1,0,0>,<1,0,1>,
 * <1,1,0>,<1,1,1>. The enumeration of the eight corners in a leaf node 
 * follows the same order (e.g., the lowest bit records the sign at 
 * <0,0,0>).
 * 
 * @section REFERENCES
 *
 * http://www1.cse.wustl.edu/~taoju/code/polymender.htm
 * http://sourceforge.net/projects/dualcontouring/
 */

#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <iostream>
#include <string>

/**
 * The sof_io class houses functions used to export octrees to .sof files
 *
 * The .sof (Signed Octree Format) was developed by Tao Ju for his
 * Dual Contouring and PolyMender code.
 */
class sof_io
{
	/* functions */
	public:

		/**
		 * Exports the given octree to a .sof file
		 *
		 * Given a populated octree, will export it to a
		 * .sof (Signed Octree Format) file.
		 *
		 * @param tree       The tree to export
		 * @param filename   Where to write the .sof file
		 *
		 * @return    Returns zero on success, non-zero on failure
		 */
		static int writesof(const octree_t& tree,
				const std::string& filename);

		/**
		 * Exports the given octree to a .sog file
		 *
		 * Given a populated octree, will export it to a
		 * .sog (Signed Octree with Geometry) file.
		 *
		 * @param tree      The tree to export
		 * @param filename  Where to write the .sog file
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		static int writesog(const octree_t& tree,
				const std::string& filename);

	/* helper functions */
	private:

		/**
		 * Writes header information to the .sof file stream
		 *
		 * @param tree  The tree to export
		 * @param os    The output stream for the .sof file
		 *
		 * @return   Returns zero on success, non-zero on failure
		 */
		static int writesof_header(const octree_t& tree,
				std::ostream& os);

		/**
		 * Writes node information to the .sof file stream
		 *
		 * Will recursively write this node and its subnodes
		 * to the given .sof file stream.
		 *
		 * @param node    The node to export
		 * @param os      The stream to export to
		 *
		 * @return   Returns zero on success, non-zero on failure
		 */
		static int writesof_node(const octnode_t* node,
				std::ostream& os);

		/**
		 * Writes header information for the .sog file stream
		 *
		 * @param tree   The tree to export
		 * @param os     The output stream for the .sog file
		 *
		 * @return    Return zero on success, non-zero on failure
		 */
		static int writesog_header(const octree_t& tree,
				std::ostream& os);

		/**
		 * Writes node information to the .sog file stream
		 *
		 * Will recursively write this node and its subnodes
		 * to the given .sog file stream.
		 *
		 * @param node    The node to export
		 * @param os      The stream to export to
		 *
		 * @return        Returns zero on success, non-zero on
		 *                failure.
		 */
		static int writesog_node(const octnode_t* node,
				std::ostream& os);
};

#endif
