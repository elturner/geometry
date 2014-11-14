#ifndef COLOR_H
#define COLOR_H

/**
 * @file   color.h
 * @author Eric Turner
 * @brief  The color_t class represents a R,G,B color
 *
 * @section DESCRIPTION
 *
 * This file contains the color_t class.
 * This file was originally written for CS-184, Fall 2014, Assignment #1,
 * but was modified to be incorporated with the backpack processing code.
 *
 * Last modified:  October 24, 2014
 */

#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <cmath>

/**
 * The color_t class describes the red, green, and blue values of a color
 */
class color_t
{
	/* parameters */
	private:

		/**
		 * The red component of a color
		 *
		 * Component is stored as a float in the range [0,1.0]
		 */
		float red;

		/**
		 * The green component of a color
		 *
		 * Component is stored as a float in range [0,1.0]
		 */
		float green;

		/**
		 * The blue component of a color
		 *
		 * Component is stored as a float in range [0,1.0]
		 */
		float blue;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Constructs default color (black)
		 */
		color_t() : red(0.0f), green(0.0f), blue(0.0f)
		{};

		/**
		 * Constructs color from other color
		 *
		 * @param other   The other color to copy
		 */
		color_t(const color_t& other) 
			: red(other.red), 
			  green(other.green), 
			  blue(other.blue)
		{};

		/**
		 * Constructs color based on r,g,b values as floats
		 *
		 * @param r   The red component to use
		 * @param g   The green component to use
		 * @param b   The blue componenet to use
		 */
		color_t(float r, float g, float b)
			: red(r), green(g), blue(b)
		{};

		/*-----------*/
		/* modifiers */
		/*-----------*/

		/**
		 * Sets the value of this color to the specified components.
		 *
		 * The components are given as floats, in the range [0,1.0]
		 *
		 * @param r   The red component to use
		 * @param g   The green component to use
		 * @param b   The blue componenet to use
		 */
		inline void set(float r, float g, float b)
		{
			this->red   = r;
			this->green = g;
			this->blue  = b;
		};
		
		/**
		 * Sets the red component of this color
		 */
		inline void set_red(float r)
		{ this->red = r; };

		/**
		 * Sets the green component of this color
		 */
		inline void set_green(float g)
		{ this->green = g; };

		/**
		 * Sets the blue component of this color
		 */
		inline void set_blue(float b)
		{ this->blue = b; };

		/**
		 * Sets the value of this color to the specified components.
		 *
		 * The components are given as ints, in the range [0,255]
		 *
		 * @param r   The red component to use
		 * @param g   The green component to use
		 * @param b   The blue componenet to use
		 */
		inline void set_ints(int r, int g, int b)
		{
			this->red   = r / 255.0f;
			this->green = g / 255.0f;
			this->blue  = b / 255.0f;
		};
		
		/**
		 * Sets the red component of this color
		 */
		inline void set_red_int(int r)
		{ this->red = r / 255.0; };

		/**
		 * Sets the green component of this color
		 */
		inline void set_green_int(int g)
		{ this->green = g / 255.0; };

		/**
		 * Sets the blue component of this color
		 */
		inline void set_blue_int(int b)
		{ this->blue = b / 255.0; };
	
		/**
		 * Sets this color to be a random color
		 *
		 * The colors available when choosing a random color
		 * are in the pastel range, so no fully saturated values
		 * will be chosen.
		 */
		inline void set_random()
		{
			int r, g, b;

			/* get some values */
			r = (rand() % 128) + 64;
			g = (rand() % 128) + 64;
			b = (rand() % 128) + 64;
			
			/* set the components */
			this->red   = ((float) r) / 255.0f;
			this->green = ((float) g) / 255.0f;
			this->blue  = ((float) b) / 255.0f;
		};

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Gets the red component of this color
		 */
		inline float get_red() const
		{ return this->red; };

		/**
		 * Gets the red component of this color as an integer
		 */
		inline int get_red_int() const
		{ return color_t::convert_to_int(this->red); };
		
		/**
		 * Gets the red component, bounded to [0,1]
		 */
		inline float get_red_bounded() const
		{ return std::min(1.0f, std::max(0.0f, this->red)); };

		/**
		 * Gets the green component of this color
		 */
		inline float get_green() const
		{ return this->green; };

		/**
		 * Gets the green component of this color as an integer
		 */
		inline int get_green_int() const
		{ return color_t::convert_to_int(this->green); };

		/**
		 * Gets the green component, bounded to [0,1]
		 */
		inline float get_green_bounded() const
		{ return std::min(1.0f, std::max(0.0f, this->green)); };

		/**
		 * Gets the blue component of this color
		 */
		inline float get_blue() const
		{ return this->blue; };

		/**
		 * Gets the blue component of this color as an integer
		 */
		inline int get_blue_int() const
		{ return color_t::convert_to_int(this->blue); };
		
		/**
		 * Gets the blue component, bounded to [0,1]
		 */
		inline float get_blue_bounded() const
		{ return std::min(1.0f, std::max(0.0f, this->blue)); };

		/**
		 * Computes the grayscale value of this color as a float
		 *
		 * @return   Returns grayscale average of color
		 */
		inline float get_grayscale() const
		{ 
			return ( this->get_red() 
					+ this->get_green() 
					+ this->get_blue() ) / 3.0f; 
		};

		/**
		 * Computes the grayscale value of this color as an int
		 *
		 * @return   Returns grayscale average as an int
		 */
		inline int get_grayscale_int() const
		{ return color_t::convert_to_int(this->get_grayscale()); };

		/**
		 * Gets the bounded grayscale value, bounded to [0,1]
		 *
		 * @return   Returns bounded grayscale average
		 */
		inline float get_grayscale_bounded() const
		{
			return std::min(1.0f, 
				std::max(0.0f, this->get_grayscale())); 
		};

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Copies the given value to this color
		 *
		 * @param other   The other color to copy
		 *
		 * @return   Returns the modified color
		 */
		inline color_t& operator = (const color_t& other)
		{
			/* copy values */
			this->red   = other.red;
			this->green = other.green;
			this->blue  = other.blue;

			/* return result */
			return (*this);
		};

		/**
		 * Checks if two colors are equal
		 *
		 * The check that colors are equal will compare
		 * the integer representation of the color components,
		 * to avoid rounding error.
		 *
		 * @param other   The other color to compare
		 *
		 * @return        Returns true iff the colors are equal
		 */
		inline bool operator == (const color_t& other) const
		{
			/* compare integer representations */
			return ((this->get_red_int() 
				 	== other.get_red_int())
				&& (this->get_green_int() 
					== other.get_green_int())
				&& (this->get_blue_int()
					== other.get_blue_int()));
		};

		/**
		 * Compares two colors for sorting
		 *
		 * A comparison operator is useful if colors are to be
		 * placed in a std::set or std::map, which use RB-trees.
		 *
		 * @param other   The other color to compare
		 *
		 * @return     Returns true iff this color is less than
		 *             the other color.
		 */
		inline bool operator < (const color_t& other) const
		{
			int i,o;

			/* compare reds */
			i = this->get_red_int();
			o = other.get_red_int();
			if(i < o)	return true;
			if(i > o)	return false;

			/* compare greens */
			i = this->get_green_int();
			o = other.get_green_int();
			if(i < o)	return true;
			if(i > o)	return false;

			/* compare blues */
			i = this->get_blue_int();
			o = other.get_blue_int();
			return (i < o);
		};

		/**
		 * Adds two colors together
		 *
		 * Uses basic superposition to do a component-wise addition
		 * of colors.
		 *
		 * @param other    The other color to add to this one
		 *
		 * @return   Returns the newly created sum of colors
		 */
		inline color_t operator + (const color_t& other) const
		{
			/* add components
			 *
			 * Note that this allows for the components to go
			 * beyond 1.0, which is okay because they will
			 * be truncated when cast to integers. */
			return color_t(this->red + other.red,
					this->green + other.green,
					this->blue + other.blue);
		};

		/**
		 * Adds another color to this one
		 *
		 * Adds the given color to this one, component-by-component.
		 *
		 * @param other   The other color to add to this one
		 *
		 * @return    Returns the modified color
		 */
		inline color_t& operator += (const color_t& other)
		{
			/* add each component */
			this->red   += other.red;
			this->green += other.green;
			this->blue  += other.blue;

			/* return the modified value */
			return (*this);
		};

		/**
		 * Multiply two colors together
		 *
		 * This multiplication is performed component-by-component
		 *
		 * @param other  The other color to multiply with this one
		 *
		 * @return   Returns the product of the two colors
		 */
		inline color_t operator * (const color_t& other) const
		{
			/* perform component-wise multiplication */
			return color_t(this->red * other.red,
					this->green * other.green,
					this->blue * other.blue);
		};

		/**
		 * Multiples the other color with this one, stores in this
		 * structure.
		 *
		 * Will multiply the two colors, and store the
		 * result in this structure.
		 *
		 * @param other    The other color to multiply with
		 *                 this one
		 *
		 * @return     Returns the modified structure
		 */
		inline color_t& operator *= (const color_t& other)
		{
			/* multiple each component */
			this->red   *= other.red;
			this->green *= other.green;
			this->blue  *= other.blue;

			/* return the result */
			return (*this);
		};

		/**
		 * Scalar multiplication of this color by a constant
		 *
		 * Will mutliply each component of this color by
		 * the specified constant.
		 *
		 * @param c   The constant to use
		 *
		 * @return    Returns the new color
		 */
		inline color_t operator * (float c) const
		{
			/* multiply each component by c */
			return color_t(	this->red   * c,
					this->green * c,
					this->blue  * c);
		};

		/**
		 * In-place scalar multiplication of this color by constant
		 *
		 * Will multiply each component of this color by
		 * the specified constant, and store the result in this
		 * color object.
		 *
		 * @param c   The scalar constant to use
		 *
		 * @return    Returns a reference to the modified color
		 */
		inline color_t& operator *= (float c)
		{
			/* multiply each component */
			this->red   *= c;
			this->green *= c;
			this->blue  *= c;

			/* return the result */
			return (*this);
		};

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Prints this color info to the given stream
		 */
		inline void print(std::ostream& os) const
		{
			os << this->red   << " " 
			   << this->green << " " 
			   << this->blue; 
		};

	/* helper functions */
	private:

		/**
		 * Converts from a float representation of a color component
		 * to an integer representation of a color component:
		 *
		 * f: [0,1.0] -> [0,255]
		 *
		 * @param f   The float to convert
		 *
		 * @return    The integer value of this float
		 */
		inline static int convert_to_int(float f)
		{
			int i;

			/* convert */
			i = (int) round(f*255);

			/* check bounds */
			if(i < 0)	i = 0;
			if(i > 255)	i = 255;

			/* return */
			return i;
		};
};

#endif
