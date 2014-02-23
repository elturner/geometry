#ifndef OCTDATA_H
#define OCTDATA_H

/**
 * @file octdata.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This is the virtual class used to store data in an octree_t.  Arbitrary
 * data can be stored in the octree by extending this class and adding
 * these to the tree.
 */

#include <iostream>

/* this class represents the data that are stored in the nodes of
 * the octree.  This is only interesting at the leaves. */
class octdata_t
{
	/* functions */
	public:
		
		/* constructors */

		/**
		 * Frees all memory and resources
		 */
		virtual ~octdata_t() =0;

		/* accessors */

		/**
		 * Merges the given data into this object
		 *
		 * This will be called when data are to be inserted
		 * into a tree node that is already populated.  This
		 * function should be implemented so to be communitive.
		 * That is, it should be the case that X.merge(Y) results
		 * in X containing the same information as Y would contain
		 * if Y.merge(X) was called.
		 *
		 * @param p  The data to merge into this object
		 */
		virtual void merge(octdata_t* p) =0;

		/**
		 * Clones this object.
		 *
		 * Allocates new memory that is a deep clone of
		 * this data object.  This memory will eventually
		 * need to be freed.
		 *
		 * @return  Returns a deep-copy of this object.
		 */
		virtual octdata_t* clone() =0;

		/* i/o */

		/**
		 * Serializes these data to binary stream
		 *
		 * Will write all necessary data to the binary stream
		 * so that the information in this object can be
		 * fully recovered.
		 *
		 * @param os  The output binary stream to write to
		 */
		virtual void serialize(std::ostream& os) =0;

		/**
		 * Parses stream to populate this object
		 *
		 * Will read the input binary stream and parse
		 * the object represented.  This should be in the
		 * same format as what is written by the serialize()
		 * function.
		 *
		 * @param is  The input binary stream to read from
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		virtual int parse(std::istream& is) =0;
};

#endif
