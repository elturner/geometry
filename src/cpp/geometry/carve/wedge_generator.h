#ifndef WEDGE_GENERATOR_H
#define WEDGE_GENERATOR_H

/**
 * @file wedge_generator.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Computes all wedges and writes them to file
 *
 * @section DESCRIPTION
 *
 * This file contains a class that will perform all probabilistic
 * computations for a set of scanners, in order to compute all
 * wedges from these scanners.  These wedges can then we exported
 * to a binary file.
 */

#include <timestamp/sync_xml.h>
#include <geometry/system_path.h>
#include <string>
#include <vector>

/**
 * The wedge_generator_t class computes all wedges for a set of sensors
 */
class wedge_generator_t
{
	/* parameters */
	private:

		/* The system path */
		system_path_t path;
	
		/* The clock error represents the uncertainty (std. dev.)
		 * of the system clock when timestamping hardware sensors.
		 * It is expressed in units of seconds. This error can
		 * be different for different sensors, and is described
		 * in the time synchronization output file. */
		SyncXml timesync;

		/* if unable to compute the timestamp uncertainty for a
		 * particular sensor, use this value */
		double default_clock_uncertainty;

		/* the carving buffer, in units of standard deviations,
		 * dictates how far past each point will be carved. A
		 * non-zero buffer allows for the exterior-mapped portions
		 * of the probability distributions to be explicity
		 * represented in the octree. */
		double carving_buffer;

		/* the line-fit distance indiates how far away neighbors
		 * can be within a scan to still be considered for line-
		 * fit parameters.  Making this smaller will allow for
		 * smaller lines, but making it larger will allow the
		 * line-fit results to be less noisy and more robust. */
		double linefit_dist;

	/* functions */
	public:
		
		/**
		 * Initializes this object with specified data sources
		 *
		 * Given localization information and processing parameters,
		 * this function call will prepare this object to perform
		 * compute probability distributions for each scan frame.
		 *
		 * @param madfile   The path file for this dataset
		 * @param confile   The xml hardware config file
		 * @param tsfile    The time synchronization output xml file
		 * @param dcu       The default clock uncertainty
		 * @param carvebuf  The carving buffer, units of std. dev.
		 * @param lf_dist   The line-fit distance, units of meters
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int init(const std::string& madfile,
		         const std::string& confile,
		         const std::string& tsfile,
		         double dcu, double carvebuf, double lf_dist);

		/**
		 * Computes and exports all wedges
		 *
		 * Given a list of fss files, will compute all wedge
		 * distributions from the scan frames in these files,
		 * and will export these wedge distributions to disk.
		 *
		 * @param fssfiles     The list of fss files to analyze
		 * @param wedgefile    Where to write the wedge file
		 *
		 * @return    Returns zero on success, non-zero on failure
		 */
		int process(const std::vector<std::string>& fssfiles,
		            const std::string& wedgefile) const;
	
	/* helper functions */
	private:

		/**
		 * Gets the timestamp uncertainty for a specific sensor
		 *
		 * Given the sensor name, will look up the timestamp
		 * uncertainty for that sensor, and return the desired
		 * value.
		 *
		 * @return   Returns the std. dev. of the clock error for
		 *           the specified sensor.
		 */
		double get_clock_uncertainty_for_sensor(
				const std::string& sensor_name) const;
};

#endif
