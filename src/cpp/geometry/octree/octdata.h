#ifndef OCTDATA_H
#define OCTDATA_H

/**
 * @file octdata.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This is the class used to store data in an octree_t.  Arbitrary
 * data can be stored in the octree by extending this class and adding
 * these to the tree.
 */

#include <iostream>

/* this class represents the data that are stored in the nodes of
 * the octree.  This is only interesting at the leaves. */
class octdata_t
{
	/* parameters */
	private:

		/* the following values are used to track statistical
		 * samples of the corresponding node to these data */
		unsigned int count; /* number of observed samples */
		double prob_sum; /* sum of probability samples */
		double prob_sum_sq; /* sum of square of prob samples */

		/* the following values are used to estimate geometric
		 * properties of this voxel, such as flatness, curviture,
		 * or corner detection */
		double corner_sum; /* sum of corner estimates for node */
		double planar_sum; /* sum of flatness estimates for node */

		/* the following values relate to any imported floorplans,
		 * which associate this node to a room within the floor
		 * plans.  A negative value indicates that it intersected
		 * no rooms. */
		int fp_room;

		/* This value is set to true only if this node intersects
		 * an original deterministic input scan.  It is used for
		 * debugging and comparison purposes to see the effect
		 * of the probabilistic scanning method compared to
		 * previous techniques. */
		bool is_carved;

	/* functions */
	public:

		/*########################################################
		 * The following functions are called by the octree class
		 *########################################################
		 */

		/* constructors */

		/**
		 * Initializes empty octdata object
		 */
		octdata_t();

		/**
		 * Initializes octdata object with a single sample
		 */
		octdata_t(double prob_samp, double corner_samp=0.0,
		          double planar_samp=0.0);

		/**
		 * Frees all memory and resources
		 */
		~octdata_t();

		/* accessors */

		/**
		 * Merges the given data into this object
		 *
		 * This will be called when data are to be inserted
		 * into a tree node that is already populated.  This
		 * function should be implemented so to be communitive.
		 * That is, it should be the case that X.merge(Y) results
		 * in X containing the same information as Y would contain
		 * if Y.merge(X) was called.
		 *
		 * @param p  The data to merge into this object
		 */
		void merge(octdata_t* p);

		/**
		 * Clones this object.
		 *
		 * Allocates new memory that is a deep clone of
		 * this data object.  This memory will eventually
		 * need to be freed.
		 *
		 * @return  Returns a deep-copy of this object.
		 */
		octdata_t* clone() const;

		/* i/o */

		/**
		 * Serializes these data to binary stream
		 *
		 * Will write all necessary data to the binary stream
		 * so that the information in this object can be
		 * fully recovered.
		 *
		 * @param os  The output binary stream to write to
		 */
		void serialize(std::ostream& os) const;

		/**
		 * Parses stream to populate this object
		 *
		 * Will read the input binary stream and parse
		 * the object represented.  This should be in the
		 * same format as what is written by the serialize()
		 * function.
		 *
		 * @param is  The input binary stream to read from
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int parse(std::istream& is);

		/*
		 *########################################################
		 * The remaining functions will not be called by octree_t
		 *########################################################
		 */

		/**
		 * Adds a carving observation to this data object
		 *
		 * Will increment the observation count, and update
		 * the appropriate sums based on this observation.
		 *
		 * @param prob     The observed carved probability
		 * @param corner   The observed corner coefficient
		 * @param planar   The observed planarity coefficient
		 */
		void add_sample(double prob, double corner=0.0, 
		                double planar=0.0);
};

#endif
