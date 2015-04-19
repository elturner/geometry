#ifndef H_VERTEX3_H
#define H_VERTEX3_H

/*
	Vertex3.h

	This function defines a templated vertex type for 3d points
*/

#include <stdlib.h>

template<typename T>
class Vertex3
{

	/* Constructors
	*
	* Construct either with unitialized memory or with a pointer to 
	* a 3 element T array or 3 T elements
	*/
	Vertex3();
	Vertex3(const T* const v) 
	{
		_v[0] = v[0];
		_v[1] = v[1];
		_v[2] = v[2];
	}
	Vertex3(T v0, T v1, T v2)
	{
		_v[0] = v0;
		_v[1] = v1;
		_v[2] = v2;
	}

	/*
	* Access elements or the bare pointer
	*/
	inline const T* ptr() const {return _v;}
	inline const T& operator()(const size_t i) const {return _v[i];}
	inline T& operator()(const size_t i) {return _v[i];}

private:
	T _v[3];
};

#endif