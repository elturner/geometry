#ifndef HISTOGRAM_H
#define HISTOGRAM_H

/**
 * @file    histogram.h
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Performs a 1D histogram from input data
 *
 * @section DESCRIPTION
 *
 * This file defines a class that will construct a histogram of
 * scalar values at a specified bin-size / resolution.
 */

#include <map>
#include <vector>
#include <iostream>

/**
 * The histogram_t class represents a 1D histogram.
 *
 * The following class represents the histogram
 * which has dynamic ranging, but fixed bin size.
 */
class histogram_t
{
	/* parameters */
	private:

		/**
		 * The map of the histogram, from bins to weights.
		 *
		 * The histogram is stored as a map,
		 * where the keys represent discretized
		 * values given to the histogram, and the
		 * elements of the map are counts for
		 * the keys.
		 *
		 * Note that counts are allowed to be fractional,
		 * so are stored as doubles.
		 */
		std::map<int, double> hist;

		/**
		 * The resolution parameter is the width of each bin.
		 *
		 * To discretize the keys, a resolution must be specified.
		 */
		double res;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/
		
		/**
		 * Constructs an empty histogram with a default
		 * bin-size of 1 unit.
		 */
		histogram_t();

		/**
		 * Constructs an empty hisogram with the specified
		 * bin-size.
		 *
		 * The given resolution parameter represents
		 * the bin-size of the histogram to be created.
		 *
		 * @param r   The bin size of this histogram
		 */
		histogram_t(double r) : hist(), res(r)
		{};

		/**
		 * Frees all memory and resources.
		 */
		~histogram_t();

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Clears any info from the histogram.
		 *
		 * Resets the bin-size to be 1 unit.
		 */
		void clear();

		/**
		 * Specifies the resolution / bin-size parameter
		 *
		 * Resets the resolution of this histogram.  Will
		 * clear any existing values in the histogram.
		 *
		 * @param r     The resolution to use.  The sign of r
		 * 		will be ignored.
		 */
		void set_resolution(double r);

		/*---------------------*/
		/* accessors/modifiers */
		/*---------------------*/

		/**
		 * Will add a value to the histogram.
		 *
		 * Can optionally give a weight to this value.
		 * By default, the weight is one.
		 *
		 * @param v      The location of value to add
		 * @param w      Optional weight argument.
		 */
		void insert(double v, double w=1.0);

		/**
		 * Adds all of the given histogram.
		 *
		 * Will merge the specified histogram with this one.
		 * The argument histogram will be unmodified.
		 *
		 * @param other      The histogram to merge into this one
		 */
		void insert(const histogram_t& other);

		/** 
		 * Returns the count of the histogram at the specified
		 * location.
		 *
		 * Note that since the input values can be given arbitrary
		 * weights, the count is expressed as a double.
		 *
		 * @param v   The location to analyze
		 *
		 * @return    The total count/weight in the bin 
		 *            at location v
		 */
		inline double count(double v) const
		{
			std::map<int, double>::const_iterator it;
		
			/* find value in histogram */
			it = this->hist.find(this->get_index(v));
			if(it == this->hist.end())
				return 0;

			/* return stored count */
			return it->second;
		};

		/*----------*/
		/* analysis */
		/*----------*/

		/**
		 * Finds bin location with maximum count/weight.
		 *
		 * Returns the center location of the bin with the
		 * maximum count/weight.
		 *
		 * @return  Returns center location of maximum bin
		 */
		double max() const;

		/**
		 * Will find peaks in the histogram, which are the
		 * locations of the largest local maxima.
		 *
		 * @param peaks         Where to store the location of peaks
		 *                      in the histogram, sorted in 
		 *                      ascending order.
		 *
		 * @param counts        Where to store the histogram count 
		 *                      for each peak location returned. 
		 *                      Since each sample can have 
		 *                      non-integer weight, the counts are 
		 *                      stored as doubles.
		 *
		 * @param min_buffer    The minimum separation between 
		 *                      returned peaks. Represents the
		 *                      'neighborhood' that is considered
		 *                      local by this analysis.
		 */
		void find_peaks(std::vector<double>& peaks, 
				std::vector<double>& counts, 
				double min_buffer) const;

		/*-------*/
		/* debug */
		/*-------*/

		/**
		 * Exports data to a .m file stream
		 *
		 * Will create a .m matlab script that will
		 * define the values contained in this histogram,
		 * and display them in a figure.
		 *
		 * @param outfile     The location to export the .m file
		 *
		 * @param vertical    If true, will plot the histogram 
		 *                    vertically, false will plot the 
		 *                    histogram horizontally. Default 
		 *                    is false.
		 */
		void export_to_matlab(std::ostream& outfile, 
				bool vertical=false) const;

	/* helper functions */
	private:

		/**
		 * Gets the discretized bin index of a continuous value
		 * in this histogram.
		 */
		inline int get_index(double v) const
		{
			return (int) (v / this->res);
		};

		/**
		 * Will give the continous position of the center of the
		 * bin at the specified index.
		 */
		inline double bin_center(int i) const
		{
			return ((i + 0.5) * this->res);
		};
};

#endif
