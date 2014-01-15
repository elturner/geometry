#ifndef CELL_GRAPH_H
#define CELL_GRAPH_H

/* cell_graph.h:
 *
 * This file contains definitions for classes used to
 * define a graph (V,E), where V consists of a set of gridcells
 * which are stored in a quadtree.  E contains edges between cells
 * that will eventually be exported as walls in a floor plan.
 */

#include <map>
#include <set>
#include <vector>
#include <ostream>
#include "quadtree.h"
#include "path.h"
#include "../rooms/tri_rep.h"
#include "../util/constants.h"

using namespace std;

/* the following classes are defined in this file */
class cell_graph_t;
class cell_t;
class edge_error_t;

/* the following is the definition of the graph class */
class cell_graph_t
{
	/*** parameters ***/
	public:

	/* the following structure stores the cells in this
	 * graph, which can be identified by their data pointers. */
	set<cell_t> V;
	int num_rooms; /* number of rooms defined in this graph */

	/*** functions ***/
	public:

	/* constructors */
	
	cell_graph_t();
	~cell_graph_t();

	/* graph operations */

	/* remove:
	 *
	 * 	Removes the specified cell from the graph.
	 */
	void remove(cell_t* c);

	/* remove_outliers:
	 *
	 * 	Removes all cells in this graph that are outliers.
	 */
	void remove_outliers();

	/* height operations */

	/* reset_heights:
	 *
	 * 	Will reset the ranges of heights for all cells to
	 * 	be the default range of [-ASSUMED_WALL_HEIGHT/2, 
	 * 	ASSUMED_WALL_HEIGHT/2].
	 */
	void reset_heights();

	/* flatten_room_heights:
	 *
	 * 	Will force all rooms to have single height ranges.
	 * 	These ranges are chosen by the median values of min
	 * 	and max z values for each given room.
	 */
	void flatten_room_heights();

	/* processing */

	/* populate:
	 *
	 * 	Given a quadtree, will populate this
	 * 	graph with nodes consisting of the
	 * 	data from the tree.
	 *
	 *	Will destroy any existing information.
	 *
	 * arguments:
	 *
	 * 	tree -	The tree from which to draw data points
	 * 		to create cells.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int populate(quadtree_t& tree);

	/* simplify_straights:
	 *
	 * 	Will reduce the number of cells in the graph
	 * 	by removing cells that fit both of the following
	 * 	details:
	 *
	 * 		- have exactly two edges
	 * 		- edges point in opposite directions
	 *
	 * arguments:
	 *
	 * 	trirep -	The triangulation that created this
	 * 			graph.  Will simplify this triangulation
	 * 			in tandum with any simplifications on
	 * 			the graph.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int simplify_straights(tri_rep_t& trirep);

	/* simplify:
	 *
	 *	Will apply QEM simplification to graph, collapsing
	 *	cells up to the threshold error.
	 *
	 *	Will also modify the originating triangulation, so it
	 *	is simiplified in the same manner.
	 *
	 * arguments:
	 *
	 *	trirep -	The spawning triangulation topology
	 *	threshold -	The error threshold to use during
	 *			simplification.
	 *
	 * return value:
	 *
	 *	Returns zero on success, non-zero on failure.
	 */
	int simplify(tri_rep_t& trirep, double threshold);

	/* union operations */

	/* union_find:
	 *
	 * 	Partitions the graph into unions
	 * 	of disjoint connected components.
	 *
	 * arguments:
	 *
	 * 	unions -	A list of unions.
	 */
	void union_find(vector<set<cell_t*> >& unions);

	/* remove_unions_below:
	 *
	 * 	Removes any connected component composed of
	 * 	edges whose lengths add up to less than the
	 * 	specified threshold.
	 *
	 * arguments:
	 *
	 * 	len -	The minimum length that a union must
	 * 		be to survive.
	 */
	void remove_unions_below(double len);

	/* partition_regions:
	 *
	 * 	Will partition the edges of the graph into regions
	 * 	that are nearly parallel.  Each region is a connected
	 * 	link of nearly parallel edges, in order.
	 *
	 * 	This operation fails if any cell is extraordinary
	 * 	in this graph.
	 *
	 * arguments:
	 *
	 * 	regions -	Where to store the partitioned graph,
	 * 			each element represents a region, with
	 * 			cells listed in order from start to end,
	 * 			defining the edges between each adjacent
	 * 			cell.
	 *
	 *	orienter -	The triangulation used to orient these
	 *			regions.  The regions will be listed
	 *			so they go around the interior in
	 *			the counter-clockwise direction.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int partition_regions(vector<vector<cell_t*> >& regions,
				tri_rep_t& orienter);

	/* analysis */

	/* compute_height_bounds:
	 *
	 * 	Computes the min and max height in the graph.
	 *
	 * arguments:
	 *
	 * 	min_z, max_z -	Where to store the min and max heights,
	 * 			respectively.
	 */
	void compute_height_bounds(double& min_z, double& max_z);

	/* find_clique:
	 *
	 *	Finds a clique of size 3 that contains this cell.
	 *	If no such clique exists, then the set 'clique' will
	 *	be empty.
	 *
	 *	If the cell is a part of multiple size-3 cliques or
	 *	part of cliques of size greater than 3, will return
	 *	any valid clique.
	 *
	 * arguments:
	 *
	 * 	c -		The cell to check
	 * 	clique -	Will store the nodes of the clique
	 * 			in this set.
	 */
	void find_clique(cell_t* c, set<cell_t*>& clique);
	
	/* i/o and debugging */

	/* index_cells:
	 *
	 * 	Assigns cells a unique id.
	 */
	void index_cells();

	/* print_cells:
	 *
	 *	Prints the nodes of the graph to the specified stream.
	 *
	 * format:
	 *
	 * 	<x1> <y1>
	 * 	<x2> <y2>
	 * 	...
	 */
	void print_cells(ostream& os);

	/* print_edges:
	 *
	 *	Prints the edges currently in the graph as 2D lines.
	 *
	 *	Note: since this graph is bidirectional, edges may be
	 *	printed twice.
	 *
	 * format:
	 *
	 * 	Each line represents an edge:
	 *
	 * 	<x1> <y1> <x2> <y2>
	 */
	void print_edges(ostream& os);

	/* print_edges_3D:
	 *
	 * 	Prints the edges currently in the graph as 3D planes,
	 * 	extruded in the z-dimension by the specified height.
	 *
	 * 	Output format is Wavefront OBJ.
	 *
	 * arguments:
	 *
	 * 	os -	The stream to which the faces will be written
	 */
	void print_edges_3D(ostream& os);
};

/* the following defines the cell_t class */
class cell_t
{
	/*** security ***/
	friend class cell_graph_t;
	friend class edge_error_t;

	/*** parameters ***/
	public:

	/* geometry */

	/* the position of cell in R^2 */
	point_t pos;

	/* the ranges of heights allowed for this cell (optional) */
	double min_z;
	double max_z;

	/* the error associated with this cell */
	double err_mat[ERROR_MATRIX_SIZE];

	/* edges connect cells in the graph, and
	 * can be arbitrary length.
	 *
	 * In this structure, edges are bi-directional */
	set<cell_t*> edges;

	/* metadata */

	quaddata_t* data;
	int union_id; /* -1 normally, non-neg if unions 
					defined. */
	int vertex_index; /* index of cell's vertex in the triangulation */
	int index; /* a unique id for this cell */
	set<int> room_ids; /* unique ids for the rooms this cell is in. */

	/*** functions ***/
	public:

	/* constructors */
	
	cell_t();
	cell_t(quaddata_t* dat);
	~cell_t();

	/* initializers */

	inline void init(quaddata_t* dat)
	{
		this->data = dat;
		if(dat)
			this->pos = dat->average;
		this->min_z = dat->norm.get(0);
		this->max_z = dat->norm.get(1);
		this->union_id = -1;
		this->index = -1;
		this->vertex_index = -1;
		this->room_ids.clear();
		this->edges.clear();
	};

	/* init_err_mat:
	 *
	 * 	Will initialize the error matrix using the
	 * 	current edges of this cell.
	 */
	void init_err_mat();

	/* operators */

	inline bool operator < (const cell_t& rhs) const
	{
		/* used for storage in sets, so must be unique to cell */
		return (this->data < rhs.data);
	};

	/* accessors */

	/* get_data:
	 *
	 * 	Returns pointer to cell data.
	 */
	inline quaddata_t* get_data() const
	{
		return (this->data);
	};

	/* is_interior:
	 *
	 * 	Returns true iff this cell is
	 * 	completely surrounded by neighbors.
	 */
	inline bool is_interior()
	{ 
		return (this->edges.size() 
			== (NUM_EDGES_PER_SQUARE + NUM_VERTS_PER_SQUARE)); 
	};

	/* is_outlier:
	 *
	 * 	Returns true iff this cell has no neighbors.
	 */
	inline bool is_outlier()
	{
		return (this->edges.empty());
	};

	/* is_ordinary:
	 *
	 * 	An ordinary cell is one that is part of a wall,
	 * 	so it has exactly two edges incident to it.
	 */
	inline bool is_ordinary()
	{
		return (this->edges.size() == 2);
	};

	/* is_extraordinary:
	 *
	 * 	An extraordinary cell has more edges than an ordinary
	 * 	cell.
	 */
	inline bool is_extraordinary()
	{
		return (this->edges.size() > 2);
	};

	/* is_room_boundary:
	 *
	 * 	Checks if this cell belongs to multiple rooms.
	 */
	inline bool is_room_boundary()
	{
		return (this->room_ids.size() > 1);
	};

	/* is_corner:
	 *
	 * 	If the angle created by the two edges touching
	 * 	an ordinary cell are below the parallel threshold,
	 * 	then it is considered a corner.
	 *
	 * 	Any extraordinary cell is not a corner.
	 */
	bool is_corner();

	/* corner_angle:
	 *
	 * 	Returns the cosine of the angle between edges
	 * 	for ordinary cells.
	 */
	bool corner_angle();

	/* traverse:
	 *
	 * 	Will return the oriented neighbor of this cell,
	 * 	such that the edge (this, ret) goes counter-
	 * 	clockwise about the interior space of the
	 * 	environment.
	 *
	 * arguments:
	 *
	 * 	edge_dir -	The previous direction taken by
	 * 			the traversal.  Only used if
	 * 			there are multiple valid choices
	 * 			among the neighbors.
	 *
	 *			Should point towards 'this' cell
	 *
	 * 			This value will be overwritten
	 * 			with the new edge's direction.
	 *
	 * 	orienter -	The triangulation used to define
	 * 			the interior space of this graph.
	 *
	 * return value:
	 * 
	 * 	Returns the neighbor of this cell that is
	 * 	maximally in a counter-clockwise orientation.
	 */
	cell_t* traverse(normal_t& edge_dir, tri_rep_t& orienter);

	/* traverse_back:
	 *
	 * 	Same functionality as traverse(), but will iterate
	 * 	in reverse order.
	 */
	cell_t* traverse_back(normal_t& edge_dir, tri_rep_t& orienter);

	/* get_simplification_error:
	 *
	 * 	Will determine the error of this cell
	 * 	with respect the argument simplification error
	 * 	matrix.
	 */
	double get_simplification_error(double* mat);

	/* graph operations */
	
	/* add_edge:
	 *
	 *	Adds an edge between the two cells
	 */
	void add_edge(cell_t* other);

	/* dist_sq:
	 *
	 * 	Computes the squared distance between this cell and the
	 * 	argument cell.
	 */
	inline double dist_sq(cell_t* other)
	{
		return (this->pos.dist_sq(other->pos));
	};

	/* remove_edge:
	 *
	 * 	Removes an edge, if it exists, between the
	 * 	two cells.
	 */
	void remove_edge(cell_t* other);

	/* transfer_all_edge:
	 *
	 * 	Transfers all edges from the argument to this cell.
	 * 	After this call, the argument will have no connections
	 *	and this cell will have (possibly) more edges than before.
	 *
	 * arguments:
	 *
	 * 	other -	The cell to remove edges from
	 */
	void transfer_all_edges(cell_t* other);

	/* replace_with_clique:
	 *
	 *	Severs all edges that are connected to this cell.  Any
	 *	neighboring cells will be connected to one another.  Thus
	 *	the set of original edge neighbors of this node are now
	 *	a clique.
	 */
	void replace_with_clique();

	/* debug */

	/* color_by_room:
	 *
	 * 	Will assign a color based on the room
	 * 	id's of this cell.
	 *
	 * arguments:
	 *
	 * 	r,g,b -	Where to store the red, green, blue values
	 * 		of the determined color.
	 */
	void color_by_room(int& r, int& g, int& b) const;
};

/* this class contains information pertaining to simplifying an edge
 * between two cells in the graph. */
class edge_error_t
{
	/*** parameters ***/
	public:

	/* the cells that compose this edge */
	cell_t* a; /* the collapse will have the output cell be 'a' */
	cell_t* b;

	/* and with the sum of their error matrices */
	double err_mat_sum[ERROR_MATRIX_SIZE];
	double err; /* the error of comb_pos to this matrix */

	/*** functions ***/
	public:

	/* constructors */
	edge_error_t();
	edge_error_t(cell_t* aa, cell_t* bb);
	~edge_error_t();

	/* comparison operators */
	inline bool operator < (const edge_error_t& rhs) const
	{
		/* sort by error:
		 *
		 * this is reversed ordering, so that priority
		 * queues put smallest error first */
		return (this->err > rhs.err);
	};
};

#endif
