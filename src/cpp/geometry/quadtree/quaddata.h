#ifndef QUADDATA_H
#define QUADDATA_H

/**
 * @file   quaddata.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file contains classes used to define data in quadtrees
 *
 * @section DESCRIPTION
 *
 * This file defines the quaddata_t structure.  These are the
 * data elements stored at the leafs of quadtrees.
 */

#include <Eigen/Dense>
#include <iostream>
#include <set>

/**
 * this class represents the data that are stored in the nodes of
 * the quad tree.  This is only interesting at the leaves.
 */
class quaddata_t
{
	/* parameters */
	public:

		/**
		 * The average position of the samples
		 * in the cell this data structure represents
		 */
		Eigen::Vector2d average; /* the average position */
		
		/**
		 * The average normal vector of this cell
		 *
		 * This is not assumed to be normalized
		 */
		Eigen::Vector2d normal;

		/**
		 * The total weight of all points in this data structure
		 */
		double total_weight; /* number of points added */

		/**
		 * The set of pose indices that observe this cell
		 */
		std::set<size_t> pose_inds;
	
		/**
		 * The minimum and maximum elevations for the
		 * points in this cell
		 */
		double min_z, max_z;

	/* functions */
	public:
	
		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Constructs empty data structure
		 */
		quaddata_t();

		/**
		 * Frees all memory and resources
		 */
		~quaddata_t();

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Adds point to this structure
		 *
		 * Incorporates a point into this data structure,
		 * updating fields.
		 *
		 * @param p   The point to add
		 * @param n   The normal to add
		 * @param w   The weight of these points 
		 */
		void add(const Eigen::Vector2d& p,
			const Eigen::Vector2d& n, double w);

		/**
		 * Adds height ranges to this cell
		 *
		 * Will set the height range of this cell
		 * to be the union of the existing range and
		 * the argument min/max z values.
		 *
		 * @param miz   Minimum z
		 * @param maz   Maximum z
		 */
		void add(double miz, double maz);

		/**
		 * Creates a deep copy of this object
		 *
		 * Allocates new memory that is a deep clone of
		 * this data object.
		 *
		 * @return  Newly allocated clone
		 */
		quaddata_t* clone() const;

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Exports this object to the given stream
		 *
		 * Prints the information in this struct to the
		 * given stream.
		 *
		 * Does NOT print a new line.
		 *
		 * format:
		 *
		 * 	<x> <y> <min_z> <max_z> <num_pts> 
		 * 			<num_poses> <pose1> <pose2> ...
		 *
		 * @param os   The output stream to write to
		 */
		void print(std::ostream& os) const;
};
