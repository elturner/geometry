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

#include <geometry/shapes/plane.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <float.h>
#include <Eigen/Dense>
#include <Eigen/StdVector>

/* the following classes are defined in this file */
class planar_region_graph_t;
class planar_region_info_t;
class planar_region_pair_t;

/* the following types are defined in this file */
typedef std::map<node_face_t, planar_region_info_t> regionmap_t;
typedef std::map<node_face_t, node_face_t>          seedmap_t;

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
		 *
		 * This can be thought of as the mapping: regions --> faces
		 */
		regionmap_t regions;

		/**
		 * Represents the mapping: faces --> regions
		 *
		 * This structure can be thought of as the inverse mapping
		 * from the regionmap_t, since it allows fast determination
		 * of which region a given node face is in.
		 */
		seedmap_t seeds;

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

		/**
		 * The distance threshold is used to determine whether
		 * a given face (and its corresponding center position)
		 * are inliers or outliers to a region.
		 *
		 * If a face has a position p with variance v, then
		 * the face will be considered an outlier if the distance
		 * of p to the plane of the region is greater than 
		 * sqrt(v)*distance_threshold.
		 *
		 * Thus, distance_threshold is measured in units of
		 * standard deviations.
		 */
		double distance_threshold;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Constructs this object with default parameters
		 */
		planar_region_graph_t();

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
		 * @param distthresh    The distance threshold to use, in
		 *                      units of standard deviations.
		 */
		void init(double planethresh, double distthresh);

		/*------------*/
		/* processing */
		/*------------*/

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

		/**
		 * Will attempt to coalesce regions in this graph
		 *
		 * This function will join regions in this until no
		 * neighboring regions can join without violating the
		 * error thresholds provided when init() was called.
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int coalesce_regions();

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Writes region geometry to specified Wavefront OBJ file
		 *
		 * Given the location of a .obj file to write to, will
		 * export the node faces for each region to this file.
		 *
		 * @param filename    Where to write the file
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int writeobj(const std::string& filename) const;

		/**
		 * Writes OBJ geometry representing region linkages
		 *
		 * Will iterate over all neighboring regions, and 
		 * write arrows pointing from each region seed to
		 * its neighboring region seeds.
		 *
		 * @param os   The output stream to write to
		 */
		void writeobj_linkages(std::ostream& os) const;

	/* private helper functions */
	private:

		/**
		 * Will compute the best-fit plane for the combination
		 * of the two regions in this pair.
		 *
		 * Will perform PCA analysis on the center points of the
		 * faces in these regions.  Will also compute the
		 * normalized maximal error (in units of standard
		 * deviations) of these center points from the computed
		 * plane.
		 *
		 * These values will be stored in the parameters of this
		 * structure 'plane' and 'max_err' respectively.
		 *
		 * This call will cache statistical information in
		 * this structure.
		 *
		 * @param pair   The pair of regions to analyze
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int compute_planefit(planar_region_pair_t& pair);

		/**
		 * Will combine the two regions into one region
		 *
		 * This function, when given a pair of regions, will
		 * combine their contents into one region.  This process
		 * includes joining face sets, removing the now-outdated
		 * regions from the structure, and updating seed-neighbor
		 * linkages.
		 *
		 * @param pair   The pair of regions to join
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int merge_regions(const planar_region_pair_t& pair);
};

/**
 * This class represents a planar region along with its connectivity info
 */
class planar_region_info_t
{
	/* security */
	friend class planar_region_graph_t;

	/* parameters */
	private:

		/**
		 * The region in question
		 */
		planar_region_t region;

		/**
		 * This set represents the seed faces for each region
		 * that neighbors the region represented by this object
		 *
		 * Note that these links are symmetric, and need to be
		 * updated after each coalescion.
		 */
		faceset_t neighbor_seeds;

		/**
		 * The following vectors store the center positions for
		 * each face in this region.
		 *
		 * If the vector is empty, that means that the centers
		 * have not yet been computed.  Note that the 'centers'
		 * vector should always be the same length as the
		 * 'variances' vector.
		 *
		 * These values are cached so not to be doubly-computed.
		 * Note that they are not necessarily stored in any
		 * particular order, except that each index in 'centers'
		 * corresponds to each index in 'variances'.
		 */
		std::vector<Eigen::Vector3d, 
			Eigen::aligned_allocator<Eigen::Vector3d> > centers;
		std::vector<double> variances;

		/**
		 * The planarity in this region
		 *
		 * This value indicates the planarity of this
		 * region.  If this value is set to be negative, then
		 * that means that the value has not yet been computed.
		 *
		 * Note that this value is not explicitly specified
		 * as the average planarity, min planarity, or max
		 * planarity, since the author (Eric Turner)
		 * wants the freedom to try different methods.
		 */
		double planarity;

	/* functions */
	public:

		/**
		 * Constructs empty region
		 */
		planar_region_info_t()
		{
			this->planarity = -1; /* not yet computed */
		};

		/**
		 * Constructs region based on flood-fill operation
		 *
		 * Given necessary face-linkage information, will perform
		 * flood-fill on the given face in order to form a region.
		 *
		 * @param f           The face to use as a seed for this 
		 *                    region
		 * @param boundary    The node_boundary_t that represents
		 *                    face connectivity
		 * @param blacklist   List of face not allowed to be a part
		 *                    of this region (typically because
		 *                    they've already been allocated
		 *                    another region).
		 * @param planethresh The minimum planarity threshold to use
		 */
		planar_region_info_t(const node_face_t& f,
				const node_boundary_t& boundary,
				faceset_t& blacklist, double planethresh);

		/**
		 * Get the computed planarity of this region
		 *
		 * Will return the cached value for the
		 * planarity of this region.  If this value has not
		 * yet been cached, then it will be computed, cached,
		 * and returned.
		 *
		 * @return   Returns the region's planarity value [0,1]
		 */
		double get_planarity();
};

/**
 * This class is used to represent two neighboring regions for coalescing.
 *
 * Objects of this class contain references to the seeds of two regions,
 * which can be used to compute the merged plane geometry of the two
 * regions.  This can be used to quantify the cost of merging the two
 * regions.
 */
class planar_region_pair_t
{
	/* parameters */
	public:

		/* the two regions to compare, as represented by their
		 * node face seeds */
		node_face_t first;
		node_face_t second;

		/* the following parameters represent the result of
		 * plane fit analysis on the two regions in this pair */
		plane_t plane; /* the computed best-fit plane */
		double err; /* normalized error in the fit */

		/**
		 * The sum of the number of faces in the two regions
		 *
		 * this value represents a checksum on the two regions
		 * described in this structure.  If these regions have
		 * been merged with other regions since this pair was
		 * initialized, then the plane/max_err parameters may
		 * be out of date.  By comparing this value to the
		 * observed sum of faces between the two regions, we
		 * can determine if these parameters should be recomputed.
		 */
		size_t num_faces;

	/* functions */
	public:

		/**
		 * Constructs a default pair
		 */
		planar_region_pair_t()
		{ 
			this->err = DBL_MAX; 
			this->num_faces = 0;
		};

		/**
		 * Constructs a pair from a given pair
		 */
		planar_region_pair_t(const planar_region_pair_t& other)
			:	first(other.first),
				second(other.second),
				plane(other.plane),
				err(other.err),
				num_faces(other.num_faces)
		{};

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Checks if two pairs represent the same set of regions
		 *
		 * Note that this operation is different than calling '==',
		 * since this will check if the regions referenced in the
		 * two pairs are the same, whereas calling '==' will
		 * compare each pair's error value.
		 *
		 * @param other   The other pair to compare to
		 *
		 * @return  Returns true iff pairs are same regions
		 */
		inline bool equivalent_to(
				const planar_region_pair_t& other) const
		{
			return (this->first == other.first) 
				&& (this->second == other.second)
				&& (this->num_faces == other.num_faces);
		};

		/**
		 * Sets the value of this pair, given the provided argument
		 */
		inline planar_region_pair_t& operator = (
				const planar_region_pair_t& other)
		{
			/* copy values */
			this->first = other.first;
			this->second = other.second;
			this->plane = other.plane;
			this->err = other.err;
			this->num_faces = other.num_faces;

			/* return the modified result */
			return (*this);
		};

		/**
		 * Determines if this pair is equal to the provided argument
		 *
		 * Pairs are compared based on the 'max_err' parameter.
		 */
		inline bool operator == (
				const planar_region_pair_t& other) const
		{
			return (this->err == other.err);
		};

		/**
		 * Compares the ordering for two pairs
		 *
		 * Checks if this pair is less than the given argument.
		 *
		 * These are compared based on the negation of the err
		 * parameter, which means that, when sorted, the element
		 * with the highest error will appear first.
		 *
		 * The reason for this is so these pairs can be put into
		 * a priority queue, and the pair with the smallest error
		 * will appear at the top.
		 */
		inline bool operator < (
				const planar_region_pair_t& other) const
		{
			/* sort the largest errors first */
			if(this->err > other.err)
				return true;
			if(this->err < other.err)
				return false;
			
			/* all else being equal, sort the smallest regions
			 * first */
			return (this->num_faces < other.num_faces); 
		};
};

#endif
