#ifndef DOOR_H
#define DOOR_H

/**
 * @file    door.h
 * @author  Eric Turner <elturner@indoorreality.com>
 * @brief   The geometric definition for a door in a building model
 *
 * @section DESCRIPTION
 *
 * This file contains the door_t class, which is used to define the
 * geometric representation of a door in a building model.
 *
 * A door is assumed to be vertically aligned, have some height and some
 * width.  This representation does not include thickness or swing.
 */

#include <Eigen/Dense>
#include <iostream>

/**
 * The door_t represents a door in a building model
 */
class door_t
{
	/* parameters */
	public:

		/**
		 * A center point representing the surface of the door
		 */
		Eigen::Vector3d center;

		/**
		 * The vertical extent of the door is determined
		 * by the floor and ceiling heights at the position
		 * of the door.
		 *
		 * Door is invalid if z_min > z_max
		 */
		double z_min, z_max;

		/**
		 * The horizontal extent of the door is represented
		 * by a 2D line segment.
		 */
		Eigen::Vector2d endpoints[2];

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Needed to ensure proper alignment of class
		 */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/**
		 * Default constructor
		 */
		door_t() : center(0,0,0), z_min(1), z_max(0)
		{
			this->endpoints[0] << 0,0;
			this->endpoints[1] << 0,0;
		};

		/**
		 * Basic constructor of door position
		 *
		 * @param p   The center position of door
		 */
		door_t(const Eigen::Vector3d& p) 
			: center(p), z_min(p(2)), z_max(p(2))
		{
			this->endpoints[0] << 0,0;
			this->endpoints[1] << 0,0;
		};

		/**
		 * The copy constructor
		 */
		door_t(const door_t& other) 
			: center(other.center), 
			  z_min(other.z_min),
			  z_max(other.z_max),
			  endpoints(other.endpoints)
		{
			this->endpoints[0] = other.endpoints[0];
			this->endpoints[1] = other.endpoints[1];
		};

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Exports the door as a set of points to the
		 * specified XYZ file stream.
		 *
		 * @param os   The output file stream to write to
		 */
		void writexyz(std::ostream& os) const;

		/**
		 * Exports the door as a surface to the specified
		 * Wavefront OBJ file stream.
		 *
		 * @param os   The output file stream to write to
		 */
		void writeobj(std::ostream& os) const;

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Copies information from the specified struct
		 *
		 * @param other   The struct to copy
		 */
		inline door_t& operator = (const door_t& other)
		{
			this->center        = other.center;
			this->z_min         = other.z_min;
			this->z_max         = other.z_max;
			this->endpoints[0]  = other.endpoints[0];
			this->endpoints[1]  = other.endpoints[1];
			return *this;
		};
};

#endif
