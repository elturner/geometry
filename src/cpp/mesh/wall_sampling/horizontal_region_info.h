#ifndef HORIZONTAL_REGION_INFO_H
#define HORIZONTAL_REGION_INFO_H

/**
 * @file    horizontal_region_info.h
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Stores information about a horizontal planar patch
 *
 * @section DESCRIPTION
 *
 * The horizontal region info structure is used to store properties
 * of a horizontal surface patch.  These are useful when aggregating
 * the locations of floors and ceilings in order separate levels
 * of a scanned environment.
 */

#include <mesh/surface/planar_region.h>

/**
 * The horizontal_region_info_t class is used to store 
 * properties of horizontal regions.
 */
class horizontal_region_info_t
{
	/* parameters */
	public:

		/**
		 * The z-elevation of the originating planar region
		 *
		 * units: meters
		 */
		double z;

		/**
		 * The surface area of the originating planar region
		 *
		 * units: meters squared
		 */
		double surface_area;

		/**
		 * The alignment factor is the magnitude of the projection
		 * of the normal vector onto the xy-plane
		 *
		 * If the alignment factor is close to zero, then the region
		 * is close to being perfectly horizontal.  If the alignment
		 * factor is relatively large, then the regions is slanted,
		 * and less likely to be horizontal.
		 *
		 * range: [0,1]
		 */
		double alignment;
		
		/**
		 * Indicates whether the region has a normal facing up or
		 * down.
		 *
		 * Normal facing up means the region is a floor, whereas
		 * normal facing down means the region is part of a ceiling.
		 */
		bool upnormal;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Initializes this structure based on the given
		 * region, which is assumed to be horizontal.
		 *
		 * Will check the generated region compared to the
		 * thresholds defined in the provided run-time arguments.
		 * If this region does not qualify as a horizontal
		 * region, then this function returns false.  If it
		 * does qualify as a horizontal region, then this
		 * function returns true.
		 *
		 * @param reg                The originating region
		 * @param verticalitythresh  The maximum value of the normal
		 *                           vector's horizontal component
		 *                           to allow for a horizontal
		 *                           surface.
		 * @param floorceilsurfareathresh    The minimum allowed
		 *                           surface area for a horizontal
		 *                           region.  Measured in meters^2.
		 *
		 * @return      Returns true iff input region qualifies
		 *              as large and horizontal.
		 */
		bool init(const planar_region_t& reg,
			double verticalitythresh, 
			double floorceilsurfareathresh);
};

#endif
