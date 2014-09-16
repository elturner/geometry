#include "window.h"
#include <mesh/floorplan/floorplan.h>
#include <util/error_codes.h>
#include <map>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>

using namespace std;
using namespace fp;

/***** WINDOW LIST FUNCTIONS ******/

windowlist_t::windowlist_t()
{
	this->clear();
}

windowlist_t::~windowlist_t()
{
	this->clear();
}

void windowlist_t::clear()
{
	this->windows.clear();
}

int windowlist_t::import_from_file(const string& filename)
{
	ifstream infile;
	string buf;
	window_t w;
	int ret;

	/* open file for reading */
	infile.open(filename.c_str());
	if(!(infile.is_open()))
		return -1;

	/* iterate through file */
	while(!infile.eof())
	{
		/* get the next line in the file */
		getline(infile, buf);

		/* check for empty line */
		if(buf.empty())
			continue;

		/* parse it as a window */
		ret = w.parse(buf);
		if(ret)
		{
			infile.close();
			return PROPEGATE_ERROR(-2, ret);
		}

		/* add window to list */
		this->add(w);
	}

	/* clean up */
	infile.close();
	return 0;
}

void windowlist_t::add(const window_t& w)
{
	map<edge_t, vector<window_t> >::iterator it;

	/* check if this wall is in the map */
	it = this->windows.find(w.wall);
	if(it == this->windows.end())
	{
		/* add a new list for this wall */
		it = this->windows.insert(pair<edge_t, 
					vector<window_t> >(w.wall, 
					vector<window_t>())).first;
	}

	/* add this window to the list */
	it->second.push_back(w);
}
	
void windowlist_t::get_windows_for(const edge_t& wall,
                                   vector<window_t>& wins) const
{
	map<edge_t, vector<window_t> >::const_iterator it;
	vector<window_t>::const_iterator wit;

	/* find wall in map */
	it = this->windows.find(wall);
	if(it == this->windows.end())
		return; /* do nothing */

	/* iterate through wall's windows, add to list */
	for(wit = it->second.begin(); wit != it->second.end(); wit++)
		wins.push_back(*wit);
}
	
int windowlist_t::export_to_obj(const string& filename,
                               const floorplan_t& fp)
{
	map<edge_t, vector<window_t> >::iterator it;
	vector<window_t>::iterator wit;
	ofstream outfile;
	int num_verts_written, i, j;
	double wx0, wy0, wz0, wxf, wyf, wzf, lx, ly, hi, hj;

	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1;

	/* iterate through windows */
	num_verts_written = 0;
	for(it = this->windows.begin(); it != this->windows.end(); it++)
	{
		/* get wall info */
		i = it->first.verts[0];
		j = it->first.verts[1];
		lx = fp.verts[j].x - fp.verts[i].x;
		ly = fp.verts[j].y - fp.verts[i].y;
		hi = (fp.verts[j].max_z - fp.verts[j].min_z);
		hj = (fp.verts[i].max_z - fp.verts[i].min_z);

		/* iterate over windows on this wall */
		for(wit=it->second.begin(); wit!=it->second.end(); wit++)
		{
			/* print this window to file */
			wx0 = (fp.verts[i].x + wit->min_h*lx) - 0.001*ly;
			wy0 = (fp.verts[i].y + wit->min_h*ly) + 0.001*lx;
			wz0 = (fp.verts[i].min_z * (1 - wit->min_h)
					+ fp.verts[j].min_z * wit->min_h)
				+ (wit->min_v)*(hi*(1 - wit->min_h)
					+ hj*wit->min_h);
			wxf = (fp.verts[i].x + wit->max_h*lx) - 0.001*ly;
			wyf = (fp.verts[i].y + wit->max_h*ly) + 0.001*lx;
			wzf = (fp.verts[i].min_z * (1 - wit->max_h)
					+ fp.verts[j].min_z * wit->max_h)
				+ (wit->max_v)*(hi*(1 - wit->max_h)
					+ hj*wit->max_h);
			
			outfile << "v"
			        << " " << wx0
				<< " " << wy0
				<< " " << wz0
				<< " 255 0 0" << endl;
			outfile << "v"
			        << " " << wxf
				<< " " << wyf
				<< " " << wz0
				<< " 255 0 0" << endl;
			outfile << "v"
			        << " " << wxf
				<< " " << wyf
				<< " " << wzf
				<< " 255 0 0" << endl;
			outfile << "v"
			        << " " << wx0
				<< " " << wy0
				<< " " << wzf
				<< " 255 0 0" << endl;
		
			/* print faces for window */
			outfile << "f " << (num_verts_written+1)
			        <<  " " << (num_verts_written+4)
				<<  " " << (num_verts_written+3)
				<< endl;
			outfile << "f " << (num_verts_written+1)
			        <<  " " << (num_verts_written+3)
				<<  " " << (num_verts_written+2)
				<< endl;
			num_verts_written += 4;
		}
	}

	/* clean up */
	outfile.close();
	return 0;
}

/***** WINDOW FUNCTIONS ******/

window_t::window_t()
{
	edge_t e; /* default edge */

	/* set to invalid */
	this->set(e, 1, 1, 0, 0);
}

window_t::window_t(const edge_t& e)
{
	/* set to invalid window */
	this->set(e, 1, 1, 0, 0);
}

window_t::window_t(const edge_t& e, double mh, double mv, 
                   double Mh, double Mv)
{
	this->set(e, mh, mv, Mh, Mv);
}
	
window_t::window_t(const string& line)
{
	this->parse(line);
}

window_t::~window_t()
{
	/* no computation necessary */
}

int window_t::parse(const string& line)
{
	stringstream ss;

	/* parse the line */
	ss.str(line);
	ss >> this->wall.verts[0] >> this->wall.verts[1]
	   >> this->min_h >> this->min_v >> this->max_h >> this->max_v;

	/* check if valid window */
	if(!(this->valid()))
	{
		/* report error to user */
		cerr << "[window_t::parse]\tInvalid window line: "
		     << line << endl;
		return -1;
	}

	/* success */
	return 0;
}
	
void window_t::get_world_coords(double* wx, double* wy, double* wz, 
				const floorplan_t& fp)
{
	double dx, dy, dz, floor_z, ceil_z;

	/* compute the horizontal displacement of the window */
	dx = fp.verts[this->wall.verts[1]].x 
			- fp.verts[this->wall.verts[0]].x;
	dy = fp.verts[this->wall.verts[1]].y 
			- fp.verts[this->wall.verts[0]].y;

	/* compute horizontal positions */
	wx[0] = wx[1] = this->min_h * dx + fp.verts[this->wall.verts[0]].x;
	wx[2] = wx[3] = this->max_h * dx + fp.verts[this->wall.verts[0]].x;
	wy[0] = wy[1] = this->min_h * dy + fp.verts[this->wall.verts[0]].y;
	wy[2] = wy[3] = this->max_h * dy + fp.verts[this->wall.verts[0]].y;

	/* compute vertical positions so that window is
	 * always perfectly level, even if top and bottom
	 * of wall are not. */
	floor_z = (fp.verts[this->wall.verts[0]].min_z
			< fp.verts[this->wall.verts[1]].min_z)
		? fp.verts[this->wall.verts[1]].min_z
		: fp.verts[this->wall.verts[0]].min_z;
	
	ceil_z = (fp.verts[this->wall.verts[0]].max_z
			> fp.verts[this->wall.verts[1]].max_z)
		? fp.verts[this->wall.verts[1]].max_z
		: fp.verts[this->wall.verts[0]].max_z;
	
	dz = ceil_z - floor_z;

	/* compute corner heights */
	wz[0] = wz[3] = this->min_v * dz + floor_z;
	wz[1] = wz[2] = this->max_v * dz + floor_z;
}
