#ifndef IMAGE_CACHE_H
#define IMAGE_CACHE_H

/**
 * @file image_cache.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the image_cache_t, which reads imagery
 * from the filesystem for quick access, and will maintain
 * cache size based on the class parameters by freeing memory
 * for the least recently used items.
 *
 * This code links to OpenCV 2.4.7
 */

#include <list>
#include <map>
#include <string>
#include <opencv2/opencv.hpp>

/* the following classes are defined in this file */
class image_cache_t;
class image_cache_element_t;

/**
 * A caching object for loading camera images.
 *
 * The image_cache_t class will provide OpenCV image structures from
 * given image file paths.  These image structures will be loaded into
 * memory as they are called, and stored in memory for quick future
 * access.  Only the N most recent images will be saved in memory at
 * a time, where N is determined by class parameters.
 */
class image_cache_t
{
	/* type defs */
	private:

		/* This is the list, sorted by recency of image use */
		typedef std::list<image_cache_element_t> cache_list_t;
		
		/* iterater into this list */
		typedef cache_list_t::iterator listptr_t;

		/* this is the lookup map type */
		typedef std::map<std::string, listptr_t> cache_map_t;

	/* parameters */
	private:

		/* max allowable cache size */
		unsigned int capacity;

		/* This list represents all images in memory.
		 * They are ordered by how recently they've been used
		 * (with most recently used at front) */
		cache_list_t image_list;

		/* This map references values in the image_list, allowing
		 * for quick look-up of a given image path to the Mat
		 * representation of that image. */
		cache_map_t lookup;

	/* functions */
	public:

		/**
		 * Initializes empty cache with default capacity
		 */
		image_cache_t();

		/**
		 * Frees all memory and resources
		 */
		~image_cache_t();

		/**
		 * Sets the capacity for this image cache
		 *
		 * Will set the maximum number of images that
		 * can be stored in the cache.  If the current
		 * number in memory exceeds this value, then the
		 * cache will be reduced by the appropriate amount.
		 *
		 * @param s    The maximum cache size
		 */
		void set_capacity(unsigned int s);

		/**
		 * Clears all contents from the cache
		 *
		 * Will free all images from the cache.
		 */
		void clear();

		/**
		 * Retrieves the image at the specified path
		 *
		 * If this image is in the cache, then it will
		 * be stored in the specified Mat. If it is not 
		 * in the cache, then the image will be read from 
		 * disk, stored in the cache, and stored in the 
		 * specified Mat.
		 *
		 * @param path   The path to the image to retrieve
		 * @param m      Where to store the retrieved image
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int get(const std::string& path, cv::Mat& m);

		/**
		 * Prints status info about this cache
		 *
		 * Useful for debugging.
		 */
		void print_status();

	/* private helper functions */
	private:

		/**
		 * Reduces size, if necessary, to maintain capacity limit
		 *
		 * This function will remove the least-recently used values
		 * from the cache until its size meets the current capacity
		 * limit.
		 */
		void enforce_capacity();
};

class image_cache_element_t
{
	/* parameters */
	public:

		/* the file path of this image */
		std::string filepath;

		/* OpenCV Matrix representation of image */
		cv::Mat image;

	/* functions */
	public:

		/**
		 * Creates blank element
		 */
		image_cache_element_t();

		/**
		 * Creates element from specified values
		 */
		image_cache_element_t(const std::string& path, cv::Mat& m);

		/**
		 * Frees all memory and resources
		 */
		~image_cache_element_t();

		/* operators */

		inline bool operator < (const image_cache_element_t& rhs)
		                        const
		{
			return (this->filepath < rhs.filepath);
		};

		inline image_cache_element_t operator = (
		                   const image_cache_element_t& rhs)
		{
			this->filepath = rhs.filepath;
			this->image = rhs.image;
			return (*this);
		};
};

#endif
