#include "latex_writer.h"
#include <config/backpackConfig.h>
#include <config/cameraProp.h>
#include <config/laserProp.h>
#include <geometry/system_path.h>
#include <mesh/floorplan/floorplan.h>
#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>

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

using namespace std;

/* function implementations */

latex_writer_t::latex_writer_t()
{
	/* do nothing here */
}
		
latex_writer_t::~latex_writer_t()
{
	/* close any open streams */
	this->close();
}
		
int latex_writer_t::open(const std::string& filename)
{
	size_t slash, dot;
	string name;

	/* close any open streams */
	this->close();

	/* attempt to open file for writing */
	this->outfile.open(filename.c_str());
	if(!(this->outfile.is_open()))
		return -1; /* could not open file */

	/* parse the name of the dataset from the filename */
	slash = filename.find_last_of("\\/");
	if(slash == string::npos)
		slash = 0;
	else
		slash++;
	dot = filename.find_last_of(".");
	if(dot == string::npos)
		dot = filename.size();
	name = filename.substr(slash, dot-slash);

	/* write header information to file */
	this->outfile
		<< "\\documentclass[10pt,onecolumn,letterpaper]{article}\n"
		<< "\n"
		<< "\\usepackage{graphicx}\n"
		<< "\n"
		<< "\% This file was auto-generated with the c++ code\n"
		<< "\% written by Eric Turner "
		<< "<elturner@eecs.berkeley.edu>\n"
		<< "\n"
		<< "\\begin{document}\n"
		<< "\n"
		<< "\\title{Indoor Modeling Dataset " << name << "}\n"
		<< "\\author{UC Berkeley VIP Lab}\n"
		<< "\\maketitle\n"
		<< "\n";
	
	/* success */
	return 0;
}
		
void latex_writer_t::write_conf_info(backpackConfig& conf)
{
	vector<laserProp> lasers;
	vector<cameraProp> cameras;

	/* get sensor properties */
	conf.get_props(lasers, true);
	conf.get_props(cameras, true);

	/* print info */
	this->outfile << "\\section*{Hardware Used}\n\n"
	              << "\\paragraph*{} Number of lasers used: " 
		      << lasers.size() << "\n\n"
		      << "\\paragraph*{} Number of cameras used: "
		      << cameras.size() << "\n\n";
}
		
void latex_writer_t::write_path_info(const system_path_t& path)
{
	double t, s, d;
	int m;

	/* compute stats */
	t = (path.endtime() - path.starttime());
	m = (int) floor(t / 60); /* number of minutes */
	s = t - 60*m;
	d = path.total_distance();

	/* print */
	this->outfile << "\\section*{Path}\n\n"
	              << "\\paragraph*{} Runtime: " 
		      << m << ":" << (s < 10 ? "0" : "")
		      << s << " (" << t << " seconds)\n\n"
		      << "\\paragraph*{} Distance walked: " 
		      << d << " meters (" << (d*3.28084) << " feet)\n\n";
}
		
void latex_writer_t::write_floorplan_info(const fp::floorplan_t& fp)
{
	double a;

	/* compute stats */
	a = fp.compute_total_area();

	/* print results */
	this->outfile << "\\section*{Floor Info}\n\n"
	              << "\\paragraph*{} Number of rooms: " 
		      << fp.rooms.size() << "\n\n"
		      << "\\paragraph*{} Area: " 
		      << a << " square meters (" << (10.7639*a)
		      << " square feet)\n\n";
}
		
void latex_writer_t::close()
{
	/* check if stream is open */
	if(!(this->outfile.is_open()))
		return; /* do nothing here */

	/* write tail information to file */
	this->outfile << "\n\\end{document}\n";
	
	/* close the stream */
	this->outfile.close();
}
