#include "fp_io.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include "../structs/triple.h"
#include "../rooms/tri_rep.h"

using namespace std;

int write_fp(char* filename, tri_rep_t& trirep, double res)
{
	map<triple_t, room_height_t>::iterator rhit;
	map<triple_t, tri_info_t>::iterator tit;
	map<int, set<triple_t> >::iterator vit;	

	vector<set<triple_t> > rooms;
	map<triple_t, int> tri_index_map;
	map<int, int> index_map;
	
	vector<set<triple_t> >::iterator rit;
	map<int, int>::iterator mit_i, mit_j, mit_k;
	set<triple_t>::iterator nit;
	ofstream outfile;
	stringstream ss;
	vertex_t* p;
	int num_verts, num_tris;

	/* get information for rooms */
	trirep.get_rooms(rooms);

	/* open file for writing */
	outfile.open(filename);
	if(!(outfile.is_open()))
		return -1;

	/* create a mapping from the indices of vertices used by
	 * these structures to indices used in the file. */
	num_verts = 0;
	for(vit = trirep.vert_map.begin(); vit != trirep.vert_map.end();
								vit++)
	{
		/* check if this vertex is actually used */
		if(vit->second.empty())
			continue;

		/* get the vertex position */
		p = TRI_VERTEX_POS(&(trirep.tri), vit->first);

		/* add to map */
		index_map.insert(make_pair<int,int>(vit->first, 
						num_verts));
	
		/* print the vertex */
		ss << p->pos[VERTEX_X_IND]
		   <<  " " << p->pos[VERTEX_Y_IND]
		   << endl;

		/* increment count for vertex */
		num_verts++;
	}

	/* print header information */
	outfile << res << endl
	        << num_verts << endl /* number of vertices */
	        << trirep.tris.size() << endl /* number of triangles */
		<< rooms.size() << endl; /* number of rooms */

	/* print the vertices */
	outfile << ss.str();

	/* print the triangles */
	num_tris = 0;
	for(tit = trirep.tris.begin(); tit != trirep.tris.end(); tit++)
	{
		/* get the OBJ indices of the vertices of this triangle */
		mit_i = index_map.find(tit->first.i);
		mit_j = index_map.find(tit->first.j);
		mit_k = index_map.find(tit->first.k);
		if(mit_i == index_map.end() || mit_j == index_map.end()
					|| mit_k == index_map.end())
		{
			outfile.close();
			return -4;
		}

		/* store this triangle's index in the file */
		tri_index_map.insert(make_pair<triple_t, int>(tit->first, 
							num_tris));
		num_tris++;

		/* print floor */
		outfile << mit_i->second
		        <<  " " << mit_j->second
			<<  " " << mit_k->second
			<< endl;
	}

	/* print the rooms */
	for(rit = rooms.begin(); rit != rooms.end(); rit++)
	{
		/* get the root for this room */
		tit = trirep.tris.find(*(rit->begin()));
		if(tit == trirep.tris.end())
		{
			outfile.close();
			return -5;
		}

		/* get height ranges for this room */
		rhit = trirep.room_heights.find(tit->second.root);
		if(rhit == trirep.room_heights.end())
		{
			outfile.close();
			return -6;
		}

		/* print room information */
		outfile << rhit->second.min_z << " "
		        << rhit->second.max_z << " "
			<< rit->size();

		/* print triangles within this room */
		for(nit = rit->begin(); nit != rit->end(); nit++)
		{
			/* print index of this triangle */
			outfile << " " << tri_index_map[*nit];
		}
		outfile << endl;
	}

	/* clean up */
	outfile.close();
	return 0;
}
