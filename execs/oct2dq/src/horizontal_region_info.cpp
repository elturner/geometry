#include "horizontal_region_info.h"
#include "oct2dq_run_settings.h"
#include <mesh/surface/planar_region.h>

/**
 * @file    horizontal_region_info.cpp
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

using namespace std;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

bool horizontal_region_info_t::init(const planar_region_t& reg,
			const oct2dq_run_settings_t& args)
{
	double nx, ny;

	/* get the normal vector's components */
	nx      = reg.get_plane().normal(0);
	ny      = reg.get_plane().normal(1);

	/* get the magnitude of the projection of the
	 * normal onto the xy-plane */
	this->alignment = sqrt( nx*nx + ny*ny );

	/* check this alignment against the threshold for
	 * horizontal regions.  If the value is too large,
	 * then this region is not horizontal enough and
	 * should be rejected. */
	if(this->alignment >= args.verticalitythresh)
		return false; /* too slanted */

	/* get the surface area of this region */
	this->surface_area = reg.surface_area();

	/* Check if this region is large enough to be
	 * considered an inlier horizontal surface by
	 * checking its surface area against our size
	 * threshold. */
	if(this->surface_area < args.floorceilsurfareathresh)
		return false; /* too small */

	/* if got here, then we have a large, horizontal
	 * region.  Now we check whether the normal
	 * is facing up or down, which helps us determine
	 * if this region is a floor or a ceiling. */
	this->upnormal = (reg.get_plane().normal(2) > 0);
	
	/* record the elevation of this region */
	this->z = reg.get_plane().point(2);

	/* we have successfully found a large horizontal region */
	return true;
}
