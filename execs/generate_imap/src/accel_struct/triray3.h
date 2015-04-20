#ifndef H_TRIRAY3_H
#define H_TRIRAY3_H

/*
	triray3.h

	This header file provides the triangle ray intersection functions
*/

#include <cmath>


#include <iostream>
using namespace std;

/*
* performs the triangle_ray intersection test 
* source : http://geomalgorithms.com/a06-_intersect-2.html
*
* This function performs the triangle ray intersection test
*/
template<typename T>
bool triangle_ray_intersection(const T v[3][3],
	const T* const n,
	const T* const p0,
	const T* const d,
	T* depth,
	T* intersection)
{

	/* first thing to do is check to see if the ray even intersects the */
	/* plane */
	T denom = n[0]*d[0] + 
		n[1]*d[1] +
		n[2]*d[2];

	/* if this denominator is zero, then it is parallel and can not */
	/* possibly intersect the triangle */
	if(std::abs(denom) < T(1e-7))
		return false;

	/* then we compute the location of the point of intersection*/
	T r = (v[0][0]-p0[0])*n[0] +
		(v[0][1]-p0[1])*n[1] + 
		(v[0][2]-p0[2])*n[2];
	r /= denom;
	*depth = r;
	T inter[3] = {p0[0]+r*d[0], p0[1]+r*d[1], p0[2]+r*d[2]};

	/* then we need to compute the coordinates of this point using */
	/* the barycetric coordinates. to do that we first need to compute */
	/* u and v the basis vectors and w the point on the plane */
	T uvec[3], vvec[3], wvec[3];
	uvec[0] = v[1][0]-v[0][0];
	uvec[1] = v[1][1]-v[0][1];
	uvec[2] = v[1][2]-v[0][2];
	vvec[0] = v[2][0]-v[0][0];
	vvec[1] = v[2][1]-v[0][1];
	vvec[2] = v[2][2]-v[0][2];
	wvec[0] = inter[0]-v[0][0];
	wvec[1] = inter[1]-v[0][1];
	wvec[2] = inter[2]-v[0][2];

	/* then we need to compute the indivdual dot products */
	T udv = uvec[0]*vvec[0]+uvec[1]*vvec[1]+uvec[2]*vvec[2];
	T wdv = wvec[0]*vvec[0]+wvec[1]*vvec[1]+wvec[2]*vvec[2];
	T vdv = vvec[0]*vvec[0]+vvec[1]*vvec[1]+vvec[2]*vvec[2];
	T wdu = wvec[0]*uvec[0]+wvec[1]*uvec[1]+wvec[2]*uvec[2];
	T udu = uvec[0]*uvec[0]+uvec[1]*uvec[1]+uvec[2]*uvec[2];

	/* check the denominator for malformed triangles */
	T denom2 = udv*udv - udu*vdv;
	if(std::abs(denom2) < T(1e-7))
		return false;

	/* compute s and t */
	T s = (udv*wdv-vdv*wdu)/denom2;
	T t = (udv*wdu-udu*wdv)/denom2;

	/* check if it is inside triangle */
	if(s < T(0) || t < T(0) || (s+t > T(1)))
		return false;
	else
	{
		intersection[0] = inter[0];
		intersection[1] = inter[1];
		intersection[2] = inter[2];
		return true;
	}
}

#endif