#ifndef BUILDING_MODEL_H
#define BUILDING_MODEL_H

/**
 * @file building_model.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Defines semantic components of building information models (BIMs)
 *
 * @section DESCRIPTION
 *
 * This file contains definitions for a holistic
 * building model, which includes building geometry
 * along with semantic labeling of building elements,
 * such as windows.
 */

#include <mesh/floorplan/floorplan.h>
#include "window.h"
#include "lights.h"
#include "people.h"
#include "plugloads.h"
#include <iostream>
#include <string>

/**
 * This structure contains BIM details
 *
 * the building_model_t class houses all required
 * aspects of a building model, and has functions
 * used to export this model in various formats.
 */
class building_model_t
{
	/* parameters */
	public:

		/* building elements */

		/**
		 * The floorplan describes the geometric building layout
		 */
		fp::floorplan_t floorplan;

		/**
		 * The name of the level of the building (e.g. "L1").
		 *
		 * If empty, then no name provided.
		 */
		std::string level_name;

		/**
		 * This list describes the location of windows
		 *
		 * The window elements reference the edges (aka walls)
		 * of the floorplan structure.
		 */
		windowlist_t windows;

		/**
		 * This list describes the wattages of lights
		 *
		 * Each room has a wattage value that indicates ceiling
		 * light usage.
		 */
		lights_t lights;

		/**
		 * This list describes the counts for people in each room
		 *
		 * Each room can house a number of people, and these values
		 * dictate our estimate.
		 */
		people_t people;

		/**
		 * This list describes the wattages of plugloads
		 *
		 * Each room has a wattage value that indicates plug-
		 * load usage.
		 */
		plugloads_t plugloads;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Constructs empty building model
		 */
		building_model_t();

		/**
		 * Frees all memory and resources
		 */
		~building_model_t();

		/**
		 * Clears all information from this model.
		 */
		void clear();

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Will read the specified file as an input floorplan.
		 *
		 * @param filename   The location of the .fp file to import
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int import_floorplan(const std::string& filename);

		/**
		 * Will read the specified file as an input window list.
		 *
		 * @param filename   The location of the .windows file
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int import_windows(const std::string& filename);

		/**
		 * Will read the specified file as an input lights list.
		 *
		 * @param filename   The location of the .lights file
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int import_lights(const std::string& filename);

		/**
		 * Will read the specified file as an input people list.
		 *
		 * @param filename   The location of the .people file
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int import_people(const std::string& filename);

		/**
		 * Will read the specified file as an input plugloads list.
		 *
		 * @return filename   The location of the .plugloads file
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int import_plugloads(const std::string& filename);

		/**
		 * Exports this model to the specified Wavefront OBJ file.
		 *
		 * @param filename   Path to the file to write
		 *
		 * @return Returns zero on success, non-zero on failure.
		 */
		int export_obj(const std::string& filename) const;
	 
		/**
		 * Exports this building model to the specified VMRL file.
		 *
		 * @param filename   Path to the .wrl file to write
		 *
		 * @return  Returns zero on success, non-zero on failure.
		 */
		int export_wrl(const std::string& filename) const;

	/* helper functions */
	private:

		/* write_floor_to_wrl:
		 *
		 * 	Exports floor as a wrl shape.
		 */
		void write_floor_to_wrl(std::ostream& outfile) const;
		
		/* write_ceiling_to_wrl:
		 *
		 * 	Exports ceiling as a wrl shape.
		 */
		void write_ceiling_to_wrl(std::ostream& outfile) const;
	
		/* write_wall_to_wrl:
		 *
		 * 	Exports walls as a wrl shape.
		 */
		void write_wall_to_wrl(std::ostream& outfile) const;
};

#endif
