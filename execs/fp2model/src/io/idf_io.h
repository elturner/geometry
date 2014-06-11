#ifndef IDF_IO_H
#define IDF_IO_H

/**
 * @file idf_io.h:
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Will export building model objects to IDF format for EnergyPlus
 *
 * @section DESCRIPTION
 *
 * This file represents functions that will
 * generate Input Data Files (IDF) for the EnergyPlus
 * simulation framework.
 *
 * The primary consumer of these functions is
 * EnergyPlus, a building energy simulation tool.
 */

#include "../structs/building_model.h"
#include <fstream>
#include <string>

/**
 * Exports building model objects to .idf files
 *
 * Will write a .idf file, which is the
 * Input Data File for EnergyPlus.
 *
 * This assumes default building materials,
 * and does not include doors or furniture.
 *
 * @param filename      Where to write the .idf files
 * @param bm            The building model to export
 * @param verbose       If true, will write extra information to file
 *
 * @return              Returns zero on success, non-zero on failure.
 */
int writeidf(const std::string& filename, const building_model_t& bm,
             bool verbose=false);

/*---------------------------------------------------------*/
/*----------------- HELPER FUNCTIONS ----------------------*/
/*---------------------------------------------------------*/

/**
 * Will export a single room to IDF file stream
 *
 * Will export room #ri of the specified building model to the
 * given filestream.
 *
 * @param outfile    Where to export the data
 * @param bm         The building model to export
 * @param r          The room information to export
 */
void writeroom(std::ofstream& outfile, const building_model_t& bm,
               const fp::room_t& r);

/**
 * Writes the floor and ceiling geometry information for the given room
 *
 * Given a building model and a particular room, will export
 * the floor and ceiling geometry of that room.  This may require exporting
 * multiple surfaces, in order to comply to the max surface vertex
 * count for IDF files.
 *
 * @param outfile     Where to export the data
 * @param bm          The building model to export
 * @param r           The room info
 * @param zonename    The name of the zone to associate these surfaces to
 * @param surfnum     The number of this floor/ceiling pair in this room
 */
void writefloorandceiling(std::ofstream& outfile,const building_model_t& bm,
                         const fp::room_t& r, const std::string& zonename,
                         unsigned int surfnum=1);

/**
 * Writes the wall geometry info for a room
 *
 * Will write the set of walls that belong to the given room.  These
 * walls will have default material properties, and will not contain
 * any inter-zone links.
 *
 * @param outfile      Where to export the data
 * @param bm           The building model to export
 * @param r            The room to analyze
 * @param zonename     The name of the zone to associate surfaces to
 */
void writewalls(std::ofstream& outfile, const building_model_t& bm,
               const fp::room_t& r, const std::string& zonename);

/**
 * Writes wall geometry info to IDF file
 *
 * The geometry given to this function can either be a full wall
 * or a subwall, but must be a vertically-aligned rectangle.  If
 * a wall has a window, then the geometry of the wall will be
 * broken up into subwalls.
 *
 * @param outfile       Where to export the data
 * @param x1            X-coordinate of first vertex
 * @param y1            Y-coordinate of first vertex
 * @param x2            X-coordinate of second vertex
 * @param y2            Y-coordinate of second vertex
 * @param min_z         Elevation of bottom edge
 * @param max_z         Elevation of top edge
 * @param name          The name of this subwall
 * @param zonename      The name of the zone that contains this wall
 */
void writesubwall(std::ofstream& outfile, double x1, double y1,
		double x2, double y2, double min_z, double max_z,
		const std::string& name, const std::string& zonename);

/**
 * Writes window geometry info to IDF file
 *
 * Will write the specified window as a vertically-oriented
 * rectangle with the specified geometry.  Windows use a different
 * construction than other surfaces in the building.
 *
 * @param outfile       Where to export the data
 * @param x1            X-coordinate of first vertex
 * @param y1            Y-coordinate of first vertex
 * @param x2            X-coordinate of second vertex
 * @param y2            Y-coordinate of second vertex
 * @param min_z         Elevation of bottom edge
 * @param max_z         Elevation of top edge
 * @param num           Index number of this window
 * @param wallname      The name of the wall that contains this window
 */
void writewindow(std::ofstream& outfile, double x1, double y1,
		double x2, double y2, double min_z, double max_z,
		unsigned int num, const std::string& wallname);

/*-----------------------------------------------------*/
/*---- NON-GEOMETRY HELPER FUNCTIONS FOR IDF FILES ----*/
/*-----------------------------------------------------*/

/**
 * Exports version information to IDF file
 */
void writeversion(std::ofstream& outfile);

/**
 * Exports building-level information to IDF file
 *
 * @param outfile   Where to export the info
 * @param name      The name of this building
 */
void writebuilding(std::ofstream& outfile, const std::string& name);

/**
 * Writes timestep field of IDF file
 */
void writetimestep(std::ofstream& outfile);

/**
 * Writes simulation control values to IDF file
 */
void writesimulationcontrol(std::ofstream& outfile);

/**
 * Writes a default location to set this building.
 *
 * Currently, it will specify the building is located in Chicago, Il
 */
void writelocation(std::ofstream& outfile);

/**
 * Writes a list of common material properties to use in this file
 */
void writecommonmats(std::ofstream& outfile);

/**
 * Writes a list of common constructions to use for different surfaces
 *
 * These constructions are used for different interior or exterior surfaces
 * in the IDF file.  They are formed from the materials specified in 
 * the file.
 */
void writecommonconstructions(std::ofstream& outfile);

/**
 * Writes an example simple building schedule to the IDF file
 *
 * This schedule includes the building's energy load during weekdays
 * and holidays.
 */
void writedefaultschedule(std::ofstream& outfile);

/**
 * Writes dictionary info to the file
 */
void writedictionary(std::ofstream& outfile);

/**
 * Writes output variable names to the IDF file
 */
void writefooter(std::ofstream& outfile);

/**
 * Will write comments in the IDF file indicating a new section
 *
 * @param outfile   The file to write to
 * @param text      The name of the section
 */
void writesection(std::ofstream& outfile, const std::string& text);

void writemat(std::ofstream& outfile, const std::string& name, 
		const string& roughness, 
		double thickness, double conductivity, double density,
		double specific_heat, double thermal_absorptance,
		double solar_absorptance, double visible_absorptance);
void writeairgap(std::ofstream& outfile, const std::string& name, 
		double thermal_resistance);
void writewindowglazing(std::ofstream& outfile, const std::string& name,
		const std::string& optical_data_type,
		const std::string& dataset_name,
		double thickness, double solar_transmittance,
		double front_solar_reflectance, 
		double back_solar_reflectance,
		double visible_transmittance,
		double front_visible_reflectance,
		double back_visible_reflectance,
		double infrared_transmittance,
		double front_infrared_emissivity,
		double back_infrared_emissivity,
		double conductivity);
void writewindowgas(std::ofstream& outfile, const std::string& name, 
		const std::string& type, double thickness);
void writeconstruction(std::ofstream& outfile, const std::string& name, 
		const std::string* layers, int num_layers);
void writescheduletypelimit(std::ofstream& outfile, const std::string& name,
		double lower, double upper, const std::string& type);
void writeholiday(std::ofstream& outfile, const std::string& name, 
		const std::string& start, int duration,
		const std::string& type);
void writethermostat(std::ofstream& outfile, const std::string& name, 
		double heating_set, double cooling_set);

#endif
