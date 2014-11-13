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
			 *
			 * @return       Returns zero on success, 
			 *               non-zero on failure.
			 */
			int compute_mesh(mesh_io::mesh_t& mesh) const;
	
			/*-----------*/
			/* debugging */
			/*-----------*/

			/**
			 * Exports all vertices to the specified stream
			 * in Wavefront OBJ format.
			 *
			 * @param os   The output stream to write to
			 */
			void writeobj_vertices(std::ostream& os) const;
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
			/* accessors */
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
			 * This represents the ordered list of boundary
			 * vertices for this region.  Each boundary is
			 * oriented in counter-clockwise order (assuming
			 * the normal of the region is pointing towards
			 * you), and each disjoint boundary is represented
			 * by its own separate list.
			 */
			std::vector<std::vector< node_corner::corner_t > >
				boundaries;

			/**
			 * The plane geometry of this regions is copied
			 * here for convenience.  This should be the
			 * same information as is stored in the original
			 * planar_region_graph_t.
			 */
			plane_t plane;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/* Since this class contains Eigen structures, we
			 * need to properly align memory */
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW
			
			/**
			 * Constructs empty region info object
			 */
			region_info_t();

			/**
			 * Constructs empty region with given plane
			 * geometry.
			 *
			 * @param p   The plane of this region
			 */
			region_info_t(const plane_t& p) : plane(p)
			{};

			/**
			 * Frees all memory and resources
			 */
			~region_info_t();
	};
}

#endif
