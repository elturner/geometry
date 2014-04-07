#ifndef RANGE_LIST_H
#define RANGE_LIST_H

/**
 * @file range_list.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the range list class, which represents
 * a subset of the real line with a sequence of disjoint closed 
 * intervals.
 */

#include <string>
#include <set>

/* the following classes are declared in this file */
class range_list_t;
class range_t;

/* the following declares the range_list_t class */
class range_list_t
{
	/*** parameters ***/
	private:

		/* the range list is represented by a sequence
		 * of disjoint closed sets.  These sets are
		 * ordered by the natural ordering of the reals */
		std::set<range_t> list;

	/*** functions ***/
	public:

		/**
		 * Constructs empty range list
		 */
		range_list_t();

		/**
		 * Frees all memory and resources
		 */
		~range_list_t();

		/**
		 * Clears all information from this list
		 */
		void clear();

		/**
		 * Parses the string as a range list
		 *
		 * The input string should be formatted as a
		 * semicolon-separated list of intervals, such as:
		 *
		 * e.g.     "[0,2.3];[5,7.4];10;11.5;[20.2,30.1]"
		 *
		 * @param s  The string to parse
		 *
		 * @return   Returns zero on success, non-zero on failure
		 */
		int parse(std::string& s);

		/**
		 * Adds range to list
		 *
		 * Will add the specified range to this list, rearranging if
		 * necessary to ensure the elements in the list are 
		 * disjoint.
		 *
		 * Invalid ranges will be ignored
		 *
		 * @param r  The range to add to list
		 */
		void add(const range_t& r);

		/**
		 * Adds range to list
		 *
		 * Will add the specified range to this list, rearranging if
		 * necessary to ensure the elements in the list are 
		 * disjoint.
		 *
		 * Invalid ranges will be ignored
		 *
		 * @param a   The minimum value in this range
		 * @param b   The maximum value in this range
		 */
		void add(double a, double b);

		/**
		 * Checks if value is contained in range list
		 *
		 * Returns true iff the specified value is covered
		 * by one of the intervals in the range list.
		 *
		 * @return  Returns true iff specified value in range list
		 */
		bool contains(double v) const;
};

/* the following decalres the range_t class, which
 * represents a single closed interval on the real line. */
class range_t
{
	/* allow the range_list_t class to modify these */
	friend class range_list_t;

	/*** parameters ***/
	private:

		double min; /* start of interval (inclusive) */
		double max; /* end   of interval (inclusive) */

	/*** functions ***/
	public:

		/**
		 * Initializes unit interval [0,1]
		 */
		range_t();

		/**
		 * Initializes specified interval [a,b]
		 *
		 * If a > b, these values will be flipped to
		 * generated interval [b,a]
		 *
		 * @param a   Min value in range
		 * @param b   Max value in range
		 */
		range_t(double a, double b);

		/**
		 * Intializes this range from the specified string
		 *
		 * Valid formatting for the string, for numbers x and y, 
		 * is one of:
		 * 		"x" or "[x,y]"
		 *
		 * Empty or invalid strings generate invalid ranges [1,0], 
		 * which will have negative length.
		 *
		 * @param s   The string to parse
		 */
		range_t(std::string& s);

		/**
		 * Frees all memory and resources
		 */
		~range_t();

		/**
		 * Returns the Lebesgue measure (a.k.a length) of the range
		 */
		double length() const;

		/**
		 * Checks if value in range
		 *
		 * Will return true iff the specified value is
		 * in the interval specified by this range.
		 *
		 * @return Returns whether given value in range
		 */
		bool contains(double v) const;

		/* overloaded operators */

		/**
		 * Sets this range to the specified range
		 */
		range_t operator = (const range_t& other);

		/**
		 * Returns true iff overlaps with other range
		 */
		bool operator == (const range_t& other) const;

		/**
		 * Returns true iff does not overlap with other range
		 */
		bool operator != (const range_t& other) const;

		/**
		 * Returns true iff all values in this less than other
		 *
		 * Returns true only iff all values in this range are 
		 * strictly less than all values in the other range.
		 *
		 * @return Returns true iff disjoint and less than other
		 */
		bool operator < (const range_t& other) const;

		/**
		 * Returns true iff all values in this greater than other
		 */
		bool operator > (const range_t& other) const;

		/**
		 * Returns true iff either '<' or '==' operator satisfied
		 */
		bool operator <= (const range_t& other) const;

		/**
		 * Returns true iff either '>' or '==' operator satisfied
		 */
		bool operator >= (const range_t& other) const;

		/**
		 * Generates the convex hull of this and the other range
		 */
		range_t operator + (const range_t& other) const;

		/**
		 * Sets this to be convex hull of this and the other range
		 */
		range_t operator += (const range_t& other);
};

#endif
