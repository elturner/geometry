#ifndef SGN_H
#define SGN_H

/**
 * @file   sgn.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Implements sgn function (which isn't in cmath for some reason)
 *
 * @section DESCRIPTION
 *
 * This file contains the implementation for the sgn() function.
 */

/**
 * Tests the sign of a value
 *
 * If the value is positive, will return +1.
 * If the value is negative, will return -1.
 * If the value is zero, will return 0.
 *
 * @param val   The value to test
 *
 * @return      Returns +1,-1, or 0 based on sign of value.
 */
template <typename T> int sgn(T val) 
{
    return (T(0) < val) - (val < T(0));
};

#endif
