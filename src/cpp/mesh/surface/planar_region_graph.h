#ifndef PLANAR_REGION_GRAPH_H
#define PLANAR_REGION_GRAPH_H

/**
 * @file   planar_region_graph.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Represents the neighbor/connectivity info for regions
 *
 * @section DESCRIPTION
 *
 * Planar regions are used to represent subsets of node faces generated
 * from an octree.  This file contains the planar_region_graph_t, which
 * is used to organize all the regions within a model, and provide
 * connectivity information between regions (i.e. which regions are adjacent
 * to which other regions).
 *
 * This class also is used to generate the set of regions from the model.
 * It is assumed that the topology of the octree has been constructed using
 * an octtopo_t object, and that the boundary faces of that topology have
 * been generated with a node_boundary_t object.
 */

#include <mesh/surface/node_boundary.h>
#include <map>
#include <Eigen/Dense>

/* the following classes are defined in this file */
class planar_region_graph_t;
class planar_region_info_t;

/* the following types are defined in this file */
typedef std::map<node_face_t, planar_region_info_t> regionmap;

/**
 * The planar_region_graph_t represents the set of regions on a model
 *
 * The set of regions on a model are represented by groupings of the
 * node faces within that model.  Each region also has references to
 * the other regions in the model that it is connected to.  Thi
 * connectivity is determined based on the linkages between node faces.
 * That is, if region A contains node 1, region B contains node 2, and
 * nodes 1 and 2 are linked, then regions A and B are also linked.
 *
 * This class is used to generate a set of coalesced regions, so it must
 * be initialized with a set of parameters that describe how to coalesce
 * each region.
 */
class planar_region_graph_t
{
	/* parameters */
	private:

		/**
		 * Represents all the generated regions and their info
		 *
		 * This mapping goes from seed faces to regions and region
		 * info.  Each region is initialized with a face node that
		 * acts as a seed for the region.  As regions get coalesced,
		 * the total number of regions is reduced but the same
		 * seeds are used to represent the regions.
		 */
		regionmap regions;

		/* the following parameters are for region coalescing */

		/**
		 * The planarity threshold is used to determine if a
		 * face should be added to a plane.  The originating
		 * nodes for faces have estimates of the planarity of
		 * surfaces in that volume.  This is used to compute
		 * the planarity of each face.
		 *
		 * If a face has a planarity below this threshold, then
		 * it cannot be used to coalesce two regions.
		 */
		double planarity_threshold;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Iniitalizes this object based on the given parameters
		 *
		 * This region graph is used to generate a set of coalesced
		 * regions from a set of faces.  In order to coalesce these
		 * regions effectively, it must use the following
		 * parameters.
		 *
		 * Note that this function should be called before calling
		 * populate.  If it is not called, then a set of default
		 * parameters will be used.
		 *
		 * @param planethresh   The planarity threshold [0,1] to use
		 */
		void init(double planethresh);

		/**
		 * Populates the set of regions from the given set of faces
		 *
		 * Given a set of faces represented by a node boundary
		 * object, will populate the regions in this model.
		 *
		 * @param boundary    The boundary faces of the model
		 *
		 * @return     Returns zero on success, non-zero on failure
		 */
		int populate(const node_boundary_t& boundary);

	/* public helper functions */
	public:

		/**
		 * Will compute the planrity estimate for the given face
		 *
		 * Will use the originating octnodes of the specified
		 * face to compute the planarity estimate of that face.
		 * A planarity estimate is a value between 0 and 1, where
		 * 1 is perfectly planar and 0 is not planar at all.
		 *
		 * @param f   The face to analyze
		 *
		 * @return    Returns the planarity estimate of f, [0,1]
		 */
		static double get_face_planarity(const node_face_t& f);

		/**
		 * Computes the position of the center of the face,
		 * assuming alignment with the local isosurface.
		 *
		 * Rather than using the face's built-in get_center()
		 * function, which aligns the center with the octree
		 * grid, this function will compute the center based
		 * on the underlying octdata in order to put the 
		 * face on the isosurface of the probability distribution.
		 *
		 * @param f    The face to analyze
		 * @param p    Where to store the center position
		 */
		 static void get_isosurface_pos(const node_face_t& f,
				 	Eigen::Vector3d& p);

		/**
		 * Computes the variance of the face position along the
		 * normal direction of the face.
		 *
		 * Given a node face to analyze, this function will
		 * determine what the positional variance is for
		 * that face's center point, along the normal of the face.
		 * This computation uses the octdata values in the
		 * originating octnodes that are used to represent this
		 * face.
		 *
		 * @param f     The face to analyze
		 *
		 * @return      Returns the variance in f's center position
		 *              along f's normal vector.
		 */
		 static double get_face_pos_var(const node_face_t& f);
};

#endif
