#ifndef CHUNK_EXPORTER_H
#define CHUNK_EXPORTER_H

/**
 * @file chunk_exporter.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief Classes to populate and export volume chunks
 *
 * @section DESCRIPTION
 *
 * This file defines the chunk_exporter_t class, which is a shape
 * that can be inserted into an octree.  By inserting this shape,
 * the tree's data elements will be created but not populated, and
 * the information about which shapes intersect which nodes will
 * be exported as chunk files.
 *
 * This class requires the Eigen framework.
 */

#include <io/carve/chunk_io.h>
#include <geometry/octree/shape.h>
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octdata.h>
#include <geometry/shapes/carve_wedge.h>
#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <Eigen/Dense>

/**
 * The chunk_exporter_t class is used to chunk out octree information
 *
 * An object of this class can be given a shape and a set of point indices,
 * and by inserting this object into an octree, any leafs that are
 * intersected by the given shape will be exported as chunks with the
 * specified point indices as chunk data.
 */
class chunk_exporter_t : public shape_t
{
	/* parameters */
	private:

		/**
		 * The list of exported chunks
		 *
		 * All observed leafs are exported as chunks.  Each leaf
		 * is uniquely identified by the address of its data. This
		 * list of file descriptors is kept so that one
		 * chunk file is generated for each leaf that contains
		 * all intersection information, over many inserted
		 * shapes.  These files will be maintained until
		 * this object is closed.
		 */
		std::map<octdata_t*, chunk::chunk_writer_t*> chunk_map;

		/**
		 * This value represents the current shape to intersect
		 *
		 * This shape will be used to intersect into given octrees.
		 * Inserting this chunk_exporter_t into an octree will
		 * intersect the same nodes as if this shape was inserted.
		 *
		 * This object should be allocated elsewhere.  This class
		 * is not responsible for memory management of reference
		 * shapes.
		 */
		shape_t* reference_shape;

		/**
		 * This list indicates what values to export to chunks
		 *
		 * When this chunk_exporter_t is inserted into an octree,
		 * then every leaf node that is intersected will be written
		 * to a chunk file, and these values will be added to each
		 * of those chunks.
		 */
		std::vector<chunk::point_index_t> vals;

		/**
		 * Where to store the final .chunklist file
		 */
		std::string chunklist_filename;

		/**
		 * The directory to store the .chunk files
		 */
		std::string full_chunk_dir; /* relative to working dir */
		std::string rel_chunk_dir; /* relative to chunklist file */

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Initializes empty object
		 */
		chunk_exporter_t();

		/**
		 * Frees all memory and resources
		 */
		~chunk_exporter_t();

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Sets the reference shape and point values
		 *
		 * This function will set the reference shape to use.
		 * After this call, and calls to the overloaded functions
		 * from the shape_t class will return the same values as
		 * if they were called to rs.
		 *
		 * The list v indicates the list of values to insert into
		 * chunk files that are intersected by this shape.
		 *
		 * @param rs   The reference shape to use
		 * @param v    The chunk values to export for this shape
		 */
		void set(shape_t* rs,
		         const std::vector<chunk::point_index_t>& v);

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Opens a .chunklist file and prepares chunk generation
		 *
		 * This function must be called before performing any
		 * inserts with this object.  This function specifies the
		 * location of the .chunklist and corresponding .chunk
		 * files.
		 *
		 * When specifying the relative directory for the .chunk
		 * files, note that this is relative to the location
		 * of the .chunklist file, so if:
		 *
		 * clfile = "foo/bar/list.chunklist"
		 * chunk_dir = "baz"
		 *
		 * then the chunk files will be stored in:
		 *
		 * foo/bar/baz/<chunkname>.chunk
		 *
		 * @param clfile     Where to write the chunklist file
		 * @param chunk_dir  The relative directory to store chunks
		 */
		void open(const std::string& clfile,
		         const std::string& chunk_dir);

		/**
		 * Will generate a .chunklist file and close all chunks.
		 *
		 * Calling this function will close all file streams to
		 * chunks that are currently active, and will generate
		 * a corresponding .chunklist file for these chunks.
		 *
		 * After this call, you must make another call to open()
		 * to use this object again.
		 *
		 * Note that the tree provided as an argument must be
		 * the same tree that was used for all inserts.
		 *
		 * @param tree          The tree that has been chunked
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int close(const octree_t& tree);

		/**
		 * Overloaded close() function.
		 *
		 * @param cx             The x-coordinate of tree center
		 * @param cy             The y-coordinate of tree center
		 * @param cz             The z-coordinate of tree center
		 * @param hw             The halfwidth of tree
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int close(double cx, double cy, double cz, double hw);

		/*----------------------------*/
		/* overloaded shape functions */
		/*----------------------------*/

		/**
		 * Specifies the number of vertices for this shape.
		 *
		 * @return   Returns the number of shape vertices
		 */
		inline unsigned int num_verts() const
		{
			/* just use the reference shape */
			if(this->reference_shape == NULL)
				return 0;
			return this->reference_shape->num_verts();
		};

		/**
		 * Retrieves the i'th vertex
		 *
		 * Will return the position of the specified vertex.  Note
		 * that only indices within [0, num_verts() ) are valid.
		 *
		 * @param i   The index of the vertex to retrieve
		 * 
		 * @return    Returns the position of the i'th vertex
		 */
		inline Eigen::Vector3d get_vertex(unsigned int i) const
		{
			Eigen::Vector3d x(0,0,0);

			/* use the reference shape */
			if(this->reference_shape == NULL)
				return x;
			return this->reference_shape->get_vertex(i);
		};

		/**
		 * Check for intersection against an AABB
		 *
		 * Checks if the reference shape of this exporter intersects
		 * the given parameters for an Axis-Aligned Bounding Box.
		 *
		 * @param c   The center of the box
		 * @param hw  The half-width of the box
		 *
		 * @return    Returns true iff an intersection occurs.
		 */
		inline bool intersects(const Eigen::Vector3d& c,
		                       double hw) const
		{
			/* use reference shape */
			if(this->reference_shape == NULL)
				return false; /* no shape -> no intersect */
			return this->reference_shape->intersects(c,hw);
		};

		/**
		 * Will update chunk file for this leaf
		 *
		 * By calling this, will generate/append the current
		 * chunk values to the chunk file associated with this
		 * leaf.  The file is hashed by the address of the leaf's
		 * data. If the leaf has no data, then a data object will
		 * be allocated to the leaf and returned.
		 *
		 * @param c   The center position of the leaf
		 * @param hw  The half-width of the leaf geometry
		 * @param d   The current data object for the leaf
		 *
		 * @return    Returns the leaf's data object, which may be
		 *            newly allocated by this function.  If the
		 *            provided data is not null, then d will be
		 *            returned.
		 */
		octdata_t* apply_to_leaf(const Eigen::Vector3d& c,
		                         double hw, octdata_t* d);

	/* helper functions */
	private:

		/**
		 * Converts a pointer to a uuid string
		 *
		 * @param p   The pointer to convert
		 *
		 * @return    A uuid string representation of pointer
		 */
		static inline std::string ptr_to_uuid(void* p)
		{
			std::stringstream ss;

			/* convert pointer to string */
			ss.str("");
			ss << p;
			return ss.str().substr(2); /* remove '0x' */
		};
};


#endif
