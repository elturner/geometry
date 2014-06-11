#ifndef WINDOW_H
#define WINDOW_H

/**
 * @file window.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file defines classes used to denote window geometry
 *
 * @section DESCRIPTION
 *
 * This file defines the windows class,
 * which is used to represent windows for
 * a specific floorplan.
 *
 * Windows are parsed from a .windows file
 */

#include <map>
#include <vector>
#include <string>
#include <mesh/floorplan/floorplan.h>

using namespace std;

/* the following classes are defined in this file */
class windowlist_t;
class window_t;

/* the windowlist_t class defines a list of windows, and can
 * parse .windows files. */
class windowlist_t
{
	/*** parameters ***/
	public:

	/* the list of windows:
	 *
	 * edge_t's define walls in the referenced floorplan.
	 * window_t's define the geometry of the window
	 * within that wall.
	 */
	map<fp::edge_t, vector<window_t> > windows;

	/*** functions ***/
	public:

	/* constructors */
	windowlist_t();
	~windowlist_t();

	/* init */

	/* clear:
	 *
	 * 	Clears all data from this struct.
	 */
	void clear();

	/* import_from_file:
	 *
	 * 	Reads windowlist data from the specified
	 * 	file, which should be formatted as a
	 * 	.windows file.
	 *
	 * arguments:
	 *
	 * 	filename -	The file to parse
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int import_from_file(const std::string& filename);

	/* add:
	 *
	 * 	Adds the given window to this list.
	 */
	void add(const window_t& w);

	/* get_windows_for:
	 *
	 * 	Gets the windows for the specified wall edge
	 * 	by inserting them into the given vector.
	 *
	 * 	If no windows exist, the argument is unchanged.
	 *
	 *	Existing elements in the vector will be kept.
	 *
	 * arguments:
	 *
	 * 	wall -	The wall to check
	 *
	 * 	wins -	Where to store the windows for the specified
	 * 		wall.
	 */
	void get_windows_for(const fp::edge_t& wall,
	                     vector<window_t>& wins) const;

	/* debugging */

	/* export_to_obj:
	 *
	 * 	Exports windows as rectangles in wavefront OBJ file.
	 *
	 * arguments:
	 * 
	 * 	filename -	Where to write the file.
	 *
	 * return value;
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int export_to_obj(const std::string& filename,
	                  const fp::floorplan_t& fp);
};

/* the following represents the window class, which defines
 * one window within a model. */
class window_t
{
	/* parameters */
	public:

		/**
		 * The floorplan edge that contains this window
		 *
		 * This edge defines the start and end vertices of
		 * the wall that contains this window.
		 */
		fp::edge_t wall;

		/**
		 * These parameters specify the window geometry.
		 *
		 * The geometry of the window.  h is a position value
		 * that goes horizontally from the start vertex to the
		 * end vertex, so that h=0 indicates the start vertex
		 * and h=1 indices the end vertex.  v is a position
		 * value that goes from the floor to the ceiling, so
		 * that v=0 indicates the floor, and v=1 indicates the 
		 * ceiling.
		 */
		double min_h;
		double max_h;
		double min_v;
		double max_v;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/
		
		/**
		 * Constructs empty window object
		 */
		window_t();

		/**
		 * Constructs 'invalid' window for the specified edge
		 */
		window_t(const fp::edge_t& e);

		/**
		 * Constructs window with the specified geometry for edge e
		 *
		 * @param e   The wall that contains this window
		 * @param mh  Minimum horizontal position, in range [0,1]
		 * @param mv  Minimum vertical position, in range [0,1]
		 * @param Mh  Maximum horizontal position, in range [0,1]
		 * @param Mv  Maximum vertical position, in range [0,1]
		 */
		window_t(const fp::edge_t& e, double mh, double mv,
		         double Mh, double Mv);

		/**
		 * Parses the window geometry from specified string
		 */
		window_t(const std::string& line);

		/**
		 * Frees all memory and resources
		 */
		~window_t();

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Parses ascii values as window geometry
		 *
		 * Will parse the given line as a window definition,
		 * and will set this window's value to that result.
		 *
		 * @param line        The line to parse.
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int parse(const std::string& line);

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Sets the value of this window to the specified parameters
		 *
		 * @param e  The edge to reference
		 * @param mh  Minimum horizontal position, in range [0,1]
		 * @param mv  Minimum vertical position, in range [0,1]
		 * @param Mh  Maximum horizontal position, in range [0,1]
		 * @param Mv  Maximum vertical position, in range [0,1]
		 */
		inline void set(const fp::edge_t& e, 
				double mh, double mv, 
				double Mh, double Mv)
		{
			this->wall = e;
			this->min_h = mh;
			this->min_v = mv;
			this->max_h = Mh;
			this->max_v = Mv;
		};

		/**
		 * Returns true iff this is a valid window definition.
		 */
		inline bool valid() const
		{
			return (this->min_h < this->max_h 
					&& this->min_v < this->max_v
					&& this->wall.verts[0] >= 0
					&& this->wall.verts[1] >= 0);
		};

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Copies window values into this object
		 */
		inline window_t operator = (const window_t& rhs)
		{
			this->set(rhs.wall, rhs.min_h, rhs.min_v,
					rhs.max_h, rhs.max_v);
			return (*this);
		};

		/**
		 * Allows windows to be sorted by horizontal position
		 *
		 * This operator will compare windows by the horizontal
		 * position of their first edge.
		 *
		 * @param rhs   The other window to compare
		 *
		 * @return      Returns true iff (this < rhs)
		 */
		inline bool operator < (const window_t& rhs) const
		{
			return (this->min_h < rhs.min_h);
		};

		/*----------*/
		/* geometry */
		/*----------*/

		/**
		 * Gets window geometry in world coordinates
		 *
		 * Computes the corner positions of this window
		 * based on the information in the provided floorplan.
		 *
		 * corners are ordered as follows (viewed from interior):
		 *
		 * 		2--------1
		 * 		|        |
		 * 		|        |
		 * 		|        |
		 * 		3--------0
		 * 
		 * Each array should be length NUM_VERTS_PER_RECT.
		 *
		 * @param wx    Where to store the x-coordinates of corners
		 * @param wy    Where to store the y-coordinates of corners
		 * @param wz    Where to store the z-coordinates of corners
		 * @param fp    The floorplan to use for analysis.
		 */
		void get_world_coords(double* wx, double* wy, double* wz, 
					const fp::floorplan_t& fp);
};

#endif
