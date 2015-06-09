#ifndef H_OCTTREEHELPER_H
#define H_OCTTREEHELPER_H

/* utility functions for the OctTree */

/* includes */
#include <vector>
#include <cmath>
#include <queue>
#include <utility>
#include <algorithm>

#include "Triangle3.h"
#include "OctTree.h"
#include "ray.h"

#include <iostream>
using std::cout;
using std::endl;

namespace OctTreeHelper 
{

/*
* computes the bounds of the triangles so we know the extents of
* the scene geometry
*/
template<typename T>
void find_bounds(const std::vector<Triangle3<T> >& triangles,
	T* boundx, T* boundy, T* boundz)
{
	// Set the bounds to be insane numbers so that we can
	// find max and min
	boundx[0] = boundy[0] = boundz[0] = 1e30;
	boundx[1] = boundy[1] = boundz[1] = -1e30;

	// loop over the triangles 
	for(size_t i = 0; i < triangles.size(); i++)
		for(size_t j = 0; j < 3; j++)
		{
			if(triangles[i].vertex(j,0) < boundx[0])
				boundx[0] = triangles[i].vertex(j,0);
			if(triangles[i].vertex(j,0) > boundx[1])
				boundx[1] = triangles[i].vertex(j,0);

			if(triangles[i].vertex(j,1) < boundy[0])
				boundy[0] = triangles[i].vertex(j,1);
			if(triangles[i].vertex(j,1) > boundy[1])
				boundy[1] = triangles[i].vertex(j,1);

			if(triangles[i].vertex(j,2) < boundz[0])
				boundz[0] = triangles[i].vertex(j,2);
			if(triangles[i].vertex(j,2) > boundz[1])
				boundz[1] = triangles[i].vertex(j,2);
		}
}

/*
* Builds the Octree using the given triangles.  Destorys any contents
* of the tree before bulding
*/
template<typename T>
OctNode<T> * build(const std::vector<Triangle3<T> >& triangles,
	size_t maxDepth)
{
	// Check if triangles is empty
	if(triangles.empty())
		return nullptr;

	// Compute the maximal bounds of the geometry
	T boundsx[2], boundsy[2], boundsz[2];
	find_bounds<T>(triangles, boundsx, boundsy, boundsz);

	// Compute the mean point
	T center[3] = { (boundsx[0]+boundsx[1])/2,
		(boundsy[0]+boundsy[1])/2,
		(boundsy[0]+boundsy[1])/2};

	// Compute the maximal spread so that is the half width
	T hwx = boundsx[1]-boundsx[0];
	T hwy = boundsy[1]-boundsy[0];
	T hwz = boundsz[1]-boundsz[0];
	T hw = std::max(std::max(hwx,hwy),hwz)/2;

	// Create the root ptr
	OctNode<T> * rootPtr = new OctNode<T>(center[0], center[1], center[2], hw);
	if(rootPtr == nullptr)
		std::cerr << "[OctTreeHelper::build] - Unable to alloc root node!" 
				  << std::endl;

	// Insert all the triangles
	for(size_t i = 0; i < triangles.size(); i++)
		rootPtr->insert(triangles[i], 1, maxDepth);

	// return the root ptr
	return rootPtr;
}

/*
* Tests intersection of the ray with an axis aligned bounding box
*/
template<typename T>
bool ray_aabb_intersection(T* center, 
	T* hws, 
	const Ray<T>& r,
	T t0,
	T t1,
	T& tmin)
{
	/* create the min and max points */
	T parameters[2][3] = {{center[0]-hws[0], center[1]-hws[1], center[2]-hws[2]}, 
		{center[0]+hws[0], center[1]+hws[1], center[2]+hws[2]}};

	/* do the test */
	T tmax, tymin, tymax, tzmin, tzmax;

	tmin = (parameters[r.sign[0]][0] - r.origin.x()) * r.inv_direction.x();
	tmax = (parameters[1-r.sign[0]][0] - r.origin.x()) * r.inv_direction.x();
	tymin = (parameters[r.sign[1]][1] - r.origin.y()) * r.inv_direction.y();
	tymax = (parameters[1-r.sign[1]][1] - r.origin.y()) * r.inv_direction.y();
	if ( (tmin > tymax) || (tymin > tmax) ) 
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;
	tzmin = (parameters[r.sign[2]][2] - r.origin.z()) * r.inv_direction.z();
	tzmax = (parameters[1-r.sign[2]][2] - r.origin.z()) * r.inv_direction.z();
	if ( (tmin > tzmax) || (tzmin > tmax) ) 
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;
	return ( (tmin < t1) && (tmax > t0) );
}

/*
* template function to do the comparison
*/
template<typename T>
class pq_node_t
{
public:
	pq_node_t(OctNode<T>* n, T d) 
		: node(n), dist(d) {};
	OctNode<T> * node;
	T dist;
	inline bool operator <(const pq_node_t& other) const
	{return other.dist < dist;};
};

/*
* Does the ray tracing function 
*/
template<typename T>
bool ray_trace(OctNode<T>* root,
	const T* origin,
	const T* direction,
	T* intersection,
	size_t * id)
{
	/* double check root is not empty */
	if(root == nullptr)
	{
		cout << "root still empty" << endl;
		return false;
	}

	/* first convert this to the internal "ray" type */
	Ray<T> r(Vector3<T>(origin[0], origin[1], origin[2]),
		Vector3<T>(direction[0], direction[1], direction[2]));

	/* create a priority queue of nodes that we will need to look through */
	std::priority_queue< pq_node_t<T> > nodeQueue;
	
	/* Start at the root node and put it in the queue */
	nodeQueue.push(pq_node_t<T>(root, 0));

	/* here is where we do the recursive search */
	OctNode<T> * thisPtr;
	T dist;
	T closestD = 1e30;
	T thisPt[3];
	bool triagleFound = false;
	while(!nodeQueue.empty())
	{
		/* pop the top one off */
		thisPtr = nodeQueue.top().node;
		nodeQueue.pop();

		/* if this node has any contents then we need to check the ray */
		/* against the contents of the node */
		if(!thisPtr->_contents.empty())
		{
			/* For each of the contents we need to check if the ray */
			/* intersects it.  If it does then we need to check if it */
			/* is the nearest object so far */
			for(typename std::list<const Triangle3<T> *>::iterator 
				it = thisPtr->_contents.begin();
				it != thisPtr->_contents.end(); it++)
			{
				if((*it)->intersects_ray(r.origin.ptr(), 
					r.direction.ptr(),
					&dist,
					thisPt))
				{
					if(dist < 0)
						continue;
					if(dist < closestD)
					{
						closestD = dist;
						(*id) = (*it)->id();
						triagleFound = true;
						intersection[0] = thisPt[0];
						intersection[1] = thisPt[1];
						intersection[2] = thisPt[2];
					}
				}
			}


			/* if the queue is empty then we are definately done */
			if(nodeQueue.empty() && triagleFound)
			{
				return true;
			}
			else if(nodeQueue.top().dist > closestD && triagleFound)
			{
				return true;
			}

			/* continue */
			continue;
		}

		/* For each of the non-null children then we need to check if the ray */
		/* intersects it and if so then we need to add it to the queue */
		for(size_t i = 0; i < OctNode<T>::NUM_CHILDREN; i++)
		{
			if(thisPtr->_children[i] == nullptr)
				continue;
			if(ray_aabb_intersection<T>(thisPtr->_children[i]->_center,
				thisPtr->_children[i]->_hw,
				r,
				0, 
				1e30,
				dist))
			{
				nodeQueue.push(
					pq_node_t<T>(thisPtr->_children[i], std::max<T>(0, dist)));
			}
		}
	}

	/* If we ever run out of things to search that means that we intersected */
	/* nothing and we failed to make a hit */
	return false;
}

}

#endif
