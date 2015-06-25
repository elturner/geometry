#ifndef WRL_IO_H
#define WRL_IO_H

/**
 * @file     wrl_io.h
 * @author   Eric Turner <elturner@indoorreality.com>
 * @brief    Exports building models to VRML (.wrl) files
 *
 * @section DESCRIPTION
 *
 * This file contains basic functionality to export building
 * models to Virtual Reality Markup Language (VRML) world
 * files (.wrl).
 *
 * File created June 25, 2015
 */

#include "../structs/building_model.h"
#include <iostream>
#include <string>

/**
 * The wrl_io namespace contains functions used to export to
 * .wrl files
 */
namespace wrl_io
{
	/**
	 * Exports this building model to the specified VMRL file.
	 *
	 * @param filename   Path to the .wrl file to write
	 * @param bm         The building model to export
	 *
	 * @return  Returns zero on success, non-zero on failure.
	 */
	int export_wrl(const std::string& filename,
			const building_model_t& bm);

	/*------------------*/
	/* helper functions */
	/*------------------*/

	/* write_floor_to_wrl:
	 *
	 * 	Exports floor as a wrl shape.
	 */
	void write_floor_to_wrl(std::ostream& outfile,
			const building_model_t& bm);
		
	/* write_ceiling_to_wrl:
	 *
	 * 	Exports ceiling as a wrl shape.
	 */
	void write_ceiling_to_wrl(std::ostream& outfile,
			const building_model_t& bm);
	
	/* write_wall_to_wrl:
	 *
	 * 	Exports walls as a wrl shape.
	 */
	void write_wall_to_wrl(std::ostream& outfile,
			const building_model_t& bm);
}

#endif
