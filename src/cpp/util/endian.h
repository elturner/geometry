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
}

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
}

/**
 * Converts quads (four-byte) values from big-endian to little-endian
 *
 * Will return the specified value in little endian format, assuming
 * that the input is in big-endian format.
 *
 * @param x   The two-byte value to convert to little endian
 *
 * @returns   Returns x in little-endian order
 */
inline unsigned int be2leq(unsigned int x)
{
	/* flips the bytes */
	return (((x >>  0) & 255) << 24)
	     | (((x >>  8) & 255) << 16)
	     | (((x >> 16) & 255) <<  8)
	     | (((x >> 24) & 255) <<  0);
}

/**
 * Converts quads (four-byte) values from little-endian to big-endian
 *
 * Will return the specified value in big endian format, assuming
 * that the input is in little-endian format.
 *
 * @param x   The two-byte value to convert to big endian
 *
 * @returns   Returns x in big-endian order
 */
inline unsigned int le2beq(unsigned int x)
{
	/* flips the bytes */
	return (((x >>  0) & 255) << 24)
	     | (((x >>  8) & 255) << 16)
	     | (((x >> 16) & 255) <<  8)
	     | (((x >> 24) & 255) <<  0);
}

/**
 * Converts doubles (eight-byte) values from big-endian to little-endian
 *
 * Will return the specified value in little endian format, assuming
 * that the input is in big-endian format.
 *
 * @param x   The two-byte value to convert to little endian
 *
 * @returns   Returns x in little-endian order
 */
inline double be2led(double x)
{
	union
	{
		double d;
		unsigned int i[2];
	} a, b;

	/* flips the bytes on the halves separately */
	a.d = x;
	b.i[0] = be2leq(a.i[0]);
	b.i[1] = be2leq(a.i[1]);

	/* return flipped result */
	return b.d;
}

/**
 * Converts doubles (eight-byte) values from little-endian to big-endian
 *
 * Will return the specified value in big endian format, assuming
 * that the input is in little-endian format.
 *
 * @param x   The two-byte value to convert to big endian
 *
 * @returns   Returns x in big-endian order
 */
inline double le2bed(double x)
{
	union
	{
		double d;
		unsigned int i[2];
	} a, b;

	/* flips the bytes on the halves separately */
	a.d = x;
	b.i[0] = be2leq(a.i[0]);
	b.i[1] = be2leq(a.i[1]);

	/* return flipped result */
	return b.d;
}

#endif
