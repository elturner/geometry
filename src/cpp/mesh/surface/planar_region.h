#ifndef PLANAR_REGION_H
#define PLANAR_REGION_H

/**
 * @file   planar_region.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Defines planar region class, made up of a set of node_face_t's
 * 
 * @section DESCRIPTION
 *
 * This file defines the planar_region_t class, which is used to cluster
 * node_face_t objects into large, planar regions.  This class gives
 * a representation of such regions, as well as a way to generate them
 * from a set of nodes.
 *
 * Note that this class operates on node_boundary_t objects, which should
 * already be populated from an octtopo_t topology.
 *
 * This class requires the Eigen Framework.
 */

#include <mesh/surface/node_boundary.h>
#include <geometry/shapes/plane.h>
#include <iostream>
#include <set>
#include <Eigen/Dense>

/**
 * The planar_region_t class represents a subset of node faces that
 * fall close to a plane.
 *
 * This class stores both the goemetry of the defined plane as well
 * as the subset of faces that it originated from.
 */
class planar_region_t
{
	/* parameters */
	private:

		/**
		 * The set of faces that contribute to this region.
		 */
		faceset_t faces;

		/**
		 * The plane geometry is defined by a normal vector
		 * and a point in 3D space.
		 */
		plane_t plane;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Clears all faces from this plane
		 */
		inline void clear()
		{ this->faces.clear(); };

		/*------------*/
		/* processing */
		/*------------*/

		/**
		 * Performs a flood-fill operation on the given face
		 *
		 * Will initialize this planar region by grouping
		 * all faces that are connected to the given face AND
		 * are exactly on the same plane as the given face.
		 *
		 * Any information stored in this planar region
		 * will be destroyed by this function call.
		 *
		 * After this call, all faces added to this region
		 * will also be added to the blacklist set.
		 *
		 * @param seed      The seed face for the floodfill
		 * @param boundary  The representation of all faces
		 * @param blacklist A set of faces to ignore
		 */
		void floodfill(const node_face_t& seed,
				const node_boundary_t& boundary,
				faceset_t& blacklist);

		/**
		 * Finds the center points to all faces in this region
		 *
		 * Will iterate through all faces in this region, compute
		 * the center point of those faces, and add those points
		 * to the given vector.
		 *
		 * Note that any values stored in the specified vector
		 * will remain after this call -- this code does not
		 * delete, it only appends.
		 *
		 * Also note that the centers are computed by determining
		 * the isosurface offset of the face, not the grid-aligned
		 * face centers.
		 *
		 * This function will also find the variances for
		 * the positions of these center points, which will be
		 * stored in the 'variances' vector.
		 *
		 * NOTE: it is assumed that these two vectors have
		 * the same size.
		 *
		 * @param centers   Where to store the center points
		 *                  of faces for this region
		 * @param variances Where to store the variance values
		 *                  for each center point.
		 */
		void find_face_centers(std::vector<Eigen::Vector3d,
				Eigen::aligned_allocator<
					Eigen::Vector3d> >& centers,
				std::vector<double>& variances) const;

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Retrieves the plane information for this region
		 *
		 * @return    Returns a reference to the plane geometry
		 */
		inline const plane_t& get_plane() const
		{ return this->plane; };

		/**
		 * Sets the plane of this region to the given geometry
		 *
		 * @param p  The plane to use for this region
		 */
		inline void set_plane(const plane_t& p)
		{ this->plane = p; };

		/**
		 * Adds a face to this region
		 *
		 * Note that this call does not modify the planar
		 * geometry at all.  It will simply add the face
		 * to the set of faces contained in this region.
		 *
		 * If the face was already a part of this region,
		 * then this function is a no-op.
		 *
		 * @param f   The face to add
		 */
		inline void add(const node_face_t& f)
		{ this->faces.insert(f); };

		/**
		 * Returns the beginning iterator to the set of faces 
		 * in this region.
		 *
		 * @return   Returns a constant iterator to the faces
		 */
		inline faceset_t::const_iterator begin() const
		{ return this->faces.begin(); };

		/**
		 * Returns the end iterator to the set of faces 
		 * in this region.
		 *
		 * @return   Returns a constant iterator to the faces
		 */
		inline faceset_t::const_iterator end() const
		{ return this->faces.end(); };

		/**
		 * Returns the number of faces stored in this region
		 *
		 * @return   The number of faces in this region
		 */
		inline size_t num_faces() const
		{ return this->faces.size(); };


		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Exports this region to an Wavefront OBJ output stream
		 *
		 * Will write the face geometry defined by this region
		 * to the given output stream.
		 *
		 * @param os   The output stream to write to
		 */
		void writeobj(std::ostream& os) const;
};

#endif
