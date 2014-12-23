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
#include <geometry/octree/octtopo.h>
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
		 * Performs flood-fill, but also selects faces based on
		 * the given planarity threshold.
		 *
		 * Given a threshold for minimum valid planarity,
		 * will perform flood-fill on the faces around the
		 * given seed face, but will only select faces that
		 * are equal to or greater than the given planarity.
		 *
		 * Note that if the seed face does not meet the threshold,
		 * then it will be considered in a region by itself.
		 *
		 * @param seed          The seed face for the floodfill
		 * @param boundary      The representation of all faces
		 * @param blacklist     A set of faces to ignore
		 * @param planethresh   The planarity threshold to use
		 */
		void floodfill(const node_face_t& seed,
				const node_boundary_t& boundary,
				faceset_t& blacklist, double planethresh);

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
		 * @param useiso    If true, will perform the computations
		 *                  of center positions and variances using
		 *                  isosurface analysis on the nodes of the
		 *                  octree.  If false, will return the
		 *                  positions on the grid of the octree.
		 */
		void find_face_centers(std::vector<Eigen::Vector3d,
				Eigen::aligned_allocator<
					Eigen::Vector3d> >& centers,
				std::vector<double>& variances,
				bool useiso=true) const;

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

		/**
		 * Checks if this region contains this face
		 *
		 * @param f   The face to analyze
		 *
		 * @return    Returns true iff f is in this region
		 */
		inline bool contains(const node_face_t& f) const
		{ return (this->faces.count(f) > 0); };

		/*----------*/
		/* geometry */
		/*----------*/

		/**
		 * Computes the surface area of this region
		 *
		 * This computation will iterate over all the
		 * faces stored in this region, and sum their
		 * surface areas.
		 *
		 * @return  Returns the surface area of this region
		 */
		double surface_area() const;

		/**
		 * Computes the bounding box for a region using the
		 * given basis vectors.
		 *
		 * Given two basis vectors, will compute the bounding
		 * box coordinates (in the specified vector space) of
		 * the given region.
		 *
		 * It is assumed that a,b are orthonormal
		 *
		 * All measurements are taken relative to the center
		 * point of the plane geometry of this region.  That
		 * is, the computed bounding box assumes that the
		 * center of the region's plane is (0,0).
		 *
		 * @param a      The first basis vector to use
		 * @param b      The second basis vector to use
		 * @param a_min  Where to store the min. coefficient 
		 *               of the bounding box for the a-vector
		 * @param a_max  Where to store the max. coefficient 
		 *               of the bounding box for the a-vector
		 * @param b_min  Where to store the min. coefficient 
		 *               of the bounding box for the b-vector
		 * @param b_max  Where to store the max. coefficient 
		 *               of the bounding box for the b-vector
		 */
		void compute_bounding_box(const Eigen::Vector3d& a,
				const Eigen::Vector3d& b,
				double& a_min, double& a_max,
				double& b_min, double& b_max) const;

		/**
		 * Orients the normal vector of this region's plane
		 *
		 * Orient's this region's plane's normal vector
		 * so that it points "inwards".  When the normal
		 * vector is computed originally for the plane,
		 * there is an ambiguity whether it is pointing
		 * inwards or outwards.  This will point it into
		 * the interior of the environment.
		 */
		void orient_normal();

		/**
		 * Computes the dominant axis-aligned face to
		 * represent this region.
		 *
		 * Returns the node face direction that best matches
		 * the normal direction of this region.
		 *
		 * @return   Returns the node face best matching
		 *           this region.
		 */
		octtopo::CUBE_FACE find_dominant_face() const;

		/**
		 * Computes the radius of this region in the L_inf
		 * meaure space.
		 *
		 * The radius is measured from the center of the octree.
		 * which will make this value larger than half the 
		 * end-to-end distance of the region itself.
		 *
		 * @param tree   The originating tree for this model
		 *
		 * @return       Returns radius of region's shape in L_inf
		 */
		double find_inf_radius(const octree_t& tree) const;

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
		 * @param project   If true, will project the mesh
		 *                  onto the plane geometry.
		 */
		void writeobj(std::ostream& os, bool project=false) const;
};

#endif
