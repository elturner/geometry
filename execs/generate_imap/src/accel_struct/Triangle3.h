#ifndef H_TRIANGLE3_H
#define H_TRIANGLE3_H

/*
	Triangle3.h

	This file defines a templated triangle class
*/

#include <cmath>
#include "tribox3.h"
#include "triray3.h"

template<typename T>
class Triangle3
{
public:

	/*
	* Construtors.
	*
	* The class can be build from a set of three vertices.	The normal
	* vector of the triangle is constructed by assuming the points are
	* given in a right handed orientation
	*
	* This makes a copy of the underlying data
	*/
	Triangle3(const T* const v1, 
		const T* const v2, 
		const T* const v3,
		size_t id) 
	: _id(id)
	{
		/* copy the verts into the struct */
		for(int i = 0; i < 3; i++)
		{
			_v[0][i] = v1[i];
			_v[1][i] = v2[i];
			_v[2][i] = v3[i];
		}
		compute_normal();
	}

	/*
	* Access Methods
	* 
	* Allows read access to the underlying points
	*/
	inline T vertex(size_t ti, size_t vi) const
	{
		return _v[ti][vi];
	};
	inline T normal(size_t i) const
	{
		return _n[i];
	};
	inline size_t id() const
	{
		return _id;
	};


	/*
	* Rebuild Triangle function.  Allows the triangle to be rebuilt
	*
	* Recomputes the normal as a side effect
	*/
	inline void rebuild(const T* const v1, 
		const T* const v2, 
		const T* const v3)
	{
		/* copy the verts into the struct */
		for(int i = 0; i < 3; i++)
		{
			_v[0][i] = v1[i];
			_v[1][i] = v2[i];
			_v[2][i] = v3[i];
		}
		compute_normal();
	};

	/*
	* Allows for the setting of a single vertex with normal rebuild
	*
	* Normal is rebuild as a consequence
	*/
	inline void reset_vertex(int i, T* v)
	{
		_v[i][0] = v[0]; 
		_v[i][1] = v[1]; 
		_v[i][2] = v[2]; 
		compute_normal();
	}

	/*
	* Required Intersection Functions
	*/

	/*
	* Tests if the triangle intersects the axis aligned bounding box
	*
	* returns true if the triangle intersects the axis aligned bounding
	* box described by a center point and the half widths along the 3
	* cardinal directions
	*/
	inline bool intersects_aabb(const T* const center, 
		const T* const halfwidths) const 
	{
		return !!triBoxOverlap<T>(center, 
			halfwidths,
			_v,
			_n);
	};

	/*
	* Tests if the triangle is intersected by a ray described
	* by a point on the ray and the direction of the ray.
	*
	* Returns true if the ray intersects the triangle and
	* false if it does not.  
	*
	* If the triangle intersects the ray the intersection
	* point is stored in the intersection variable
	*/
	inline bool intersects_ray(const T* const point, 
		const T* const direction, 
		T* depth,
		T* intersection) const 
	{
		return triangle_ray_intersection<T>(_v,
			_n,
			point,
			direction,
			depth,
			intersection);
	};

private:

	/*
	* Function to compute the normal vector from the verts
	*
	* Assumes the triangle is well formed
	*/
	inline void compute_normal()
	{
		_n[0] = (_v[0][1] - _v[1][1])*(_v[1][2] - _v[2][2]) -
			    (_v[0][2] - _v[1][2])*(_v[1][1] - _v[2][1]);
	    _n[1] = (_v[1][0] - _v[2][0])*(_v[0][2] - _v[1][2]) - 
	            (_v[0][0] - _v[1][0])*(_v[1][2] - _v[2][2]);
	    _n[2] = (_v[0][0] - _v[1][0])*(_v[1][1] - _v[2][1]) - 
	    		(_v[0][1] - _v[1][1])*(_v[1][0] - _v[2][0]);
	    T norm = std::sqrt((_n[0]*_n[0])+(_n[1]*_n[1])+(_n[2]*_n[2]));
	    _n[0] /= norm;
	    _n[1] /= norm;
	    _n[2] /= norm;
	}

	/* this is the 3 points that define the triangle */
	/* these should be given in a right handed orientation */
	T _v[3][3];

	/* this is the normal vector of the triangle */
	T _n[3];

	/* this is an index that identifies the triangle */
	size_t _id;

};


#endif