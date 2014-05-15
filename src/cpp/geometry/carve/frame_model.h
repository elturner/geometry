#ifndef FRAME_MODEL_H
#define FRAME_MODEL_H

/**
 * @file frame_model.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the frame_model_t class, which houses probabilistic
 * information for each scan-point in a frame.
 */

#include <io/data/fss/fss_io.h>
#include <io/carve/wedge_io.h>
#include <geometry/system_path.h>
#include <geometry/octree/octree.h>
#include <geometry/carve/gaussian/scan_model.h>
#include <geometry/carve/gaussian/carve_map.h>
#include <geometry/shapes/chunk_exporter.h>
#include <vector>
#include <string>

/**
 * This class holds a list of carve maps for each scan-point in a frame
 */
class frame_model_t
{
	/* parameters */
	private:

		/* a list of carve maps for each point in the frame */
		unsigned int num_points;
		carve_map_t* map_list; /* dynamically allocated list */
		
		/* this bool list indicates which carve maps are valid */
		std::vector<bool> is_valid;

	/* functions */
	public:

		/*-------------------------------*/
		/* constructors and initializers */
		/*-------------------------------*/

		/**
		 * Constructs empty frame
		 */
		frame_model_t();

		/**
		 * Frees all memory and resources
		 */
		~frame_model_t();

		/**
		 * Initializes this object from the given frame info
		 *
		 * Given an input frame, will compute the probability
		 * distributions and carving maps for each point in the
		 * frame, and store those results in this object.
		 *
		 * The provided scan_model_t should already be initialized
		 * to the current sensor (call set_sensor() before this
		 * call).  This call will call set_frame() and set_point()
		 * functions appropriately.
		 *
		 * @param frame   The input frame to model
		 * @param ang     Angular spacing between points in frame
		 * @param linefit The line-fit distance parameter
		 * @param model   The model object for the sensor
		 * @param path    The path of the system
		 *
		 * @return     Returns zero on success, non-zero on failure
		 */
		int init(const fss::frame_t& frame, double ang,
		         double linefit, scan_model_t& model,
		         const system_path_t& path);

		/**
		 * Swaps information with the other frame
		 *
		 * Calling this function will swap frame info.  It does
		 * not matter which frame calls the function.  Both of
		 * the following have the same behavior:
		 *
		 * a.swap(b)
		 * b.swap(a)
		 *
		 * for frame_model_t a, b.
		 *
		 * @param other   The other frame to swap with
		 */
		void swap(frame_model_t& other);

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Retrieves number of points stored in frame
		 *
		 * @return   Returns number of points stored in this frame
		 */
		inline unsigned int get_num_points() const
		{ return this->num_points; };

		/**
		 * Retrieves the i'th point's carve map
		 *
		 * @param i  The index of the carve map to retrieve
		 *
		 * @return   Returns the pointer for the i'th point
		 */
		inline carve_map_t* get_scan(unsigned int i) const
		{ return (this->map_list + i); };

		/*----------*/
		/* geometry */
		/*----------*/

		/**
		 * Will populate chunks using this frame with the given tree
		 *
		 * Given an octree and a chunk exporter, will insert
		 * this frame into the tree using the chunk exporter,
		 * so that this frame will be added to all chunks in
		 * the tree that intersect it.  These chunks will be
		 * written to disk by the exporter.
		 *
		 * @param tree     The tree to use to generate chunks
		 * @param next     The next frame in the sequence
		 * @param buf      In units of std. devs., carving buffer
		 * @param chunker  The chunk exporter to export chunks
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int export_chunks(octree_t& tree, const frame_model_t& next,
		                  double buf,
		                  chunk_exporter_t& chunker) const;

		/**
		 * Will carve/insert this frame info into the given octree
		 *
		 * Given an octree, and a next frame, will generate
		 * carve shapes to insert into the octree, which will map
		 * the carve_maps stored in this tree into the octree
		 * structure.
		 *
		 * The next frame is required in order to define the volume
		 * between frames, which is represented by individual
		 * carve_wedge_t's, that are inserted into the tree.
		 *
		 * @param tree   The tree to modify
		 * @param next   The next frame in the sequence
		 * @param buf    In units of std. devs., carving buffer
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int carve(octree_t& tree, const frame_model_t& next,
		          double buf) const;

		/**
		 * Will carve/insert this frame info into the given octnode
		 *
		 * Given an octnode and a next frame, will generate
		 * carve shapes that represent the entirety of the volume
		 * between this frame and the next frame, and will insert
		 * those shapes into the octnode structure.
		 *
		 * @param node   The node to modify
		 * @param depth  The maximum carve depth of this node
		 * @param next   The next frame after this one
		 * @param buf    The carve buffer paramter, units of stdev
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int carve(octnode_t* node, unsigned int depth,
		          const frame_model_t& next, double buf) const;

		/**
		 * Will carve an individual wedge from this frame into tree
		 *
		 * Given an octree and a next frame, will generate
		 * a single wedge from this frame and insert it
		 * into the octree.
		 *
		 * @param tree    The tree to modify
		 * @param next    The next frame in the sequence
		 * @param buf     In units of std. devs., carving buffer
		 * @param i       The index of the wedge to carve.
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int carve_single(octree_t& tree, const frame_model_t& next,
		                 double buf, unsigned int i) const;

		/**
		 * Will carve an individual wedge into a subnode of octree
		 *
		 * Will generate a single wedge from this frame and insert
		 * it into the specified subnode of a tree.  Useful if
		 * you want to modify only a part of a tree, and not
		 * insert the wedge into the full tree.
		 *
		 * @param node    The tree node to modify
		 * @param depth   The max depth to insert the wedge in node
		 * @param next    The next frame in the sequence
		 * @param buf     In units of std. devs., carving buffer
		 * @param i       The index of the wedge to carve.
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int carve_single(octnode_t* node, unsigned int depth,
		                 const frame_model_t& next, double buf,
		                 unsigned int i) const;

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Will export all wedges from this frame and next frame
		 *
		 * Given a populated next_frame structure, will compute
		 * all wedges between the two frames, and write out
		 * each one's serialization to the given file stream.
		 *
		 * @param os    The binary output stream to write to
		 * @param next  The next frame from this frame
		 * @param buf   In units of std. devs., carving buffer
		 *
		 * @return      Returns the number of wedges exported,
		 *              returns negative value on error.
		 */
		int serialize_wedges(wedge::writer_t& os,
			const frame_model_t& next, double buf) const;

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Will export meshes of gaussian models to OBJ file
		 *
		 * Will export a mesh representing the basic shape
		 * of the probability distribution of this point/sensor
		 * pair to a wavefront OBJ file stream.
		 *
		 * @param fn  The file to write to
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int writeobj(const std::string& fn) const;

	/* helper functions */
	private:

		/**
		 * Will find valid point indices for a wedge
		 *
		 * Given the index of a wedge, will compute the
		 * scanpoint indices to use to generate that wedge
		 *
		 * @param i     The wedge index
		 * @param next  The next frame to use to generate wedge
		 * @param ta    Where to store this scan's a-index
		 * @param tb    Where to store this scan's b-index
		 * @param na    Where to store next scan's a-index
		 * @param nb    Where to store next scan's b-index
		 */
		void find_wedge_indices(unsigned int i,
		                        const frame_model_t& next,
		                        unsigned int& ta,
		                        unsigned int& tb,
		                        unsigned int& na,
		                        unsigned int& nb) const;

		/**
		 * Will compute the probability that each point is planar
		 *
		 * For each scan point within this frame,
		 * will analyze the neighbors of this point to determine
		 * the likelihood that the point is part of a planar
		 * region.  This process requires a distance value that
		 * indicates how far away a neighbor can be to still have
		 * an effect on this processing.
		 *
		 * The results will be stored in this->planar_probs
		 *
		 * @param dist  Distance that defines neighborhood of point
		 * @param ang   The expected angular spacing (in radians)
		 *              between successive scanpoints.
		 *
		 * @return      Returns zero on success, non-zero on failure
		 */
		int compute_planar_probs(double dist, double ang);
};

#endif
