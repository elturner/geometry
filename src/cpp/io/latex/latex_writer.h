#ifndef LATEX_WRITER_H
#define LATEX_WRITER_H

/**
 * @file latex_writer.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief This class is used to export LaTeX files describing datasets
 *
 * @section DESCRIPTION
 *
 * This class is used to generate the source files for LaTeX representations
 * of datasets, so that the generated PDF file can be used to describe
 * a dataset at-a-glance, which is useful for determining intuition about
 * a dataset quickly and easily.
 */

#include <config/backpackConfig.h>
#include <geometry/system_path.h>
#include <mesh/floorplan/floorplan.h>
#include <fstream>
#include <string>

/**
 * The latex_writer_t class represents the output stream for a .tex file
 */
class latex_writer_t
{
	/* parameters */
	private:

		/* the output stream for this file */
		std::ofstream outfile;

	/* functions */
	public:

		/**
		 * Create default writer
		 */
		latex_writer_t();

		/**
		 * Frees all memory and resources 
		 */
		~latex_writer_t();

		/**
		 * Opens file for writing.
		 *
		 * If this stream was already opened, then the original
		 * file stream will be properly closed and a new one
		 * will be opened to the referenced file.
		 *
		 * This also writes header information to the file.
		 *
		 * @param filename  Where to write the file
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int open(const std::string& filename);

		/**
		 * Writes stats about the given config file to output
		 *
		 * @param conf   The hardware config information to write
		 */
		void write_conf_info(backpackConfig& conf);

		/**
		 * Writes stats about the given path to the output
		 *
		 * @param path   The system path to analyze and write
		 */
		void write_path_info(const system_path_t& path);

		/**
		 * Writes stats about the given floorplan to output
		 *
		 * @param fp     The floorplan to analyze and write
		 */
		void write_floorplan_info(const fp::floorplan_t& fp);

		/**
		 * Closes the filestream if open.
		 *
		 * Will close the file stream if currently open, which 
		 * includes writing tail information to the file.  If the
		 * stream is not open, then no action is performed.
		 */
		void close();
};

#endif
