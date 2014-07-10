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
		double surface_sum; /* sum of surface prob. observations */
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

		/* the following definitions are used in computing the
		 * statistics for a given node's data */
		static constexpr double UNOBSERVED_PROBABILITY = 0.5;
		static constexpr double MAXIMUM_VARIANCE       = 1.0;

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
		octdata_t(double prob_samp, double surface_samp=0.0,
		          double corner_samp=0.0, double planar_samp=0.0);

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
		 * @param surf     The observed surface probability
		 * @param corner   The observed corner coefficient
		 * @param planar   The observed planarity coefficient
		 */
		void add_sample(double prob, double surf=0.0,
		                double corner=0.0, double planar=0.0);

		/**
		 * Returns the count of number of observations seen
		 *
		 * @return  The count of observations seen by this object.
		 */
		inline unsigned int get_count() const
		{ return this->count; };

		/**
		 * Returns the best estimate of recorded probability
		 *
		 * Will average all samples seen, and return the best
		 * estimate for the probability value stored in this
		 * data object.
		 *
		 * @return   Returns the probability stored in this object
		 */
		inline double get_probability() const
		{
			/* check that we have any observations */
			if(this->count > 0)
				return (this->prob_sum / this->count);
			
			/* if unobserved, assume unknown */
			return UNOBSERVED_PROBABILITY; 
		};

		/**
		 * Returns the uncertainty of the probability estimate
		 *
		 * Will return the variance of the samples collected
		 * so far of the probability value in this data object.
		 * Variance is the square of the standard deviation.
		 *
		 * @return   Returns the variance of the prob. estimate
		 */
		inline double get_uncertainty() const
		{
			double m, m2, n;

			/* check if we have observed any samples */
			n = this->count;
			if(n <= 1) /* don't have multiple samples */
				return MAXIMUM_VARIANCE;
				            /* maximum uncertainty for
				             * values that are restricted
				             * to a range of [0,1] */

			/* get unbiased estimate of the variance, 
			 * by using Bessel's correction: */
			m = this->prob_sum; /* sum of samples */
			m2 = this->prob_sum_sq; /* sum of squared samples */
			return (m2 - (m*m/n)) / (n-1);
		};
		
		/**
		 * Returns best estimate of whether this node is interior
		 *
		 * Will use the recorded probability to judge if the
		 * node that contains these data should be identified
		 * as interior or exterior.
		 *
		 * @return  Returns true iff data indicates interior label
		 */
		inline bool is_interior() const
		{ return (this->get_probability() > 0.5); };

		/**
		 * Returns best estimate of whether this node is an object
		 *
		 * An 'object' is represented by exterior nodes that
		 * are contained within the extruded floorplan.  These
		 * nodes should be objects such as furniture, countertops,
		 * ceiling beams, etc.
		 *
		 * @return  Returns true iff data indicates object node
		 */
		inline bool is_object() const
		{ 
			return !(this->is_interior() 
				|| this->get_fp_room() < 0);
		};

		/**
		 * Returns the average surface probability observation
		 */
		inline double get_surface_prob() const
		{
			if(this->count == 0)
				return 0;
			return ((this->surface_sum) / this->count);
		};

		/**
		 * Returns the average planar probability observation
		 */
		inline double get_planar_prob() const
		{
			if(this->count == 0)
				return 0;
			return ((this->planar_sum) / this->count);
		};

		/**
		 * Returns the average corner probability observation
		 */
		inline double get_corner_prob() const
		{
			if(this->count == 0)
				return 0;
			return ((this->corner_sum) / this->count);
		};

		/**
		 * Gets the floor plan room number of this data object.
		 *
		 * If no room index has been assigned, the value will
		 * be negative.
		 *
		 * @return    Returns the global room number index
		 */
		inline int get_fp_room() const
		{ return (this->fp_room); };

		/**
		 * Sets the floor plan room number of this data object
		 *
		 * @param r   The index of the room that contains these data
		 */
		inline void set_fp_room(int r)
		{ this->fp_room = r; };
};

#endif
