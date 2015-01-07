#ifndef H_COLORMAP_H
#define H_COLORMAP_H

/*
	colormap.h

	This class provides a means for generating colormaps for mapping 
	unsigned char values to RGB sets.
*/

/* includes */
#include <string>

/* colormap class */
class colormap
{

public:

	//
	// This enumeratates the different colormaps supported by this class
	// 
	// These are a direct copy of the MATLAB color maps.  See the MATLAB 
	// documentation for more information
	//
	// The default colormap is always GRAY
	//
	enum COLORMAP
	{
		JET,
		HSV,
		HOT,
		COOL,
		SPRING,
		SUMMER,
		AUTUMN,
		WINTER,
		GRAY,
		BONE,
		COPPER,
		PINK,
		LINES,
		COLORCUBE,
		PRISM,
		FLAG
	};

	//
	// Constructors.  This class can be constructed to default to
	// GRAY, initialized via enumeration, or by string
	//
	colormap();
	colormap(COLORMAP map);
	colormap(const std::string& map);

	//
	// This function sets the current colormap
	//
	// The first version can never fail, but the second version can fail if
	// the given string does not match a supported colormap name
	//
	// Returns 0 on success and 1 on failure
	//
	int set(COLORMAP map);
	int set(const std::string& map);

	//
	// Do the color mapping according to the currently set map
	//
	inline unsigned char map_red(unsigned char v) const
		{return _colormap[v*3];};
	inline unsigned char map_green(unsigned char v) const
		{return _colormap[v*3+1];};
	inline unsigned char map_blue(unsigned char v) const
		{return _colormap[v*3+2];};

private:

	// This is a pointer to the static color map variable
	const unsigned char * _colormap;

	// This is the list of const color map arrays
	// These are stored in RGB ordering
	static const unsigned char _map_jet[256*3];
	static const unsigned char _map_hsv[256*3];
	static const unsigned char _map_hot[256*3];
	static const unsigned char _map_cool[256*3];
	static const unsigned char _map_spring[256*3];
	static const unsigned char _map_summer[256*3];
	static const unsigned char _map_autumn[256*3];
	static const unsigned char _map_winter[256*3];
	static const unsigned char _map_gray[256*3];
	static const unsigned char _map_bone[256*3];
	static const unsigned char _map_copper[256*3];
	static const unsigned char _map_pink[256*3];
	static const unsigned char _map_lines[256*3];
	static const unsigned char _map_colorcube[256*3];
	static const unsigned char _map_prism[256*3];
	static const unsigned char _map_flag[256*3];

};

#endif