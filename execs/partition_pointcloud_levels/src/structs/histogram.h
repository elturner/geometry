#ifndef HISTOGRAM_H
#define HISTOGRAM_H

/* histogram.h:
 *
 * This file defines a class that
 * will construct a histogram of
 * of scalar values at a specified
 * resolution.
 */

#include <map>
#include <vector>
#include <fstream>

/* the following class represents the histogram
 * which has dynamic ranging */
class histogram_t
{
	/*** parameters ***/
	private:

	/* The histogram is stored as a map,
	 * where the keys represent discretized
	 * values given to the histogram, and the
	 * elements of the map are counts for
	 * the keys */
	std::map<int, int> hist;

	/* to discretize the keys, a resolution must
	 * be specified */
	double res;

	/*** functions ***/
	public:

	/* constructors */
	histogram_t();
	~histogram_t();

	/* clear:
	 *
	 * 	Clears any info from the histogram.
	 */
	void clear();

	/* set_resolution:
	 *
	 * 	Resets the resolution of this histogram.  Will
	 * 	clear any existing values in the histogram.
	 *
	 * arguments:
	 *
	 * 	r -	The resolution to use.  The sign of r
	 * 		will be ignored.
	 */
	void set_resolution(double r);

	/* accessors */

	/* insert:
	 *
	 * 	Will add a value to the histogram.
	 *
	 * arguments:
	 *
	 * 	v -	The value to add
	 */
	void insert(double v);

	/* insert:
	 *
	 * 	Will merge the specified histogram with this one.
	 * 	The argument histogram will be unmodified.
	 *
	 * arguments:
	 *
	 * 	other -	The histogram to merge into this one
	 */
	void insert(histogram_t& other);

	/* count:
	 *
	 * 	Returns the count of the histogram at the specified
	 * 	location.
	 */
	inline int count(double v)
	{
		std::map<int, int>::iterator it;
		
		/* find value in histogram */
		it = this->hist.find(this->get_index(v));
		if(it == this->hist.end())
			return 0;

		/* return stored count */
		return it->second;
	};

	/* analysis */

	/* max:
	 *
	 * 	Returns the center location of the bin with the
	 * 	maximum value.
	 */
	double max();

	/* find_peaks:
	 *
	 * 	Will find peaks in the histogram, which are the
	 * 	locations of the largest local maxima.
	 *
	 * arguments:
	 *
	 * 	peaks -		Where to store the location of peaks
	 * 			in the histogram, sorted in ascending
	 * 			order.
	 *
	 *	counts -	Where to store the histogram count 
	 *			for each peak location returned.
	 *
	 * 	min_buffer -	The minimum separation between returned
	 * 			peaks.  If set to zero, will only use
	 * 			local maxima criterion to discern
	 * 			whether a peak is significant.
	 */
	void find_peaks(std::vector<double>& peaks, 
			std::vector<int>& counts, double min_buffer);

	/* debug */

	/* export_to_matlab:
	 *
	 * 	Will create a .m matlab script that will
	 * 	define the values contained in this histogram,
	 * 	and display them in a figure.
	 *
	 * arguments:
	 *
	 * 	outfile -	The location to export the .m file
	 *	vertical -	If true, will plot the histogram vertically,
	 *			false will plot the histogram horizontally.
	 *			Default is false.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int export_to_matlab(std::ofstream& outfile, bool vertical);

	/*** helper functions ***/
	private:

	/* get_index:
	 *
	 * 	Gets the discretized bin index of a continuous value
	 * 	in this histogram.
	 */
	inline int get_index(double v)
	{
		return (int) (v / this->res);
	};

	/* bin_center:
	 *
	 * 	Will give the continous position of the center of the
	 * 	bin at the specified index.
	 */
	inline double bin_center(int i)
	{
		return ((i + 0.5) * this->res);
	};
};

#endif
