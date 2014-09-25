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
		 * The new vertex will be placed at the face's isosurface
		 * position.
		 *
		 * @param face    The face to add
		 *
		 * @return   The index of the vertex defined by this face
		 */
		size_t add(const node_face_t& face);

		/**
		 * Adds the given node face as a vertex in this mesh
		 *
		 * Given a node face and the position of the vertex that
		 * corresponds to this face, will insert it into the mesh
		 * as a new vertex.  If the face is already present in the
		 * mesh, then nop operation is performed.
		 *
		 * @param face     The face to add
		 * @param pos      The position of the vertex for this face
		 *
		 * @return   Returns the index of the verted defined by this
		 *           face.
		 */
		size_t add(const node_face_t& face,
		           const Eigen::Vector3d& pos);

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

	/* helper functions */
	private:

		/**
		 * Computes the vertex position for the given face
		 *
		 * Given a face and a populated corner map, will determine
		 * the best vertex position for this face based on both
		 * its isosurface position and the interpolated probability
		 * values at the corners.
		 *
		 * @param pos          Where to store the position for the
		 *                     face's vertex.
		 * @param tree         The originating octree
		 * @param face         The face to analyze
		 * @param corner_map   The corner map to use
		 */
		static void get_face_pos(Eigen::Vector3d& pos,
			const octree_t& tree, const node_face_t& face, 
			const node_corner::corner_map_t& corner_map);

		/**
		 * Computes the probability value at the specified corner
		 *
		 * Will assume the corner map is populated with faces
		 * for each corner.  Will get the nodes from these faces
		 * and perform a weighted average to obtain the probability
		 * value interpolated at the corner.
		 *
		 * The probabilities from each node is weighted inversely
		 * to the distance of the corner to the node center.
		 *
		 * @param corner      The corner to analyze
		 * @param tree        The originating octree
		 * @param corner_map  The corner map to use
		 *
		 * @return   The interpolated probability value
		 */
		static double get_corner_prob(
			const node_corner::corner_t& corner,
			const octree_t& tree,
			const node_corner::corner_map_t& corner_map);
};

#endif
