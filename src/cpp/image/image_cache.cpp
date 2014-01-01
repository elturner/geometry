#include "image_cache.h"
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

/* The following defines are used by this class */
#define DEFAULT_IMAGE_CACHE_CAPACITY 5

/* function implementations */

image_cache_t::image_cache_t()
{
	/* start with default values */
	this->capacity = DEFAULT_IMAGE_CACHE_CAPACITY;
	this->image_list.clear();
	this->lookup.clear();
}

image_cache_t::~image_cache_t()
{
	/* clear the structure */
	this->clear();
}

void image_cache_t::set_capacity(unsigned int s)
{
	/* reset capacity value */
	this->capacity = s;

	/* make sure we are not over capacity */
	this->enforce_capacity();
}
		
void image_cache_t::clear()
{
	/* clear all structures in this object */
	this->image_list.clear();
	this->lookup.clear();
}

int image_cache_t::get(const string& path, Mat& m)
{
	cache_map_t::iterator it;

	/* check if this image is in the cache */
	it = this->lookup.find(path);
	if(it == this->lookup.end())
	{
		/* not in cache, retrieve from filesystem */ 
		m = imread(path, CV_LOAD_IMAGE_COLOR);
		if(m.data == NULL)
			return -1; /* could not load from file */

		/* store in cache as the most recent item */
		this->image_list.push_front(image_cache_element_t(path, m));

		/* store reference in lookup map */
		this->lookup.insert(make_pair<string, listptr_t>(
		                    path, this->image_list.begin()));

		/* ensure that the cache is not over-capacity */
		this->enforce_capacity();
	}
	else
	{
		/* shallow copy image to given Mat */
		m = it->second->image;
		
		/* set this image to be the most recent item in cache */
		if(it->second != this->image_list.begin())
		{
			/* not already first in list, erase and reinsert */
			this->image_list.push_front(*(it->second));
			this->image_list.erase(it->second);
			it->second = this->image_list.begin();
		}
	}
	
	/* success */
	return 0;
}
		
void image_cache_t::print_status()
{
	listptr_t lit;

	/* print out capacity information */
	cout << "cache size: " << this->image_list.size() << " / "
	     << this->capacity << " :" << endl;
	
	/* list the files in cache, in order */
	for(lit = this->image_list.begin();
			lit != this->image_list.end(); lit++)
		cout << "\t" << lit->filepath << endl;
	cout << endl;
}

void image_cache_t::enforce_capacity()
{
	listptr_t lit;

	/* check if we are over capacity */
	if(this->image_list.size() <= this->capacity)
		return; /* do nothing */

	/* get all positions from image list that are to be removed */
	lit = this->image_list.begin();
	advance(lit, this->capacity);
	while(lit != this->image_list.end())
	{
		/* remove value from look up map */
		this->lookup.erase(lit->filepath);

		/* erase from image list */
		lit = this->image_list.erase(lit);
	}
}
	
image_cache_element_t::image_cache_element_t()
{
	this->filepath = "";
}

image_cache_element_t::image_cache_element_t(const string& path, Mat& m)
{
	this->filepath = path;
	this->image = m;
}

image_cache_element_t::~image_cache_element_t()
{
	/* destructors are auto called */ 
}
