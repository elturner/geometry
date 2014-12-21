#ifndef REGION_MESHER_H
#define REGION_MESHER_H

/**
 * @file    region_mesher.h
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Generates a watertight mesh based on a set of planar regions
 *
 * @section DESCRIPTION
 *
 * This file contains the region_mesher_t class, which generates a unified
 * mesh based on a set of planar regions.  The mesh will be aligned with
 * the planar geometry described by the regions, and will (attempt to) use
 * an efficient number of triangles to represent these surfaces.
 */

#include <io/mesh/mesh_io.h>
#include <geometry/octree/octree.h>
#include <geometry/shapes/plane.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region_graph.h>
#include <mesh/surface/node_corner_map.h>
#include <mesh/surface/node_corner.h>
#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include <map>
#include <set>

/**
 * This namespace holds the classes used to mesh planar regions
 *
 * The region mesher will generate a watertight mesh that aligns to
 * the planar regions described in a planar_region_graph_t object.
 */
namespace region_mesher
{
	/* the following classes are defined in this namespace */
	class mesher_t;
	class vertex_info_t;
	class region_info_t;

	/* The following typedefs are used to make the containers of the
	 * classes in this file more human-readable */
	typedef std::map<node_corner::corner_t, vertex_info_t > vertmap_t;
	typedef std::map<node_face_t, region_info_t>            planemap_t;

	/**
	 * The mesher_t class is used to generate a unified mesh
	 * based on the given set of planar regions.  These regions are 
	 * used to represent planar geometry in a model.  The output mesh
	 * will attempt to align to this geometry, and use an efficient 
	 * number of elements to represent it.
	 */
	class mesher_t
	{
		/* parameters */
		private:
	
			/**
			 * The list of vertices in the model
			 *
			 * Each vertex is originally a corner of a node
			 * in the octree.  The corners stored here are ones
			 * that appear on the surface boundary AND are 
			 * connected between two or more planar regions.  
			 * We also store the list of intersecting planar 
			 * regions for each corner, since this affects the
			 * constraints imposed on the vertex position.
			 *
			 * The final fitted position of each vertex is
			 * also stored here.
			 */
			vertmap_t vertices;
	
			/**
			 * Stores region/plane information
			 *
			 * This map goes from region seed faces to the
			 * region info stored in this structure.  Each
			 * region keeps track of the ordered lists of
			 * vertices that compose its boundary edges,
			 * as well as the original plane-fit for the 
			 * region.
			 */
			planemap_t regions;

			/* the following members represent parameters
			 * of this algorithm */

			/**
			 * The threshold to use when smoothing
			 * the octtopo via outlier removal.
			 *
			 * Indicates percentage of surface area
			 * of node that has to disagree with node's
			 * flag for it to be flipped.
			 *
			 * range: (0.5, 1.0]
			 */
			double node_outlierthresh;

			/**
			 * The coalescing distance threshold to use
			 * when forming regions for this mesher
			 *
			 * units:  std devs
			 */
			double coalesce_distthresh;

			/**
			 * The coalescing plane threshold to use
			 * when forming regions for this mesher
			 *
			 * range: [0,1]
			 */
			double coalesce_planethresh;

			/**
			 * Whether or not to use the isosurface
			 * position of each node face's center
			 * when forming the plane positions.
			 */
			bool use_isosurface_pos;

			/**
			 * The following threshold parameter is used to
			 * determine when two neighboring planes are
			 * close enough to parallel to be considered
			 * the same surface when finding the vertex
			 * positions on this mesh.
			 *
			 * This value represents the minimum singular
			 * value of the SVD decomposition of plane normals
			 * that still indicates valid geometry.
			 */
			double min_singular_value;

			/**
			 * When performing region boundary simplification,
			 * the colinearity of neighboring vertices is
			 * considered when removing unnecessary geometry.
			 *
			 * This value indicates the required colinearity
			 * of neighboring pixels.  Any edges whose dot
			 * product's absolute value is greater than this
			 * threshold will not be simplified.
			 *
			 * units:  unitless (range [0,1])
			 */
			double max_colinearity;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/
	
			/**
			 * Default constructor.  Forms an empty mesh
			 */
			mesher_t();
	
			/**
			 * Frees all memory and resources
			 */
			~mesher_t();
	
			/*----------------*/
			/* initialization */
			/*----------------*/
	
			/**
			 * Clears all memory and resources
			 */
			void clear();

			/**
			 * Imports the settings specified in the given
			 * .xml file into this structure.
			 *
			 * This should be performed before calling init(),
			 * since that call will use these settings.
			 *
			 * @param xml_settings   The xml settings file to
			 *                       read from
			 *
			 * @return     Returns zero on success, non-zero 
			 *             on failure
			 */
			int import(const std::string& xml_settings);

			/**
			 * Initializes this structure from the given planar
			 * region graph.
			 *
			 * Given a set of planar regions, will process them
			 * to incorporate them in this mesh.
			 *
			 * @param tree             The original octree
			 * @param region_graph     The set of planar 
			 *                         regions to use
			 * @param corner_map       The set of corners
			 *                         in this model
			 *
			 * @return         Returns zero on success, 
			 *                 non-zero on failure.
			 */
			int init(const octree_t& tree,
			    const planar_region_graph_t& region_graph,
			    const node_corner::corner_map_t& corner_map);

			/*-----------*/
			/* accessors */
			/*-----------*/

			/**
			 * Retrieves the value for the node outlier
			 * threshold.
			 */
			inline double get_node_outlierthresh() const
			{ return this->node_outlierthresh; };
			
			/**
			 * Retrieves the value for coalescing distance
			 * threshold.
			 */
			inline double get_coalesce_distthresh() const
			{ return this->coalesce_distthresh; };

			/**
			 * Retrieves the value for coalescing plane
			 * threshold.
			 */
			inline double get_coalesce_planethresh() const
			{ return this->coalesce_planethresh; };

			/**
			 * Retrieves the flag for whether to use
			 * the isosurface position for face centers.
			 */
			inline bool get_use_isosurface_pos() const
			{ return this->use_isosurface_pos; };

			/**
			 * Retrieves the flag for minimum singular
			 * value threshold.
			 */
			inline double get_min_singular_value() const
			{ return this->min_singular_value; };

			/*-----*/
			/* i/o */
			/*-----*/
	
			/**
			 * Consolidates this mesh into the specified mesh 
			 * object
			 *
			 * Note that the elements of this mesh will be 
			 * added to any existing information in the 
			 * supplied mesh.  If you want ONLY this mesh, 
			 * then make sure that the argument
			 * mesh is clear before this call.
			 *
			 * @param mesh   The mesh to add to
			 * @param tree   The originating tree for this model
			 *
			 * @return       Returns zero on success, 
			 *               non-zero on failure.
			 */
			int compute_mesh(mesh_io::mesh_t& mesh,
					const octree_t& tree) const;
	
			/*-----------*/
			/* debugging */
			/*-----------*/

			/**
			 * Exports all vertices to the specified stream
			 * in Wavefront OBJ format.
			 *
			 * @param os   The output stream to write to
			 *
			 * @return     Returns zero on success, non-zero on
			 *             failure.
			 */
			int writeobj_vertices(std::ostream& os) const;

			/**
			 * Exports all corner map edges connected to
			 * region boundary vertices to the specified
			 * Wavefront OBJ output stream.
			 *
			 * @param os    The output stream to write to
			 * @param tree  The original tree of this model
			 * @param cm    The corner map of this model
			 */
			void writeobj_edges(std::ostream& os,
				const octree_t& tree,
				const node_corner::corner_map_t& cm) const;

		/* helper functions */
		private:

			/**
			 * Computes the ideal position of the given vertex
			 * based on the set of regions that intersect it.
			 *
			 * This computes the geometric intersection between
			 * the planes of the regions that intersect the
			 * given vertex. However, it also accounts for
			 * the fact that if two regions have planes that
			 * are nearly parallel, then the intersection
			 * should not be counted.
			 *
			 * @param vit   The iterator to this vertex
			 *
			 * @return      Returns zero on success, non-zero
			 *              on failure.
			 */
			int compute_vertex_pos(vertmap_t::iterator vit);
	};
	
	/**
	 * This class stores the necessary values for each vertex
	 * in this constructed mesh.
	 *
	 * These values include the fitted position of the vertex,
	 * and the set of planar regions that intersect with this vertex.
	 */
	class vertex_info_t
	{
		/* security */
		friend class mesher_t;

		/* parameters */
		private:

			/**
			 * The computed coordinates for this vertex
			 *
			 * This position is constrained based on the 
			 * intersection of the planes that contain this
			 * vertex.
			 */
			Eigen::Vector3d position;

			/**
			 * The following represent the set of planar
			 * regions that contain this vertex.  They
			 * are represented by their seed face objects,
			 * which is how they are indexed in the original
			 * planar region graph.
			 */
			faceset_t regions;

		/* functions */
		public:
		
			/*--------------*/
			/* constructors */
			/*--------------*/

			/* Since this class contains Eigen structures, we
			 * need to properly align memory */
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			/**
			 * Constructs default vertex info
			 */
			vertex_info_t();

			/**
			 * Frees all memory and resources
			 */
			~vertex_info_t();

			/*-----------*/
			/* modifiers */
			/*-----------*/
			
			/**
			 * Clears all info from this structure
			 */
			void clear();

			/**
			 * Adds the given region to this info structure
			 *
			 * A region is represented by its seed face
			 *
			 * @param f   The seed face for a region
			 */
			inline void add(const node_face_t& f)
			{ this->regions.insert(f); };

			/**
			 * Adds all of the given structures regions
			 * to this structure
			 *
			 * After this call, this structure will contain
			 * both its original set and the given argument's
			 * region set.
			 *
			 * @param other   The other info to add to this
			 */
			inline void add(const vertex_info_t& other)
			{ this->regions.insert(other.regions.begin(),
					other.regions.end()); };

			/*-----------*/
			/* accessors */
			/*-----------*/

			/**
			 * Get the number of regions represented
			 * by this info structure.
			 *
			 * This count indicates the number of regions
			 * that intersect with the represented vertex.
			 *
			 * @return   Returns number of intersected regions
			 */
			inline size_t size() const
			{ return this->regions.size(); };

			/**
			 * Returns the beginning iterator to the list
			 * of seed faces for the regions described in
			 * this structure.
			 *
			 * @return   Returns the begin iterator
			 */
			inline faceset_t::const_iterator begin() const
			{ return this->regions.begin(); };

			/**
			 * Returns the ending iterator to the list
			 * of seed faces for the regions described in
			 * this structure.
			 *
			 * @return   Returns the ending iterator
			 */
			inline faceset_t::const_iterator end() const
			{ return this->regions.end(); };

			/**
			 * Retrieve the position of this vertex
			 *
			 * @return   A const reference to the position
			 */
			const Eigen::Vector3d& get_position() const
			{ return this->position; };

			/*-----------*/
			/* operators */
			/*-----------*/

			/**
			 * Copies the values of the specified structure
			 * into this one.
			 *
			 * @param other   The other structure to copy
			 *
			 * @return        Returns the modified structure
			 */
			inline vertex_info_t& operator = (
					const vertex_info_t& other)
			{
				this->position = other.position;
				this->regions.clear();
				this->regions.insert(other.regions.begin(),
						other.regions.end());
				return (*this);
			};
	};
	
	/**
	 * This class stores the necessary values for each region
	 * of this planar mesh.
	 *
	 * These values include the set of boundary lists for the
	 * vertices that compose this planar region, the plane
	 * geometry itself, and other parameters.
	 */
	class region_info_t
	{
		/* security */
		friend class mesher_t;

		/* parameters */
		private:

			/**
			 * This represents all vertices on this region
			 *
			 * These are only the boundary vertices that
			 * are shared with other regions.
			 */
			node_corner::cornerset_t vertices;

			/**
			 * This set represents the subset of faces
			 * in this region that contain at least one
			 * boundary vertex.
			 *
			 * These are the border faces, and will be
			 * meshed differently than the interior
			 * of the region.
			 */
			faceset_t boundary_faces;

			/**
			 * The original information for this region,
			 * such as plane geometry, originating faces, etc.
			 */
			regionmap_t::const_iterator region_it;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/* Since this class contains Eigen structures, we
			 * need to properly align memory */
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW
			
			/**
			 * Constructs empty region with given plane
			 * geometry.
			 *
			 * @param rit   The reference for this region info
			 */
			region_info_t(regionmap_t::const_iterator rit) 
				: region_it(rit)
			{};

			/**
			 * Frees all memory and resources
			 */
			~region_info_t();

			/*-----------*/
			/* accessors */
			/*-----------*/

			/**
			 * Clears all memory and resources
			 *
			 * This will clear any vertex information
			 */
			void clear();

			/**
			 * Adds a vertex to this region
			 *
			 * @param v   The corner representing a vertex
			 */
			inline void add(const node_corner::corner_t& v)
			{ this->vertices.insert(v); };

			/**
			 * Retrieve the plane of this region
			 */
			inline const plane_t& get_plane() const
			{ 
				return this->region_it->
						second.get_region()
							.get_plane();
			};

			/*------------*/
			/* processing */
			/*------------*/

			/**
			 * Will triangulate the topology of this region
			 * and store the results in the provided mesh.
			 *
			 * Given a mesh and a mapping between vertices
			 * and indices, will form triangles that represent
			 * this region that reference the appropriate
			 * vertex indices, and store those triangles in
			 * the given mesh structure.
			 *
			 * Will perform the region meshing based on the
			 * isostuffing approach from Turner and Zakhor 2013,
			 * 3DV.
			 *
			 * @param mesh   Where to store the output triangles
			 * @param vert_ind   The mapping from vertices to
			 *                   indices in the mesh.
			 * @param tree   The originating tree for this model
			 *
			 * @return       Returns zero on success, non-zero
			 *               on failure.
			 */
			int compute_mesh_isostuff(mesh_io::mesh_t& mesh,
				const std::map<node_corner::corner_t,
						size_t>& vert_ind,
					const octree_t& tree) const;

			/*-----------*/
			/* debugging */
			/*-----------*/

			/**
			 * Writes the edges connected to each vertex
			 * of this region to the specified Wavefront
			 * OBJ file stream.
			 *
			 * @param os    The output stream to write to
			 * @param tree  The original tree for this model
			 * @param cm    The corner map for this model
			 */
			void writeobj_edges(std::ostream& os,
				const octree_t& tree,
				const node_corner::corner_map_t& cm) const;

		/* helper functions */
		private:

			/**
			 * Computes mapping matrix for given face direction
			 *
			 * Computes the matrix that maps from world
			 * coordinates onto the surface coordinates
			 * of a region that is represented by the
			 * specified by the given dominant face.
			 *
			 * Example:
			 *
			 * if f = FACE_ZPLUS, then:
			 *
			 *	[x_r;y_r] = [1,0,0; 0,1,0] * [x_w;y_w;z_w]
			 *
			 * @param f   The face that specifies the mapping
			 * @param M   Where to store the mapping matrix
			 */
			static void mapping_matrix(octtopo::CUBE_FACE f,
					Eigen::Matrix<double, 2, 3>& M);
	};
}

#endif
