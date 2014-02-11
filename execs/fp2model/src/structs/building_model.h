#ifndef BUILDING_MODEL_H
#define BUILDING_MODEL_H

/* building_model.h:
 *
 * 	This file contains definitions for a holistic
 * 	building model, which includes building geometry
 * 	along with semantic labeling of building elements,
 * 	such as windows.
 */

#include "floorplan.h"
#include "window.h"
#include <fstream>

/* the building_model_t class houses all required
 * aspects of a building model, and has functions
 * used to export this model in various formats. */
class building_model_t
{
	/*** parameters ***/
	public:

	/* building elements */
	floorplan_t floorplan;
	windowlist_t windows;

	/*** functions ***/
	public:

	/* constructors */
	building_model_t();
	~building_model_t();

	/* clear:
	 *
	 * 	Clears all information from this model.
	 */
	void clear();

	/* i/o */

	/* import_floorplan:
	 *
	 * 	Will read the specified file as an input floorplan.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int import_floorplan(char* filename);

	/* import_windows:
	 *
	 * 	Will read the specified file as an input window list.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int import_windows(char* filename);

	/* export_obj:
	 *
	 * 	Exports this building model to the specified
	 * 	Wavefront OBJ file.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int export_obj(char* filename);
	 
	/* export_wrl:
	 *
	 * 	Exports this building model to the specified
	 * 	VMRL file.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int export_wrl(char* filename);

	/* helper functions */
	private:

	/* write_floor_to_wrl:
	 *
	 * 	Exports floor as a wrl shape.
	 */
	void write_floor_to_wrl(std::ostream& outfile);
	
	/* write_ceiling_to_wrl:
	 *
	 * 	Exports ceiling as a wrl shape.
	 */
	void write_ceiling_to_wrl(std::ostream& outfile);
	
	/* write_wall_to_wrl:
	 *
	 * 	Exports wall #i as a wrl shape.
	 */
	void write_wall_to_wrl(std::ostream& outfile);
};

#endif
