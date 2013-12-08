#ifndef ENDIAN_H
#define ENDIAN_H

/**
 * @file endian.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file includes functions that can convert values between
 * big and little endian formatting.  It is meant as a very
 * simple replacement for (some of) the functions in endian.h
 * on linux platforms.
 */

/**
 * Converts short (two-byte) values from big-endian to little-endian
 *
 * Will return the specified value in little endian format, assuming
 * that the input is in big-endian format.
 *
 * @param x   The two-byte value to convert to little endian
 *
 * @returns   Returns x in little-endian order
 */
inline unsigned short be2les(unsigned short x)
{
	/* flips the bytes */
	return ((x & 255) << 8) | ((x >> 8) & 255);
};

/**
 * Converts short (two-byte) values from little-endian to big-endian
 *
 * Will return the specified value in big endian format, assuming
 * that the input is in little-endian format.
 *
 * @param x   The two-byte value to convert to big endian
 *
 * @returns   Returns x in big-endian order
 */
inline unsigned short le2bes(unsigned short x)
{
	/* flips the bytes */
	return ((x & 255) << 8) | ((x >> 8) & 255);
};


#endif
