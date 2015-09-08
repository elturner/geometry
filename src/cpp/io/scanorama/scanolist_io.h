#ifndef SCANOLIST_IO_H
#define SCANOLIST_IO_H

/**
 * @file    scanolist_io.h
 * @author  Eric Turner <elturner@indoorreality.com>
 * @brief   Reads and writes scanorama metadata files
 *
 * @section DESCRIPTION
 *
 * When exporting scanoramas as .ptx files, metadata for each
 * scanorama pose is also recorded in a scanorama metadata list
 * file (.scanolist).
 *
 * Created July 8, 2015
 */

#include <iostream>
#include <string>
#include <vector>

/**
 * The scanolist_io namespace contains all classes used for i/o
 */
namespace scanolist_io
{
	/* the following classes are defined in this namespace */
	class scanometa_t;
	class scanolist_t;
	
	/**
	 * The scanometa_t class stores metadata for a single
	 * scanorama pose.
	 */
	class scanometa_t
	{
		/* parameters */
		public:

			/**
			 * The index of this pose
			 */
			size_t index;

			/**
			 * The timestamp of this pose
			 */
			double timestamp;

			/**
			 * The path to the scanorama file
			 */
			std::string filepath;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Constructs default, invalid metadata
			 */
			scanometa_t(): index(0), timestamp(0), filepath("")
			{};

			/**
			 * Constructs metadata from specified parameters
			 *
			 * @param i    The index of this pose
			 * @param t    The timestamp of this pose
			 * @param f    The filepath to scanorama file
			 */
			scanometa_t(size_t i,double t,const std::string& f)
				: index(i), timestamp(t), filepath(f)
			{};

			/**
			 * Constructs by copying from specified scanometa_t
			 * object
			 *
			 * @param other   The object to copy
			 */
			scanometa_t(const scanometa_t& other)
				: index(other.index),
				  timestamp(other.timestamp),
				  filepath(other.filepath)
			{};

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Prints this metadata info to the specified
			 * text stream.
			 *
			 * Will print the data in the format needed
			 * for a .scanolist file to the specified text
			 * stream.  This will generate one new line
			 * in the file stream.
			 *
			 * @param os   The output stream to write to
			 */
			void print(std::ostream& os) const;

			/**
			 * Parses the next metadata line from the
			 * specified input stream
			 *
			 * Will attempt to parse the next line as a
			 * scanometa_t.
			 *
			 * @param is   The input stream to parse
			 *
			 * @return     Returns zero on success, non-zero
			 *             on failure.
			 */
			int parse(std::istream& is);

			/*-----------*/
			/* operators */
			/*-----------*/

			/**
			 * Copies values from specified other scanometa_t
			 *
			 * @param other   The scanometa_t to copy
			 *
			 * @return    Returns copy of modified structure
			 */
			inline scanometa_t& operator = (
					const scanometa_t& other)
			{
				/* copy values */
				this->index     = other.index;
				this->timestamp = other.timestamp;
				this->filepath  = other.filepath;

				/* return the result */
				return *(this);
			};

			/*------------------*/
			/* helper functions */
			/*------------------*/

			/**
			 * Strips the directory information from this->filepath
			 *
			 * Will modify this->filepath to show only the filename,
			 * not the directory information.  This is important so
			 * that the metdata file can be useful across different
			 * machines, and doesn't contain the absolute path to 
			 * some guy's account on a specific computer.
			 */
			void truncate_filepath();
	};

	/**
	 * The scanolist_t class is used to read and write .scanolist
	 * files.
	 *
	 * It contains some header information and a list of metadata
	 * about each scanorama pose.
	 */
	class scanolist_t
	{
		/* parameters */
		private:

			/**
			 * The following is the list of cameras
			 * that were used to color these
			 * scanoramas.
			 */
			std::vector<std::string> camera_names;

			/**
			 * The following are the dimensions of the
			 * exported scanoramas, (num_rows x num_cols).
			 *
			 * Each pixel is represented by a 3D point
			 * and a color within a scanorama.
			 *
			 * The scanoramas are stored in column-major
			 * order.
			 */
			size_t num_rows;
			size_t num_cols;

			/**
			 * The following is the list of scanorama poses
			 * generated for this dataset.
			 */
			std::vector<scanometa_t> scano_poses;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Constructs empty scanolist
			 */
			scanolist_t() : 
				camera_names(), 
				num_rows(0),
				num_cols(0),
				scano_poses()
			{};


			/*----------------*/
			/* initialization */
			/*----------------*/

			/**
			 * Clears all info in this object
			 */
			inline void clear()
			{
				this->camera_names.clear();
				this->num_rows = 0;
				this->num_cols = 0;
				this->scano_poses.clear();
			};

			/**
			 * Sets the dimensions of each scanorama
			 *
			 * @param nrows   The number of rows in each
			 *                scanorama
			 * @param ncols   The number of columns in each
			 *                scanorama
			 */
			inline void set_dims(size_t nrows, size_t ncols)
			{
				this->num_rows = nrows;
				this->num_cols = ncols;
			};

			/**
			 * Adds a camera name
			 *
			 * The camera names indicate which cameras
			 * were used to color the scanoramas
			 * in this list.
			 *
			 * @param name   The name of the camera to
			 *               add.
			 */
			inline void add_camera(const std::string& name)
			{ this->camera_names.push_back(name); };

			/**
			 * Adds metadata about the next scanorama pose
			 *
			 * Will add this metadata to the end of the
			 * list of poses.
			 *
			 * @param p   The pose information to add
			 */
			inline void add(const scanometa_t& p)
			{ 
				this->scano_poses.push_back(p); 
				this->scano_poses.back().truncate_filepath();
			};

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Writes this data to the specified .scanolist
			 * file.
			 *
			 * @param filename   The .scanolist file to write to
			 *
			 * @return    Returns zero on success, non-zero
			 *            on failure.
			 */
			int write(const std::string& filename) const;

			/**
			 * Reads data from specified scanolist file.
			 *
			 * All data stored previously will be destroyed
			 * and replaced by data from given file.
			 *
			 * @param filename  The .scanolist file to read
			 *
			 * @return    Returns zero on success, non-zero
			 *            on failure.
			 */
			int read(const std::string& filename);
	};
	
}

#endif
