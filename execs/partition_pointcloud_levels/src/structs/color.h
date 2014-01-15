#ifndef COLOR_H
#define COLOR_H

/* color.h:
 *
 *	The color_t class represents a color
 *	via red, green, blue values on the
 *	range of 0 to 255.
 */

class color_t
{
	/*** parameters ***/
	public:
	
	/* color value */
	unsigned char red;
	unsigned char green;
	unsigned char blue;

	/*** functions ***/
	public:

	/* constructors */
	color_t();
	color_t(unsigned char r, unsigned char g, unsigned char b);
	~color_t();

	/* modifiers */

	/* set:
	 *
	 * 	Sets the value of this color.
	 */
	inline void set(unsigned char r, unsigned char g, unsigned char b)
	{
		this->red = r;
		this->green = g;
		this->blue = b;
	};

	/* operators */

	inline color_t operator = (const color_t& other)
	{
		/* copy info */
		this->red = other.red;
		this->green = other.green;
		this->blue = other.blue;

		/* return the new result */
		return (*this);
	};

	inline bool operator == (const color_t& other)
	{
		return ( (this->red == other.red)
				&& (this->green == other.green)
				&& (this->blue == other.blue) );
	};
};

#endif
