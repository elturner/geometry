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
#include <memory>
#include <fstream>
#include <Eigen/Dense>
#include <io/data/urg/urg_data_reader.h>
#include <io/data/d_imager/d_imager_data_reader.h>
#include <io/data/fss/fss_io.h>
#include <timestamp/sync_xml.h>
#include <geometry/system_path.h>
#include <geometry/transform.h>
#include <image/fisheye/fisheye_camera.h>
#include <image/rectilinear/rectilinear_camera.h>

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
			XYZ_FILE,   				  /* ascii-formatted xyz file */
			OBJ_FILE,   				  /* wavefront OBJ file */
			PTS_FILE,   				  /* ascii-formatted pts file */
			UNKNOWN_FILE 				  /* unknown filetype */
		};

		/**
		 * valid coloring options
		 */
		enum COLOR_METHOD
		{
			NO_COLOR, 					  /* add no color information */
			COLOR_BY_HEIGHT, 			  /* color output by height */
			COLOR_BY_NOISE, 			  /* color output by noise level */
			COLOR_BY_TIME, 				  /* color output by timestamp */
			NEAREST_IMAGE, 				  /* color points based on imagery */
			NEAREST_IMAGE_DROP_UNCOLORED  /* don't export * uncolored points */
		};

		/**
 		 * valid camera options
 		 */
 		enum CAMERA_TYPES
 		{
 			CAMERA_TYPE_FISHEYE,		  /* fisheye camers */
 			CAMERA_TYPE_RECTILINEAR  	  /* rectilienar cameras */
 		};

	/* parameters */
	private:

		/*---------------------------------*/
		/* data acquistion characteristics */
		/*---------------------------------*/
		
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

		/*-----------------------*/
		/* processing parameters */
		/*-----------------------*/

		/**
		 * Range limit.  If non-negative, will throw away far points
		 *
		 * An optional range limit.  If this value is non-negative,
		 * then will throw away any points that are farther away
		 * from their source scanner than this distance.
		 *
		 * Measured in meters.
		 */
		double max_range_limit;

		/**
		 * Camera time buffer.
		 *
		 * This value indicates how many seconds to look into
		 * the past and future for each camera when coloring 
		 * points.  The goal of looking at multiple images from 
		 * each camera is that a point that wasn't seen by any 
		 * cameras may have been by an image from close by,
		 * given that the system is moving in space.
		 *
		 * This value searches in both past and future, in units
		 * of seconds:   [-t, +t]
		 */
		double camera_time_buffer_range; /* how far to search */
		double camera_time_buffer_dt; /* time step between search */

		/*------------------------*/
		/* file export parameters */
		/*------------------------*/
	
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
		 * Specifies the fisheye cameras to use for coloring, if any
		 */
		std::vector<std::shared_ptr<camera_t> > cameras;

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
		 * @param maxrange  Optional range limit.  Negative value
		 *                  uses all points.
		 * @param timebuf_range     Specifies range of timebuf in
		 *                          each direction (units: seconds)
		 * @param timebuf_dt        Specifies step-size of time
		 *                          buffer search (units: seconds)
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int open(const std::string& pcfile,
		         const std::string& pathfile,
		         const std::string& timefile,
		         const std::string& conffile,
			 double u,
		         COLOR_METHOD c,
			 double maxrange,
		         double timebuf_range, double timebuf_dt);

		/**
		 * Adds a camera to this object for use of coloring
		 *
		 * If nearest image coloring method is used, then this
		 * camera will be considered for providing images to
		 * color the points.  Call this function multiple times
		 * to provide multiple cameras.
		 *
		 * Should be called after this object is opened, since
		 * camera initialization requires path to be read in.
		 *
		 * @param metafile   The metadata file for this camera
		 * @param calibfile  The binary fisheye calibration file
		 * @param imgdir     The directory containing camera images
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int add_camera(const std::string& metafile,
		               const std::string& calibfile,
		               const std::string& imgdir,
		               CAMERA_TYPES cameraType);

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
		 * Exports all points from this fss file to output
		 *
		 * Will export all points parsed from the input .fss
		 * file to the output pointcloud file.
		 *
		 * @param filename   The input file to parse
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int export_fss(const std::string& filename);

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
		 * Writes a set of points to specified outfile
		 *
		 * Given a set of points as an Eigen matrix, will write
		 * each point (each column of the matrix) to the output
		 * file in the format specified by the structure.
		 *
		 * @param pts   The matrix specifying the points to write
		 * @param ind   The index of this scan
		 * @param ts    The timestamp for these points
		 * @param noise Noise values for points, Ignored if empty.
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int write_to_file(const Eigen::MatrixXd& pts,
		                  int ind, double ts,
		                  std::vector<double>& noise);
		
		/**
		 * Writes a point to an *.xyz formatted outfile
		 *
		 * Given a point and its info, will write to the output
		 * file in the format of an *.xyz file.
		 *
		 * @param x     The x-coordinate of point to write
		 * @param y     The y-coordinate of point to write
		 * @param z     The z-coordinate of point to write
		 * @param r     The red component of color to write
		 * @param g     The green component of color to write
		 * @param b     The blue component of color to write
		 * @param ind   The index of this scan
		 * @param ts    The timestamp for these points
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int write_to_xyz_file(double x, double y, double z,
		                      int r, int g, int b,
		                      int ind, double ts);
			
		/**
		 * Writes a point to an *.obj formatted outfile
		 *
		 * Given a point and its info, will write to the output
		 * file in the format of an *.obj file.
		 *
		 * @param x     The x-coordinate of point to write
		 * @param y     The y-coordinate of point to write
		 * @param z     The z-coordinate of point to write
		 * @param r     The red component of color to write
		 * @param g     The green component of color to write
		 * @param b     The blue component of color to write
		 * @param ind   The index of this scan
		 * @param ts    The timestamp for these points
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int write_to_obj_file(double x, double y, double z,
		                      int r, int g, int b,
		                      int ind, double ts);
		
		/**
		 * Writes a point to an *.pts formatted outfile
		 *
		 * Given a point and its info, will write to the output
		 * file in the format of an *.pts file.
		 *
		 * @param x     The x-coordinate of point to write
		 * @param y     The y-coordinate of point to write
		 * @param z     The z-coordinate of point to write
		 * @param r     The red component of color to write
		 * @param g     The green component of color to write
		 * @param b     The blue component of color to write
		 * @param ind   The index of this scan
		 * @param ts    The timestamp for these points
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int write_to_pts_file(double x, double y, double z,
		                      int r, int g, int b,
		                      int ind, double ts);

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
		 * @param h     The input height component
		 */
		void height_to_color(int& red, int& green, int& blue,
		                     double h) const;
	
		/**
		 * Generates a color based on a given noise value
		 *
		 * Will generate a color as an (r,g,b) triplet
		 * based on the provided noise (std. dev.) estimate
		 * for the point in question.
		 *
		 * Color components will be in [0,255]
		 *
		 * @param red   The output red component
		 * @param green The output green component
		 * @param blue  The output blue component
		 * @param n     The input noise component
		 */
		void noise_to_color(int& red, int& green, int& blue,
		                    double n) const;

		/**
		 * Generates a color based on a given timestamp
		 *
		 * Will generate a color as an (r,g,b) triplet
		 * based on the provided timestamp.
		 *
		 * Color components will be in [0,255]
		 *
		 * @param red   The output red component
		 * @param green The output green component
		 * @param blue  The output blue component
		 * @param n     The input noise component
		 */
		void time_to_color(int& red, int& green, int& blue,
		                   double n) const;

		/**
		 * Generates a color for given point based on all cameras
		 *
		 * Analyzes coloring from all available cameras to find
		 * the optimal coloring for the specified point.  This 
		 * will be colored based on the nearest temporal image
		 * from each camera available, and the camera with the
		 * best normal vector at that timestamp will be chosen.
		 *
		 * Color components will be [0, 255]
		 *
		 * @param red   The output red component
		 * @param green The output green component
		 * @param blue  The output blue component
		 * @param x     The input world x-coordinate of point
		 * @param y     The input world y-coordinate of point
		 * @param z     The input world z-coordinate of point
		 * @param t     The input timestamp of point
		 * @param quality  The quality of the coloring [0,1]
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int color_from_cameras(int& red, int& green, int& blue,
		                       double x, double y, double z,
		                       double t, double& quality);

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
		 * @param rangelimit   If non-negative, will throw away
		 *                     points farther than this limit
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		static int rectify_urg_scan(Eigen::MatrixXd& mat,
		                          const urg_frame_t& scan,
		                          const std::vector<double>& coses,
                                          const std::vector<double>& sines,
					  double rangelimit);

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
		 * Converts a fss frame to an Eigen matrix structure
		 *
		 * Will take one frame parsed from a fss file, and
		 * populates the specified Eigen matrix with points from
		 * the frame.
		 *
		 * @param mat    The matrix to populate
		 * @param frame  The fss frame to convert
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		static int convert_fss_scan(Eigen::MatrixXd& mat,
		                            const fss::frame_t& frame);

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
