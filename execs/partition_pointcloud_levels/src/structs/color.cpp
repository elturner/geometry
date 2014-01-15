#include "color.h"

color_t::color_t()
{
	this->red = 255;
	this->green = 255;
	this->blue = 255;
}

color_t::color_t(unsigned char r, unsigned char g, unsigned char b)
{
	this->red = r;
	this->green = g;
	this->blue = b;
}

color_t::~color_t()
{
}
