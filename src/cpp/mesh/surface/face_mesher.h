#ifndef FACE_MESHER_H
#define FACE_MESHER_H

/**
 * @file    face_mesher.h
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Converts octree geometry to a dense mesh
 *
 * @section DESCRIPTION
 *
 * This file contains the face_mesher_t class, which is used
 * to convert from node_boundary_t (which stores the boundary
 * faces of octree geometry) to a mesh that can be exported.
 *
 * The meshing technique used is to represent each face as a
 * vertex in the output, and each corner of the octree as a
 * polygon.  In this sense, it is a dual method (much like
 * dual contouring).
 */

#include <io/mesh/mesh_io.h>
#include <geometry/octree/octree.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/node_corner_map.h>
#include <map>

/**
 * This class is used to convert node_boundary_t faces to a polygonal
 * mesh to export
 *
 * The meshing technique used is to represent each boundary face
 * of the octree as a vertex in the output mesh, and each corner
 * of the octree as a polygon.  In this sense, it is a dual method,
 * much like dual contouring.
 */
class face_mesher_t
{
	/* parameters */
	private:

		/**
		 * This structure represents the output mesh
		 */
		mesh_io::mesh_t mesh;

		/**
		 * This map represents the conversion from node faces to
		 * vertices.
		 *
		 * Each node face is given an index, which corresponds
		 * to a vertex index in the output mesh.
		 */
		std::map<node_face_t, size_t> face_index_map;

	/* functions */
	public:

		/*-----------*/
		/* modifiers */
		/*-----------*/

		/**
		 * Clears all information from this structure
		 */
		inline void clear()
		{
			/* clear each individual structure */
			this->mesh.clear();
			this->face_index_map.clear();
		};

		/**
		 * Adds all faces and corners in the given boundary 
		 * structure to this mesh.
		 *
		 * Given a set of faces in the given node boundary
		 * structure, will compute the node corners and add
		 * all faces to this mesh.
		 *
		 * Assuming that the boundary structure is complete and
		 * watertight, then the resulting mesh will also be
		 * watertight.
		 *
		 * This function can either be called directly, or
		 * the user can manually generate a corner_map_t
		 * from this boundary and call add(corner_map), which
		 * will do the same thing.
		 *
		 * @param tree       The original octree
		 * @param boundary   The set of faces to add
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int add(const octree_t& tree, 
		        const node_boundary_t& boundary);

		/**
		 * Adds all faces and corners in the given corner map
		 * to this mesh.
		 *
		 * Given a set of corners with face neighbor information,
		 * will add all corners and faces to this mesh as polygons
		 * and vertices, respectively.
		 *
		 * Assuming that all corners and faces are represented
		 * for a watertight section of the octree, then the
		 * resulting mesh will also be watertight.
		 *
		 * This function can either be called directly with a set
		 * of corners with node information, or by calling add()
		 * on the node_boundary_t object of the same set of faces.
		 *
		 * @param tree      The original octree
		 * @param corners   The corners (and faces) to add
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int add(const octree_t& tree,
		        const node_corner::corner_map_t& corners);

		/**
		 * Adds the given node face as a vertex in this mesh
		 *
		 * Given a node face, will insert it into the mesh as
		 * a new vertex.  If the face already exists in the mesh,
		 * no operation is performed.
		 *
		 * @param face    The face to add
		 *
		 * @return   The index of the vertex defined by this face
		 */
		size_t add(const node_face_t& face);

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Retrieves the resulting mesh
		 *
		 * This function will return a const reference to
		 * the mesh that has been populated so far.
		 *
		 * @return    Returns a reference to the output mesh
		 */
		inline const mesh_io::mesh_t get_mesh() const
		{ return this->mesh; };
};

#endif
