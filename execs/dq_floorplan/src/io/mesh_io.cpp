#include "mesh_io.h"
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <float.h>
#include "../structs/normal.h"
#include "../rooms/tri_rep.h"
#include "../util/error_codes.h"

using namespace std;

int writeobj(char* filename, tri_rep_t& trirep)
{
	map<triple_t, room_height_t>::iterator rit;
	map<triple_t, tri_info_t>::iterator tit;
	map<int, set<triple_t> >::iterator vit;	
	map<int, int> index_map;
	map<int, int>::iterator mit_i, mit_j, mit_k;
	set<triple_t>::iterator nit;
	vector<edge_t> walls;
	vector<edge_t>::iterator wit;
	ofstream outfile;
	vertex_t* p;
	point_t pt1, pt2;
	int ret, r, g, b, num_verts;
	double min_z, max_z;

	/* open file for writing */
	outfile.open(filename);
	if(!(outfile.is_open()))
		return -1;

	/* print some comments */
	outfile << "# 2.5D Generated Model" << endl 
		<< endl;

	/* create a mapping from the indices of vertices used by
	 * these structures to indices used in the OBJ file. */
	num_verts = 0;
	for(vit = trirep.vert_map.begin(); vit != trirep.vert_map.end();
								vit++)
	{
		/* check if this vertex is actually used */
		if(vit->second.empty())
			continue;

		/* determine height of floor and ceiling at this vertex */
		min_z = -DBL_MAX;
		max_z = DBL_MAX;
		for(nit = vit->second.begin(); nit != vit->second.end(); 
								nit++)
		{
			/* get info about this neighboring triangle */
			tit = trirep.tris.find(*nit);
			if(tit == trirep.tris.end())
			{
				/* uh oh! */
				outfile.close();
				return -2;
			}

			/* look up the room heights */
			rit = trirep.room_heights.find(tit->second.root);
			if(rit == trirep.room_heights.end())
			{
				/* also uh oh! */
				outfile.close();
				return -3;
			}

			/* compare to current range of heights,
			 * and choose the intersection of all ranges */
			if(rit->second.min_z > min_z)
				min_z = rit->second.min_z;
			if(rit->second.max_z < max_z)
				max_z = rit->second.max_z;
		}
		
		/* get the vertex position */
		p = TRI_VERTEX_POS(&(trirep.tri), vit->first);

		/* get some color */
		trirep.color_by_room(vit->first, r, g, b);

		/* make a unique index for this vertex, start at
		 * index 1, since OBJ's are like that... */
		num_verts++;

		/* add to map */
		index_map.insert(pair<int,int>(vit->first, 
						num_verts));
	
		/* print the vertex (floor level) */
		outfile << "v " << p->pos[VERTEX_X_IND]
		        <<  " " << p->pos[VERTEX_Y_IND]
			<<  " " << min_z
			<<  " " << r
			<<  " " << g
			<<  " " << b
			<< endl;

		/* increment count for ceiling vertex as well */
		num_verts++;

		/* print the vertex (ceiling level) */
		outfile << "v " << p->pos[VERTEX_X_IND]
		        <<  " " << p->pos[VERTEX_Y_IND]
			<<  " " << max_z
			<<  " " << r
			<<  " " << g
			<<  " " << b
			<< endl;
	}

	/* there are now 2*num_verts_2D vertices.  The odd vertices are
	 * on the floor, and the even vertices are on the ceiling. */
	outfile << endl;

	/* print the floor and ceiling triangles */
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

		/* print floor */
		outfile << "f " << mit_i->second
		        <<  " " << mit_j->second
			<<  " " << mit_k->second
			<< endl;

		/* print ceiling, which has a reversed normal */
		outfile << "f " << (1+mit_k->second)
		        <<  " " << (1+mit_j->second)
			<<  " " << (1+mit_i->second)
			<< endl;
	}

	/* compute walls */
	walls.clear();
	ret = trirep.get_walls(walls);
	if(ret)
		return PROPEGATE_ERROR(-5, ret);

	/* print the wall triangles */
	for(wit = walls.begin(); wit != walls.end(); wit++)
	{
		/* get OBJ indices for these cells */
		mit_i = index_map.find(wit->i);
		mit_j = index_map.find(wit->j);
		if(mit_i == index_map.end() || mit_j == index_map.end())
		{
			outfile.close();
			return -6;
		}

		/* print edge in two triangles */
		outfile << "f " << mit_i->second
		        <<  " " << (1+mit_j->second)
			<<  " " << mit_j->second
			<< endl
		        << "f " << mit_i->second
			<<  " " << (1+mit_i->second)
			<<  " " << (1+mit_j->second)
			<< endl;
	}

	/* clean up */
	outfile.close();
	return 0;
}

int writeobj_2d(char* filename, tri_rep_t& trirep)
{
	map<triple_t, tri_info_t>::iterator tit;
	ofstream outfile;
	vertex_t* p;
	int r, g, b, num_verts;

	/* open file for writing */
	outfile.open(filename);
	if(!(outfile.is_open()))
		return -1;

	/* print some comments */
	outfile << "# 2D Generated Model" << endl 
		<< endl;

	/* print the floor triangles */
	num_verts = 0;
	for(tit = trirep.tris.begin(); tit != trirep.tris.end(); tit++)
	{
		/* get triangle color */
		trirep.color_by_room(tit->first, r, g, b);
		
		/* get the vertex position */
		p = TRI_VERTEX_POS(&(trirep.tri), tit->first.i);
		outfile << "v " << p->pos[VERTEX_X_IND]
		        <<  " " << p->pos[VERTEX_Y_IND]
			<<  " " << 0
			<<  " " << r
			<<  " " << g
			<<  " " << b
			<< endl;

		/* get the vertex position */
		p = TRI_VERTEX_POS(&(trirep.tri), tit->first.j);
		outfile << "v " << p->pos[VERTEX_X_IND]
		        <<  " " << p->pos[VERTEX_Y_IND]
			<<  " " << 0
			<<  " " << r
			<<  " " << g
			<<  " " << b
			<< endl;

		/* get the vertex position */
		p = TRI_VERTEX_POS(&(trirep.tri), tit->first.k);
		outfile << "v " << p->pos[VERTEX_X_IND]
		        <<  " " << p->pos[VERTEX_Y_IND]
			<<  " " << 0
			<<  " " << r
			<<  " " << g
			<<  " " << b
			<< endl;

		/* print floor */
		outfile << "f " << (1+num_verts)
		        <<  " " << (2+num_verts)
			<<  " " << (3+num_verts)
			<< endl;
		
		/* update counter */
		num_verts += 3;
	}

	/* clean up */
	outfile.close();
	return 0;
}

int writeedge(char* filename, tri_rep_t& trirep)
{
	vector<edge_t> walls;
	ofstream outfile;
	point_t p, q;
	int i, n, ret;

	/* open file for writing */
	outfile.open(filename);
	if(!(outfile.is_open()))
		return -1;

	/* compute walls */
	walls.clear();
	ret = trirep.get_walls(walls);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* iterate over walls */
	n = walls.size();
	for(i = 0; i < n; i++)
	{
		/* get edge positions */
		p = trirep.pos(walls[i].i);
		q = trirep.pos(walls[i].j);

		/* write to file */
		outfile << p.get(0) << " "
		        << p.get(1) << " "
			<< q.get(0) << " "
			<< q.get(1) << endl;
	}

	/* clean up */
	outfile.close();
	return 0;
}

int writeply(char* filename, tri_rep_t& trirep)
{	
	map<triple_t, room_height_t>::iterator rit;
	map<triple_t, tri_info_t>::iterator tit;
	map<int, set<triple_t> >::iterator vit;	
	
	map<int, int> index_map;
	map<int, int>::iterator mit_i, mit_j, mit_k;
	
	set<triple_t>::iterator nit;
	
	map<triple_t, int> room_to_floor_region_map;
	map<triple_t, int>::iterator rtfrit;
	vector<edge_t> wall_list;

	vector<vector<double> > pts_to_write;
	vector<vector<int> > tris_to_write;
	vector<vector<double> > region_normals;
	vector<vector<double> > region_pts;
	vector<vector<int> > region_tri_inds;
	vector<vector<int> > region_edges;
	
	vertex_t* p;
	point_t pt1, pt2;
	double min_z, max_z;
	triple_t t, tf, tb, room_root;
	normal_t norm, norm2;
	int i, j, n, m, ret;
	
	ofstream outfile;

	/* initialize regions for the floor and ceiling for each room */
	for(tit = trirep.tris.begin(); tit != trirep.tris.end(); tit++)
	{
		/* check if this triangle's room's root has been added */
		room_root = tit->second.root;
		if(room_to_floor_region_map.count(room_root))
			continue;

		/* create a new floor region for this room, so
		 * we should remember the mapping between root and
		 * region number.  Add one to floor index to get ceiling
		 * region index. */
		room_to_floor_region_map.insert(pair<triple_t, int>(
				tit->second.root, region_normals.size()));

		/* get heights */
		rit = trirep.room_heights.find(tit->second.root);
		if(rit == trirep.room_heights.end())
			return -1;

		/* make new floor region */
		region_normals.push_back(vector<double>(3));
		region_normals.back()[0] = 0;
		region_normals.back()[1] = 0;
		region_normals.back()[2] = 1;
		region_pts.push_back(vector<double>(3));
		region_pts.back()[0] = 0;
		region_pts.back()[1] = 0;
		region_pts.back()[2] = rit->second.min_z;
		region_tri_inds.push_back(vector<int>(0));
		region_edges.push_back(vector<int>(0));

		/* make new ceiling region */
		region_normals.push_back(vector<double>(3));
		region_normals.back()[0] = 0;
		region_normals.back()[1] = 0;
		region_normals.back()[2] = -1;
		region_pts.push_back(vector<double>(3));
		region_pts.back()[0] = 0;
		region_pts.back()[1] = 0;
		region_pts.back()[2] = rit->second.max_z;
		region_tri_inds.push_back(vector<int>(0));
		region_edges.push_back(vector<int>(0));
	}
	
	/* even regions are floors, odd regions are ceilings */

	/* create a mapping from the indices of vertices used by
	 * these structures to indices used in the PLY file. */
	for(vit = trirep.vert_map.begin(); vit != trirep.vert_map.end();
								vit++)
	{
		/* check if this vertex is actually used */
		if(vit->second.empty())
			continue;

		/* determine height of floor and ceiling at this vertex */
		min_z = -DBL_MAX;
		max_z = DBL_MAX;
		for(nit = vit->second.begin(); nit != vit->second.end(); 
								nit++)
		{
			/* get info about this neighboring triangle */
			tit = trirep.tris.find(*nit);
			if(tit == trirep.tris.end())
			{
				/* uh oh! */
				return -2;
			}

			/* look up the room heights */
			rit = trirep.room_heights.find(tit->second.root);
			if(rit == trirep.room_heights.end())
			{
				/* also uh oh! */
				return -3;
			}

			/* compare to current range of heights,
			 * and choose the intersection of all ranges */
			if(rit->second.min_z > min_z)
				min_z = rit->second.min_z;
			if(rit->second.max_z < max_z)
				max_z = rit->second.max_z;
		}
		
		/* get the vertex position */
		p = TRI_VERTEX_POS(&(trirep.tri), vit->first);

		/* add to map */
		index_map.insert(pair<int,int>(vit->first, 
						pts_to_write.size()));
	
		/* print the vertex (floor level) */
		pts_to_write.push_back(vector<double>(3));
		pts_to_write.back()[0] = p->pos[VERTEX_X_IND];
		pts_to_write.back()[1] = p->pos[VERTEX_Y_IND];
		pts_to_write.back()[2] = min_z;

		/* print the vertex (ceiling level) */
		pts_to_write.push_back(vector<double>(3));
		pts_to_write.back()[0] = p->pos[VERTEX_X_IND];
		pts_to_write.back()[1] = p->pos[VERTEX_Y_IND];
		pts_to_write.back()[2] = max_z;
	}

	/* there are now 2*num_verts_2D vertices.  The even vertices are
	 * on the floor, and the odd vertices are on the ceiling. */

	/* print the floor and ceiling triangles */
	for(tit = trirep.tris.begin(); tit != trirep.tris.end(); tit++)
	{
		/* get the OBJ indices of the vertices of this triangle */
		mit_i = index_map.find(tit->first.i);
		mit_j = index_map.find(tit->first.j);
		mit_k = index_map.find(tit->first.k);
		if(mit_i == index_map.end() || mit_j == index_map.end()
					|| mit_k == index_map.end())
		{
			return -4;
		}

		/* print floor */
		tris_to_write.push_back(vector<int>(3));
		tris_to_write.back()[0] = mit_i->second;
		tris_to_write.back()[1] = mit_j->second;
		tris_to_write.back()[2] = mit_k->second;

		/* get region id for this triangle */
		rtfrit = room_to_floor_region_map.find(tit->second.root);
		if(rtfrit == room_to_floor_region_map.end())
			return -5;

		/* add region information */
		region_tri_inds[rtfrit->second].push_back(
					tris_to_write.size() - 1);

		/* print ceiling, which has a reversed normal */
		tris_to_write.push_back(vector<int>(3));
		tris_to_write.back()[0] = 1+mit_k->second;
		tris_to_write.back()[1] = 1+mit_j->second;
		tris_to_write.back()[2] = 1+mit_i->second;
		
		/* add region information */
		region_tri_inds[rtfrit->second + 1].push_back(
					tris_to_write.size() - 1);	
	
		/* get region edges for both floor and ceiling that
		 * are connected to this triangle */
		
		/* check edge (i,j) */
		if(trirep.room_edge(tit->first.i, tit->first.j, t))
		{
			/* add mit_i, mit_j as edge to floor */
			region_edges[rtfrit->second].push_back(
							mit_i->second);
			region_edges[rtfrit->second].push_back(
							mit_j->second);

			/* add mit_j, mit_i as edge to ceiling */
			region_edges[1+rtfrit->second].push_back(
							1+mit_j->second);
			region_edges[1+rtfrit->second].push_back(
							1+mit_i->second);
		}
		
		/* check edge (j,k) */
		if(trirep.room_edge(tit->first.j, tit->first.k, t))
		{
			/* add mit_j, mit_k as edge to floor */
			region_edges[rtfrit->second].push_back(
							mit_j->second);
			region_edges[rtfrit->second].push_back(
							mit_k->second);

			/* add mit_k, mit_j as edge to ceiling */
			region_edges[1+rtfrit->second].push_back(
							1+mit_k->second);
			region_edges[1+rtfrit->second].push_back(
							1+mit_j->second);
		}
		
		/* check edge (k,i) */
		if(trirep.room_edge(tit->first.k, tit->first.i, t))
		{
			/* add mit_k, mit_i as edge to floor */
			region_edges[rtfrit->second].push_back(
							mit_k->second);
			region_edges[rtfrit->second].push_back(
							mit_i->second);

			/* add mit_i, mit_k as edge to ceiling */
			region_edges[1+rtfrit->second].push_back(
							1+mit_i->second);
			region_edges[1+rtfrit->second].push_back(
							1+mit_k->second);
		}
	}

	/* partition all the walls into nearly planar regions */
	ret = trirep.get_walls(wall_list);
	if(ret)
		return PROPEGATE_ERROR(-6, ret);

	/* iterate over each wall region */
	n = wall_list.size();
	for(i = 0; i < n; i++)
	{
		/* we need to convert the i'th wall region into
		 * a 3D region for the PLY file.  So make a new
		 * region in the lists. */
		region_tri_inds.push_back(vector<int>(0));
		region_edges.push_back(vector<int>(0));

		/* get wall position */
		pt1 = trirep.pos(wall_list[i].i);
		pt2 = trirep.pos(wall_list[i].j);

		/* point on region's plane: average of cell positions */
		region_pts.push_back(vector<double>(3,0));
		region_pts.back()[0] = (pt1.get(0) + pt2.get(0))/2;
		region_pts.back()[1] = (pt1.get(1) + pt2.get(1))/2;

		/* region normal: average of edges, weighted
		 * by edge length */
		norm.dir(pt1, pt2); 
		norm.rotate90();
		region_normals.push_back(vector<double>(3,0.0));
		region_normals.back()[0] = norm.get(0);
		region_normals.back()[1] = norm.get(1);
	
		/* get the index mapping for this edge */
		mit_i = index_map.find(wall_list[i].i);
		mit_j = index_map.find(wall_list[i].j);
		if(mit_i == index_map.end() || mit_j == index_map.end())
			return -7;
			
		/* make triangles from this edge */
			
		/* lower triangle */
		tris_to_write.push_back(vector<int>(3));
		tris_to_write.back()[0] = mit_i->second;
		tris_to_write.back()[1] = 1+mit_j->second;
		tris_to_write.back()[2] = mit_j->second;
				
		/* upper triangle */
		tris_to_write.push_back(vector<int>(3));
		tris_to_write.back()[0] = mit_i->second;
		tris_to_write.back()[1] = 1+mit_i->second;
		tris_to_write.back()[2] = 1+mit_j->second;
		
		/* add these triangles to the current region */
		region_tri_inds.back().push_back(tris_to_write.size()-2);
		region_tri_inds.back().push_back(tris_to_write.size()-1);
			
		/* add edges to region */

		/* top edge, along ceiling */
		region_edges.back().push_back(1+mit_i->second);
		region_edges.back().push_back(1+mit_j->second);
			
		/* bottom edge, along floor */
		region_edges.back().push_back(mit_j->second);
		region_edges.back().push_back(mit_i->second);	

		/* starting edge, going up */
		region_edges.back().push_back(mit_i->second);
		region_edges.back().push_back(1+mit_i->second);

		/* ending edge, going down */
		region_edges.back().push_back(1+mit_j->second);
		region_edges.back().push_back(mit_j->second);
	}

	/* now that we have all the information about this 3D mesh
	 * compiled, we can write it to the file */

	/* open file for writing */
	outfile.open(filename);
	if(!(outfile.is_open()))
		return -8;

	/* write header */
	outfile << "ply" << endl
	        << "format ascii 1.0" << endl
		<< "element vertex " << pts_to_write.size() << endl
		<< "property float x" << endl
		<< "property float y" << endl
		<< "property float z" << endl
		<< "element face " << tris_to_write.size() << endl
		<< "property list uchar int vertex_index" << endl
		<< "element region " << region_normals.size() << endl
		<< "property float nx" << endl
		<< "property float ny" << endl
		<< "property float nz" << endl
		<< "property float px" << endl
		<< "property float py" << endl
		<< "property float pz" << endl
		<< "property list int int triangle_index" << endl
		<< "property list int int edge_pair_index" << endl
		<< "end_header" << endl;

	/* write vertices to disk */
	n = pts_to_write.size();
	for(i = 0; i < n; i++)
		outfile << pts_to_write[i][0] << " " 
		        << pts_to_write[i][1] << " "
			<< pts_to_write[i][2] << endl;

	/* write triangles to disk */
	n = tris_to_write.size();
	for(i = 0; i < n; i++)
		outfile << "3 "
		        << tris_to_write[i][0] << " "  
		        << tris_to_write[i][1] << " "  
		        << tris_to_write[i][2] << endl;

	/* write regions to disk */
	n = region_normals.size();
	for(i = 0; i < n; i++)
	{
		/* print out plane geometry info */
		outfile << region_normals[i][0] << " "
		        << region_normals[i][1] << " " 
		        << region_normals[i][2] << " "
		        << region_pts[i][0] << " "
		        << region_pts[i][1] << " "
		        << region_pts[i][2] << " ";

		/* print out triangles contained in this region */
		m = region_tri_inds[i].size();
		outfile << m;
		for(j = 0; j < m; j++)
			outfile << " " << region_tri_inds[i][j];

		/* print edges */
		m = region_edges[i].size();
		outfile << " " << m;
		for(j = 0; j < m; j++)
			outfile << " " << region_edges[i][j];
	
		/* end this region */
		outfile << endl;
	}

	/* clean up */
	outfile.close();
	return 0;
}
