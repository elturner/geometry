#ifndef FRAME_MODEL_H
#define FRAME_MODEL_H

/**
 * @file frame_model.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the frame_model_t class, which houses probabilistic
 * information for each scan-point in a frame.
 */

#include <io/data/fss/fss_io.h>
#include <geometry/system_path.h>
#include <geometry/carve/gaussian/scan_model.h>
#include <geometry/carve/gaussian/carve_map.h>
#include <vector>
#include <string>

/**
 * This class holds a list of carve maps for each scan-point in a frame
 */
class frame_model_t
{
	/* parameters */
	private:

		/* a list of carve maps for each point in the frame */
		unsigned int num_points;
		carve_map_t* map_list; /* dynamically allocated list */
		
		/* this bool list indicates which carve maps are valid */
		std::vector<bool> is_valid;

	/* functions */
	public:

		/*-------------------------------*/
		/* constructors and initializers */
		/*-------------------------------*/

		/**
		 * Constructs empty frame
		 */
		frame_model_t();

		/**
		 * Frees all memory and resources
		 */
		~frame_model_t();

		/**
		 * Initializes this object from the given frame info
		 *
		 * Given an input frame, will compute the probability
		 * distributions and carving maps for each point in the
		 * frame, and store those results in this object.
		 *
		 * The provided scan_model_t should already be initialized
		 * to the current sensor (call set_sensor() before this
		 * call).  This call will call set_frame() and set_point()
		 * functions appropriately.
		 *
		 * @param frame   The input frame to model
		 * @param model   The model object for the sensor
		 * @param path    The path of the system
		 *
		 * @return     Returns zero on success, non-zero on failure
		 */
		int init(const fss::frame_t& frame,
		         scan_model_t& model, const system_path_t& path);

		/**
		 * Swaps information with the other frame
		 *
		 * Calling this function will swap frame info.  It does
		 * not matter which frame calls the function.  Both of
		 * the following have the same behavior:
		 *
		 * a.swap(b)
		 * b.swap(a)
		 *
		 * for frame_model_t a, b.
		 *
		 * @param other   The other frame to swap with
		 */
		void swap(frame_model_t& other);
		
		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Will export meshes of gaussian models to OBJ file
		 *
		 * Will export a mesh representing the basic shape
		 * of the probability distribution of this point/sensor
		 * pair to a wavefront OBJ file stream.
		 *
		 * @param fn  The file to write to
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int writeobj(const std::string& fn) const;
};

#endif
