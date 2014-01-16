#ifndef WINDOW_H
#define WINDOW_H

/* window.h:
 *
 * 	This file defines the windows class,
 * 	which is used to represent windows for
 * 	a specific floorplan.
 *
 * 	Windows are parsed from a .windows file
 */

#include <map>
#include <vector>
#include "floorplan.h"

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
	map<edge_t, vector<window_t> > windows;

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
	int import_from_file(const char* filename);

	/* add:
	 *
	 * 	Adds the given window to this list.
	 */
	void add(window_t& w);

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
	void get_windows_for(edge_t& wall, vector<window_t>& wins);

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
	int export_to_obj(const char* filename, floorplan_t& fp);
};

/* the following represents the window class, which defines
 * one window within a model. */
class window_t
{
	/*** parameters ***/
	public:

	/* this edge defines the start and end vertices of
	 * the wall that contains this window. */
	edge_t wall;

	/* the geometry of the window.  h is a position value
	 * that goes horizontally from the start vertex to the
	 * end vertex, so that h=0 indicates the start vertex
	 * and h=1 indices the end vertex.  v is a position
	 * value that goes from the floor to the ceiling, so
	 * that v=0 indicates the floor, and v=1 indicates the ceiling. */
	double min_h;
	double max_h;
	double min_v;
	double max_v;

	/*** functions ***/
	public:

	/* constructors */
	window_t();
	window_t(edge_t& e);
	window_t(edge_t& e, double mh, double mv, double Mh, double Mv);
	window_t(char* line);
	~window_t();

	/* i/o */

	/* parse:
	 *
	 * 	Will parse the given line as a window definition,
	 * 	and will set this window's value to that result.
	 *
	 * argument:
	 *
	 * 	line -	The line to parse.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int parse(const char* line);

	/* accessors */

	/* set:
	 *
	 * 	Sets the value of this window to the specified
	 * 	parameters.
	 */
	inline void set(const edge_t& e, const double mh, const double mv, 
					const double Mh, const double Mv)
	{
		this->wall = e;
		this->min_h = mh;
		this->min_v = mv;
		this->max_h = Mh;
		this->max_v = Mv;
	};

	/* valid:
	 *
	 * 	Returns true iff this is a valid window definition.
	 */
	inline bool valid() const
	{
		return (this->min_h < this->max_h 
				&& this->min_v < this->max_v
				&& this->wall.verts[0] >= 0
				&& this->wall.verts[1] >= 0);
	};

	/* operators */

	inline window_t operator = (const window_t& rhs)
	{
		this->set(rhs.wall, rhs.min_h, rhs.min_v,
				rhs.max_h, rhs.max_v);
		return (*this);
	};

	/* geometry */

	/* get_world_coords:
	 *
	 *	Computes the corner positions of this window
	 *	based on the information in the provided floorplan.
	 *
	 *	corners are ordered as follows (viewed from interior):
	 *
	 * 		2--------1
	 * 		|        |
	 * 		|        |
	 * 		|        |
	 * 		3--------0
	 *
	 * arguments:
	 *
	 * 	wx,
	 * 	wy,
	 * 	wz -	Where to store the x-, y-, and z-coordinates of
	 * 		the window corners.  Each array should be length
	 * 		NUM_VERTS_PER_RECT.
	 *
	 * 	fp -	The floorplan to use for analysis.
	 */
	void get_world_coords(double* wx, double* wy, double* wz, 
				floorplan_t& fp);
};

#endif
