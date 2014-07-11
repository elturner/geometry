#include "csv_io.h"
#include "../structs/building_model.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

/**
 * @file   csv_io.cpp
 * @author Eric Turner
 * @brief  Contains functions to export building models to .csv files
 *
 * @section DESCRIPTION
 *
 * This file contains functions used to export building models to
 * .csv files.  These files will contain statistics about the
 * various rooms recovered from the represented floorplan.
 */

using namespace std;
using namespace fp;

/* the following definitions are used in this file */

#define METERS_TO_FEET 3.28084
#define M2_TO_F2 10.7639 /* square meters to square feet */
#define M3_TO_F3 35.3147 /* cubic meters to cubic feet */ 

/* function implementations */

int writecsv(const std::string& filename, const building_model_t& bm)
{
	ofstream outfile;
	vector<edge_t> edges;
	double p, p_sum, a, a_sum, v, v_sum;
	size_t i, j, num_rooms, num_walls;

	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1;

	/* write out header information */
	outfile << "," << endl
		<< ",Floorplan Statistics," << endl
		<< "," << endl
		<< "," << endl /* space for user-written comments */
		<< "," << endl
		<< ",Room ID,,Perimeter (meters),Area (m^2),Volume (m^3),"
		<< ",Perimeter (feet),Area (feet^2),Volume (feet^3),"
		<< ",Comments," << endl;

	/* iterate through rooms */
	p_sum = a_sum = v_sum = 0;
	num_rooms = bm.floorplan.rooms.size();
	for(i = 0; i < num_rooms; i++)
	{
		/* compute room boundary */
		bm.floorplan.compute_edges_for_room(edges, i);
		num_walls = edges.size();
		p = 0;
		for(j = 0; j < num_walls; j++)
			p += bm.floorplan.compute_edge_length(edges[j]);
		p_sum += p;

		/* compute room area */
		a = bm.floorplan.compute_room_area(i);
		a_sum += a;

		/* compute room volume */
		v = a * (bm.floorplan.rooms[i].max_z
			- bm.floorplan.rooms[i].min_z);
		v_sum += v;

		/* export to file */
		outfile << "," << i << ",," << p << "," << a << "," << v
			<< ",," << (METERS_TO_FEET * p)
			<< "," << (M2_TO_F2 * a)
			<< "," << (M3_TO_F3 * v)
			<< "," << endl;
	}

	/* write footer information */
	outfile << "," << endl
		<< ",Total,," << p_sum << "," << a_sum << "," << v_sum
		<< ",," << (METERS_TO_FEET * p_sum)
		<< "," << (M2_TO_F2 * a_sum)
		<< "," << (M3_TO_F3 * v_sum) << "," << endl
		<< "," << endl;

	/* success */
	outfile.close();
	return 0;
}
