#ifndef POINTCLOUD_WRITER_H
#define POINTCLOUD_WRITER_H

/**
 * @file pointcloud_writer.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the pointcloud_writer_t class,
 * which is used to convert 3D scans into world
 * coordinates and export them in common pointcloud
 * file formats.
 */

#include <string>
#include <vector>
#include <fstream>
#include <Eigen/Dense>
#include <io/data/urg/urg_data_reader.h>
#include <io/data/d_imager/d_imager_data_reader.h>
#include <timestamp/sync_xml.h>
#include <geometry/system_path.h>
#include <geometry/transform.h>

/* the pointcloud writer class */
class pointcloud_writer_t
{
	/* states */
	public:

		/**
		 * valid filetypes for export
		 */
		enum FILE_TYPE
		{
			XYZ_FILE,   /* ascii-formatted xyz file */
			OBJ_FILE,   /* wavefront OBJ file */
			UNKNOWN_FILE /* unknown filetype */
		};

		/**
		 * valid coloring options
		 */
		enum COLOR_METHOD
		{
			NO_COLOR, /* add no color information to output */
			COLOR_BY_HEIGHT, /* color output by height */
			NEAREST_IMAGE /* color points based on imagery */
		};

	/* parameters */
	private:

		/* data acquistion characteristics */
		
		/**
		 * The path of the system over time
		 *
		 * The system path indicates the trajectory
		 * of the system during the data acquisition.
		 */
		system_path_t path;
	
		/**
		 * The timestamp synchronization of the system
		 *
		 * These values are used to synchronize the 
		 * 3D scanner times to the system clock.
		 */
		SyncXml time_sync;

		/* file export parameters */
	
		/**
		 * The output file stream.
		 */
		std::ofstream outfile;

		/**
		 * The format of the output file
		 */
		FILE_TYPE outfile_format;

		/**
		 * Specifies what coloring technique is used.
		 */
		COLOR_METHOD coloring;

		/**
		 * Specifies the units to use in output file.
		 *
		 * Value represents the conversion from meters to
		 * desired units.  1000 would specify millimeters.
		 * 3.28084 specifies feet.  Defaults to meters (1)
		 */
		double units;

	/* functions */
	public:

		/* constructors */

		/**
		 * Default constructor initializes empty object.
		 */
		pointcloud_writer_t();

		/**
		 * Frees all memory and resources.
		 */
		~pointcloud_writer_t();

		/* i/o */

		/**
		 * Prepares this writer to export to file
		 *
		 * Will open the destination pointcloud file, and
		 * will parse the input files given.
		 *
		 * @param pcfile    The output pointcloud file to write to
		 * @param pathfile  The input path file to use when writing
		 * @param timefile  The input time sync file to use
		 * @param conffile  The input hardware xml config file
		 * @param u         The units to use for output
		 * @param c         The coloring method to use
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int open(const std::string& pcfile,
		         const std::string& pathfile,
		         const std::string& timefile,
		         const std::string& conffile,
			 double u,
		         COLOR_METHOD c);

		/**
		 * Exports all points from this laser scanner to file
		 *
		 * Will export all points recorded in the specified
		 * data file from the given laser scanner to the output
		 * pointcloud file.
		 *
		 * @param name      The name of this laser scanner
		 * @param datfile   The input laser scan file to convert
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int export_urg(const std::string& name, 
		               const std::string& datfile);
		
		/**
		 * Exports all points from this tof scanner to file
		 *
		 * Will export all points recorded in the specified
		 * data file from the given tof scanner to the output
		 * pointcloud file.
		 *
		 * @param name      The name of this tof scanner
		 * @param datfile   The input d-imager scan file to convert
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int export_tof(const std::string& name, 
		               const std::string& datfile);

		/**
		 * Closes all open files
		 *
		 * Closes all input and output file streams in use
		 * by this structure.
		 */
		void close();

	/* helper functions */
	private:

		/**
		 * Writes a set of points to an *.xyz formatted outfile
		 *
		 * Given a set of points as an Eigen matrix, will write
		 * each point (each column of the matrix) to the output
		 * file in the format of an *.xyz file.
		 *
		 * @param pts   The matrix specifying the points to write
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int write_to_xyz_file(const Eigen::MatrixXd& pts);

		/**
		 * Writes a set of points to an *.obj formatted outfile
		 *
		 * Given a set of points as an Eigen matrix, will write
		 * each point (each column of the matrix) to the output
		 * file in the format of an *.obj file.
		 *
		 * @param pts   The matrix specifying the points to write
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int write_to_obj_file(const Eigen::MatrixXd& pts);
		
		/**
		 * Generates a color based on the given height
		 *
		 * Will generated a color as an (r,g,b) triplet
		 * based on the provided elevation value.
		 *
		 * Color components will be in [0,255]
		 *
		 * @param red   The output red component
		 * @param green The output green component
		 * @param blue  The output blue component
		 * @param h     THe input height component
		 */
		void height_to_color(int& red, int& green, int& blue,
		                     double h) const;
		
		/**
		 * Rectifies the input 2D laser scan and converts to matrix
		 *
		 * Given a single scan frame from an urg scanner, will
		 * compute the x,y,z positions of the scan in sensor
		 * coordinates, and store it in an Eigen matrix.
		 *
		 * The rectified points will be in units of meters.
		 *
		 * @param mat    Where to store the computed points
		 * @param scan   The input scan frame
		 * @param coses  The list of cosines for polar->rect calc
		 * @param sines   The list of sins for polar->rect calc
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		static int rectify_urg_scan(Eigen::MatrixXd& mat,
		                          const urg_frame_t& scan,
		                          const std::vector<double>& coses,
                                          const std::vector<double>& sines);

		/**
		 * Converts a d-imager scan to Eigen matrix structure
		 *
		 * Will take one frame of a d-imager scan, and populates
		 * the specified Eigen matrix with the points from the
		 * scan.
		 *
		 * @param mat    The matrix to populate with points
		 * @param frame  The d-imager frame to convert
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		static int convert_d_imager_scan(Eigen::MatrixXd& mat,
		                        const d_imager_frame_t& frame);

		/**
		 * Determine file type given file path
		 *
		 * Will parse the given file path to determine what
		 * type of file is referenced.
		 *
		 * @param file   The file path to parse
		 */
		static FILE_TYPE get_file_type(const std::string& file);
};

#endif
