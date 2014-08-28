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
 */

#include <mesh/surface/node_boundary.h>
#include <geometry/shapes/plane.h>
#include <iostream>
#include <set>

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
		faceset faces;

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
				faceset& blacklist);

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
