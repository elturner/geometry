#ifndef H_OCTTREE_H
#define H_OCTTREE_H

/*
	OctTree.h 

	This header files defines an octree that can accept any 
	number of shape objects so long as the are derived from
	OctGeometry.  Its main purpose is to support ray tracing
	operations
*/

/* includes */
#include <list>
#include <vector>
#include <ostream>

#include "Triangle3.h"

/* Declare classess upfront */
template<typename T> class OctNode;
template<typename T> class OctTree;

/* include the helper */
#include "OctTreeHelper.h"

/* The OctNode class implementation */
template<typename T>
class OctNode
{
public:

	/* enum that defines the number of children */
	enum {NUM_CHILDREN = 8};

	/*
	* Constructors.  Can be constructed with a single half width or
	* 3 different halfwidths for each of the axis
	*/
	OctNode(T cx, T cy, T cz, T hw)
	{
		_center[0] = cx;
		_center[1] = cy;
		_center[2] = cz;
		_hw[0] = _hw[1] = _hw[2] = hw;
		for(int i = 0; i < NUM_CHILDREN; i++)
			_children[i] = nullptr;
	};
	OctNode(T cx, T cy, T cz, T hwx, T hwy, T hwz)
	{
		_center[0] = cx;
		_center[1] = cy;
		_center[2] = cz;
		_hw[0] = hwx;
		_hw[1] = hwy;
		_hw[2] = hwz;
		for(int i = 0; i < NUM_CHILDREN; i++)
			_children[i] = nullptr;
	};

	/*
	* Destructor.  Recursively cleans up its own children
	*/
	~OctNode()
	{
		for(int i = 0; i < NUM_CHILDREN; i++)
			if(_children[i] != nullptr)
			{
				delete _children[i];
				_children[i] = nullptr;
			}
	}

	/*
	* subdivide function
	*
	* splits the node into its eight children.  If it has already been
	* split then this function does nothing
	*/
	inline void subdivide()
	{
		// Check if this node is already split
		if(!is_leaf())
			return;

		// Create the children
		_children[0] = new OctNode<T>(_center[0]-_hw[0]/2,
			_center[1]-_hw[1]/2,
			_center[2]+_hw[2]/2,
			_hw[0]/2, _hw[1]/2, _hw[2]/2);
		_children[1] = new OctNode<T>(_center[0]+_hw[0]/2,
			_center[1]-_hw[1]/2,
			_center[2]+_hw[2]/2,
			_hw[0]/2, _hw[1]/2, _hw[2]/2);
		_children[2] = new OctNode<T>(_center[0]+_hw[0]/2,
			_center[1]+_hw[1]/2,
			_center[2]+_hw[2]/2,
			_hw[0]/2, _hw[1]/2, _hw[2]/2);
		_children[3] = new OctNode<T>(_center[0]-_hw[0]/2,
			_center[1]+_hw[1]/2,
			_center[2]+_hw[2]/2,
			_hw[0]/2, _hw[1]/2, _hw[2]/2);
		_children[4] = new OctNode<T>(_center[0]-_hw[0]/2,
			_center[1]-_hw[1]/2,
			_center[2]-_hw[2]/2,
			_hw[0]/2, _hw[1]/2, _hw[2]/2);
		_children[5] = new OctNode<T>(_center[0]+_hw[0]/2,
			_center[1]-_hw[1]/2,
			_center[2]-_hw[2]/2,
			_hw[0]/2, _hw[1]/2, _hw[2]/2);
		_children[6] = new OctNode<T>(_center[0]+_hw[0]/2,
			_center[1]+_hw[1]/2,
			_center[2]-_hw[2]/2,
			_hw[0]/2, _hw[1]/2, _hw[2]/2);
		_children[7] = new OctNode<T>(_center[0]-_hw[0]/2,
			_center[1]+_hw[1]/2,
			_center[2]-_hw[2]/2,
			_hw[0]/2, _hw[1]/2, _hw[2]/2);
	}

	/*
	* Checks if this node is a leaf
	*/
	inline bool is_leaf() const
	{
		for(int i = 0; i < NUM_CHILDREN; i++)
			if(_children[i] != nullptr)
				return false;
		return true;
	};

	/*
	* Test if the node is empty
	*/
	inline bool empty() const { return _contents.empty();};

	/*
	* Insert function 
	*/
	inline bool insert(const Triangle3<T>& triangle, 
		size_t currentDepth,
		size_t maxDepth)
	{
		/* if the current depth is equal to the max depth then we just */
		/* insert everything here and return */
		if(maxDepth == currentDepth)
		{
			_contents.push_back(&triangle);
			return true;
		}

		/* If this contains no other elements then we will fill it with */
		/* the triangle */
		if(_contents.empty() && is_leaf())
		{
			_contents.push_back(&triangle);
			return true;
		}

		/* If this contains something and is a leaf then we need to */
		/* subdivide it and insert into the lower level */
		/* subdivide automatically takes care of the check if this is */
		/* a leaf or not */
		subdivide();

		/* then we need to check if the triangle intersects each of the */
		/* child nodes and if so then we need to insert it into them    */
		for(size_t i = 0; i < NUM_CHILDREN; i++)
			if(triangle.intersects_aabb(_children[i]->_center, 
				_children[i]->_hw))
			{
				_children[i]->insert(triangle, currentDepth+1, maxDepth);
			}
			
		/* Then we need to push any of the contents down the tree */
		for(typename std::list<const Triangle3<T> *>::iterator it = _contents.begin();
			it != _contents.end(); it++)
		{
			for(size_t i = 0; i < NUM_CHILDREN; i++)
				if((*it)->intersects_aabb(_children[i]->_center, 
					_children[i]->_hw))
				{
					_children[i]->insert(*(*it), currentDepth+1, maxDepth);
				}
		}

		/* Clear this of any contents */
		_contents.clear();

		/* return success */
		return true;
	}

	/*
	* is trimable function 
	*/
	inline bool is_trimable() const
	{ return is_leaf() && _contents.empty();}

	/*
	*	trim function
	*/
	void trim() 
	{
		/* call trim on all your non-null children first */
		for(size_t i = 0; i < OctNode<T>::NUM_CHILDREN; i++)
			if(_children[i] != nullptr)
				_children[i]->trim();

		/* If we are a leaf node then we cant do anything so return */
		if(is_leaf())
			return;

		/* Then check if the children are deletable */
		for(size_t i = 0; i < OctNode<T>::NUM_CHILDREN; i++)
		{
			if(_children[i] == nullptr)
				continue;

			if(_children[i]->is_trimable())
			{
				delete _children[i];
				_children[i] = nullptr;
			}
		}

	}

	/*
	* print function
	*/
	void print(std::ostream& os)
	{
		os << _center[0] << " "
		   << _center[1] << " "
		   << _center[2] << " "
		   << _hw[0] << " "
		   << _hw[1] << " "
		   << _hw[2] << " " 
		   << _contents.size() << " " << is_leaf() << endl;
		for(size_t i = 0; i < NUM_CHILDREN; i++)
			if(_children[i] != nullptr)
				_children[i]->print(os);
	}

	// This holds the half widths of the box.  We allow _hw to take
	// three values so that we can support oblong node types if we
	// desire
	T _hw[3];

	// This is the center of the box
	T _center[3];

	// This is a list of pointers to 8 OctNode objects that are
	// the children of this node
	// The node IS responsible for these pointers
	//
	// Should always be in the following order :
	// 
	//       3 ---- 2 
	//		/      /|
	//    0 ---- 1  |    z  y
	//    |  7 --|- 6    | /
	//    | /    | /     |/
	//    4 ---- 5        --> x
	//
	OctNode<T> * _children[NUM_CHILDREN];

	// Pointer to the objects that are contained in this node
	// The node is NOT responsible for these pointers
	std::list<const Triangle3<T> *> _contents;
};

/* The OctTree class implementation */
template<typename T>
class OctTree
{
public:

	/*
	* Constructor
	*
	*/
	OctTree(size_t max_depth = 10)
		: _max_depth(max_depth), _root(nullptr) {};
	OctTree(const std::vector<Triangle3<T> >& triangles,
		size_t max_depth = 10)
		: _max_depth(max_depth), _root(nullptr)
	{
		_contents = triangles;
		_root = OctTreeHelper::build<T>(_contents, _max_depth);
		trim();
	}

	/*
	* Destructor.  Handles freeing the contents of the 
	* tree and all associated nodes
	*/
	~OctTree()
	{
		if(_root != nullptr)
		{
			delete _root;
			_root = nullptr;
		}
	}

	/*
	* Test if the tree is empty
	*/
	inline bool empty() const { return (_root != nullptr); };

	/*
	* Rebuild function.  Destroys the contents of the tree and rebuilds
	* with the new geometry.
	*
	* Returns true if the tree was able to rebuild or false if it was
	* not able to rebuild the tree
	*/
	inline bool rebuild(const std::vector<Triangle3<T> >& triangles)
	{
		if(_root != nullptr)
			delete _root;
		_contents = triangles;
		_root = OctTreeHelper::build<T>(_contents, _max_depth);
		trim();
		return (_root != nullptr);
	}

	/*
	*	Access triangle by number
	*/
	inline const Triangle3<T>& triangle(size_t i) const
		{ return _contents[i]; };

	/*
	*	Get number of triangles
	*/
	inline size_t num_triangles() const
		{return _contents.size();};

	/*
	* Ray trace the ray against the geometry stored in the OctTree.
	*
	* This function returns false if it does not intersect any
	* of the geometry in the model
	*
	*/
	inline bool ray_trace(const T* origin, 
		const T* direction,
		T* intersection,
		size_t * id) const
	{
		if(_root == nullptr)
			return false;
		return OctTreeHelper::ray_trace<T>(_root,
			origin,
			direction,
			intersection,
			id);
	};

	/* debug function */

	/*
	* Print to an ostream
	*/
	inline void print(std::ostream& os)
	{
		_root->print(os);
	}

private:

	/*
		Function to trim the OctTree 
	*/
	inline void trim()
	{
		if(_root != nullptr)
			_root->trim();
	};

	// This is the maximal depth of the tree
	size_t _max_depth;

	// This is the root of the tree
	// The tree is responsible for this node
	OctNode<T> * _root;

	// This is the internal list of the contents of the octtree
	std::vector<Triangle3<T> > _contents;
};

#endif
