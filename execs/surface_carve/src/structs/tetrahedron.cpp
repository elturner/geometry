#include "tetrahedron.h"

inline double orient3D(double px, double py, double pz,
			double qx, double qy, double qz,
			double rx, double ry, double rz,
			double sx, double sy, double sz)
{
	return (px-sx) * ( (qy-sy)*(rz-sz) - (qz-sz)*(ry-sy) )
		- (py-sy) * ( (qx-sx)*(rz-sz) - (qz-sz)*(rx-sx) )
		+ (pz-sz) * ( (qx-sx)*(ry-sy) - (qy-sy)*(rx-sx) );
}

bool inside_tet(tetrahedron_t& tet, double x, double y, double z)
{
	/* get whether the tet is oriented positively */
	double tet_parity = orient3D(tet.a.x, tet.a.y, tet.a.z,
				tet.b.x, tet.b.y, tet.b.z,
				tet.c.x, tet.c.y, tet.c.z,
				tet.pose.x, tet.pose.y, tet.pose.z);
	
	/* get orientation of point off of all faces */
	double p_side = orient3D(tet.a.x, tet.a.y, tet.a.z,
				tet.b.x, tet.b.y, tet.b.z,
				tet.c.x, tet.c.y, tet.c.z,
				x, y, z); /* pos if inside */
	double a_side = orient3D(tet.b.x, tet.b.y, tet.b.z,
				tet.pose.x, tet.pose.y, tet.pose.z,
				tet.c.x, tet.c.y, tet.c.z,
				x, y, z); /* pos if inside */
	double b_side = orient3D(tet.a.x, tet.a.y, tet.a.z,
				tet.c.x, tet.c.y, tet.c.z,
				tet.pose.x, tet.pose.y, tet.pose.z,
				x, y, z); /* pos if inside */
	double c_side = orient3D(tet.a.x, tet.a.y, tet.a.z,
				tet.pose.x, tet.pose.y, tet.pose.z,
				tet.b.x, tet.b.y, tet.b.z,
				x, y, z);

	/* fix all parity */
	p_side *= tet_parity;
	a_side *= tet_parity;
	b_side *= tet_parity;
	c_side *= tet_parity;

	/* check all parity */
	return ( (p_side >= 0) && (a_side >= 0) 
			&& (b_side >= 0) && (c_side >= 0) );
}
