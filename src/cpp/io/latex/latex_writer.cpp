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
#include <sstream>
#include <string>
#include <set>

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
	this->fp_counter = 0;
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
		<< "\\usepackage{tikz}\n"
		<< "\n"
		<< "\% This file was auto-generated with the c++ code\n"
		<< "\% written by Eric Turner "
		<< "<elturner@eecs.berkeley.edu>\n"
		<< "\n"
		<< "\\begin{document}\n"
		<< "\n"
		<< "\\title{Indoor Modeling Dataset $" 
		<< latex_writer_t::sanitize(name) << "$}\n"
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
	double a, min_x, min_y, max_x, max_y, scale;
	vector<fp::edge_t> edges;
	vector<fp::edge_t>::iterator eit;

	/* compute stats */
	fp.compute_bounds(min_x, min_y, max_x, max_y);
	a = fp.compute_total_area();
	this->fp_counter++;
	scale = 4.25 / max(max(-min_x, -min_y), max(max_x, max_y));

	/* print results */
	this->outfile << "\\section*{Floor " << (this->fp_counter) 
	              << " Info}\n\n"
	              << "\\paragraph*{} Number of rooms: " 
		      << fp.rooms.size() << "\n\n"
		      << "\\paragraph*{} Area: " 
		      << a << " square meters (" << (10.7639*a)
		      << " square feet)\n\n  \\\n  \\\n\n";

	/* export vectorized image */
	fp.compute_edges(edges);
	this->outfile << "\\begin{tikzpicture}[scale=" << scale << "]\n"
	              << "\\draw[ultra thick]";
	for(eit = edges.begin(); eit != edges.end(); eit++)
	{
		/* export position of this edge */
		this->outfile << "\n(" << fp.verts[eit->verts[0]].x << ","
			      << fp.verts[eit->verts[0]].y << ")"
			      << " -- "
			      << "(" << fp.verts[eit->verts[1]].x << ","
			      << fp.verts[eit->verts[1]].y << ")";
	}
	this->outfile << ";\n\\end{tikzpicture}\n\n";
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
	this->fp_counter = 0;
}

/*------------------*/
/* helper functions */
/*------------------*/

string latex_writer_t::sanitize(const string& s)
{
	stringstream out;
	size_t i, n;

	/* iterate over input string */
	n = s.size();
	for(i = 0; i < n; i++)
	{
		/* check for special characters */
		switch(s[i])
		{
			case ' ':
				out << "/,"; /* specifies a space */
				break;
			case '_':
				out << "\\_"; /* specifies underscore */
				break;
			case '^':
				out << "\\^"; /* specifies carat */
				break;
			default:
				/* just write the character as-is */
				out << s[i];
				break;
		}
	}

	/* return the sanitized string */
	return out.str();
}
