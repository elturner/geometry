#include "door.h"
#include <Eigen/Dense>
#include <iostream>

/**
 * @file    door.cpp
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

using namespace std;
using namespace Eigen;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

void door_t::writexyz(std::ostream& os) const
{
	size_t i, j, n;
	Vector2d dir, p;
	double height, width;

	/* get dimenions of door */
	height = this->z_max - this->z_min;
	dir = this->endpoints[1] - this->endpoints[0];
	width = dir.norm();
	dir.normalize();

	/* sample points along door */
	n = 100;
	for(i = 0; i < n; i++)
	{
		/* get horizontal position */
		p = this->endpoints[0] + (dir*(i*width/n));
		for(j = 0; j < n; j++)
		{
			os << p(0) << " " << p(1) << " "
			   << this->z_min + (j*height/n) << " "
			   << "255 0 0" << endl;
		}
	}
}

void door_t::writeobj(std::ostream& os) const
{
	/* export vertices */
	os << "v " << this->endpoints[0].transpose()
	   << " "  << this->z_min << " 255 0 0" << endl /* bottom right */
	   << "v " << this->endpoints[0].transpose()
	   << " "  << this->z_max << " 255 0 0" << endl /* top right */
	   << "v " << this->endpoints[1].transpose()
	   << " "  << this->z_max << " 255 0 0" << endl /* top left */
	   << "v " << this->endpoints[1].transpose()
	   << " "  << this->z_min << " 255 0 0" << endl /* bottom left */
	   << "f -4 -3 -2 -1" << endl  /* rectangle face */
	   << "f -1 -2 -3 -4" << endl; /* both directions! */
}
