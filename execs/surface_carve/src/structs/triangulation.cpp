#include "triangulation.h"
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <math.h>
#include "dgrid.h"
#include "../marching_cubes/LookUpTable.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

using namespace std;

/* The following implement the functions for the voxelface_t class */

voxelface_t::voxelface_t()
{
	/* set some default values, which do not represent
	 * a valid voxelface */
	this->x_ind = this->y_ind = this->z_ind = 0;
	facenum = 10;
}

voxelface_t::voxelface_t(int xi, int yi, int zi, unsigned char fn)
{
	/* record parameters */
	this->x_ind = xi;
	this->y_ind = yi;
	this->z_ind = zi;
	facenum = fn;

	/* edit for uniqueness between all faces */
	switch(fn)
	{
		/* In the following cases, just record values.
		 * don't do anything else. */
		case 0:
		case 3:
		case 4:
			break;

		/* the following edits are for
		 * uniqueness, since a face is shared
		 * by two voxels */
		case 1:
			this->facenum = 3;
			this->x_ind++;
			break;
		case 2:
			this->facenum = 0;
			this->y_ind++;
			break;
		case 5:
			this->facenum = 4;
			this->z_ind++;
			break;
		default:
			/* ERROR, not a valid face number */
			LOGI("[vertex_compute_hash]\tBAD FACE NUM: %d\n",
							this->facenum);
			break;
	}
}

bool operator < (const voxelface_t& lhs, const voxelface_t& rhs)
{
	if(lhs.x_ind < rhs.x_ind)
		return true;
	if(lhs.x_ind > rhs.x_ind)
		return false;
	if(lhs.y_ind < rhs.y_ind)
		return true;
	if(lhs.y_ind > rhs.y_ind)
		return false;
	if(lhs.z_ind < rhs.z_ind)
		return true;
	if(lhs.z_ind > rhs.z_ind)
		return false;
	if(lhs.facenum < rhs.facenum)
		return true;
	return false;
}

bool operator == (const voxelface_t& lhs, const voxelface_t& rhs)
{
	return (lhs.x_ind == rhs.x_ind) && (lhs.y_ind == rhs.y_ind) 
		&& (lhs.z_ind == rhs.z_ind) && (lhs.facenum == rhs.facenum);
}

/* The following implement the functions for the vertex_t class */

vertex_t::vertex_t(voxelface_t& h, dgrid_t& g)
{
	/* record hash value */
	this->hash.x_ind = h.x_ind;
	this->hash.y_ind = h.y_ind;
	this->hash.z_ind = h.z_ind;
	this->hash.facenum = h.facenum;

	/* initialize position */
	this->init_pos(g);

	/* initialize index */
	this->index = -1;

	/* initialize color */
	this->red = 255;
	this->green = 255;
	this->blue = 255;

	/* default to not being a boundary */
	this->boundary = false;

	/* currently not connected to any triangles */
	this->mytris.clear();
}

void vertex_t::init_pos(dgrid_t& g)
{
	int xi, yi, zi;

	/* get originating voxel indices */
	xi = this->hash.x_ind;
	yi = this->hash.y_ind;
	zi = this->hash.z_ind;

	/* set position in 3D space */
	switch(this->hash.facenum)
	{
		case 0:
			this->x = g.vs * ( xi + 0.5 );
			this->y = g.vs * ( yi );
			this->z = g.vs * ( zi + 0.5 );
			break;
		case 1:
			this->x = g.vs * ( this->hash.x_ind + 1 );
			this->y = g.vs * ( this->hash.y_ind + 0.5 );
			this->z = g.vs * ( this->hash.z_ind + 0.5 );
			break;
		case 2:
			this->x = g.vs * ( this->hash.x_ind + 0.5 );
			this->y = g.vs * ( this->hash.y_ind + 1 );
			this->z = g.vs * ( this->hash.z_ind + 0.5 );
			break;
		case 3:
			this->x = g.vs * ( this->hash.x_ind );
			this->y = g.vs * ( this->hash.y_ind + 0.5 );
			this->z = g.vs * ( this->hash.z_ind + 0.5 );
			break;
		case 4:
			this->x = g.vs * ( this->hash.x_ind + 0.5 );
			this->y = g.vs * ( this->hash.y_ind + 0.5 );
			this->z = g.vs * ( this->hash.z_ind );
			break;
		case 5:
			this->x = g.vs * ( this->hash.x_ind + 0.5 );
			this->y = g.vs * ( this->hash.y_ind + 0.5 );
			this->z = g.vs * ( this->hash.z_ind + 1 );
			break;
		default:
			LOGI("[vertex_t::init_pos]"
				"\tBAD FACE NUM: %d\n",
				this->hash.facenum);
			break;
	}
}

/* The following implement the functions for the triangle_t class */

triangle_t::triangle_t(vertex_t** vs, int ind)
{
	int i;

	/* copy each vertex pointer */
	for(i = 0; i < NUM_VERTS_PER_TRI; i++)
		this->v[i] = vs[i];

	/* initialize neighbor pointers to be null */
	for(i = 0; i < NUM_EDGES_PER_TRI; i++)
		this->t[i] = NULL;

	/* initialize index */
	index = ind;
	region_neigh_count = 0;
}
	
int triangle_t::shortest_edge()
{
	double d, d_best;
	int i_best;

	/* iterate over edges, checking lengths */
	
	/* start with edge #0 */
	i_best = 0;
	d_best = (this->v[1]->x * this->v[2]->x)
			+ (this->v[1]->y * this->v[2]->y)
			+ (this->v[1]->z * this->v[2]->z);

	/* check edge #1 */
	d =  (this->v[0]->x * this->v[2]->x)
			+ (this->v[0]->y * this->v[2]->y)
			+ (this->v[0]->z * this->v[2]->z);
	if(d_best > d)
	{
		i_best = 1;
		d_best = d;
	}
	
	/* check edge #2 */
	d =  (this->v[0]->x * this->v[1]->x)
			+ (this->v[0]->y * this->v[1]->y)
			+ (this->v[0]->z * this->v[1]->z);
	if(d_best > d)
	{
		i_best = 2;
		d_best = d;
	}

	/* return result */
	return i_best;
}
	
bool triangle_t::shares_edge_with(triangle_t* other)
{
	int i, j, n;

	/* check for matches between the vertices of two
	 * triangles. */
	n = 0;
	for(i = 0; i < NUM_VERTS_PER_TRI; i++)
		for(j = 0; j < NUM_VERTS_PER_TRI; j++)
			if(this->v[i] == other->v[j])
				n++;

	/* check if we found an edge's worth of matches */
	return (n == NUM_VERTS_PER_EDGE);
}

double triangle_t::area()
{
	double m, x, y, z;

	/* get determinant elements using triangle vertices */
	double ux = this->v[0]->x - this->v[2]->x;
	double uy = this->v[0]->y - this->v[2]->y;
	double uz = this->v[0]->z - this->v[2]->z;
	double vx = this->v[1]->x - this->v[2]->x;
	double vy = this->v[1]->y - this->v[2]->y;
	double vz = this->v[1]->z - this->v[2]->z;
	
	/* compute normal */
	x = (uy * vz) - (uz * vy);
	y = (uz * vx) - (ux * vz);
	z = (ux * vy) - (uy * vx);

	/* compute magnitude */
	m = sqrt( (x * x) + (y * y) + (z * z) );

	/* area is half of this */
	return m/2;
}

void triangle_t::print()
{
	int i;

	/* print info about triangle */
	LOGI("\ntriangle %p\n", (void*) this);
	LOGI("\tindex = %d\n", this->index);
	LOGI("\tregion_id = %d\n", this->region_id);
	LOGI("\tregion_neigh_count = %d\n", this->region_neigh_count);

	LOG("\tvertices:\n");
	for(i = 0; i < NUM_VERTS_PER_TRI; i++)
	{
		LOGI("\t\tv[%d]", i);
		LOGI(" = %p\n", (void*) this->v[i]);
	}
	
	LOG("\tneighbors:\n");
	for(i = 0; i < NUM_EDGES_PER_TRI; i++)
	{
		LOGI("\t\tt[%d]", i);
		LOGI(" = %p\n", (void*) this->t[i]);
	}
}

/************* TRIANGULATION FUNCTIONS *********************/

triangulation_t::triangulation_t()
{
	/* empty triangulation */
	this->vertices.clear();
	this->triangles.clear();
}

triangulation_t::~triangulation_t()
{
	map<voxelface_t, vertex_t*>::iterator vit;
	vector<triangle_t*>::iterator tit;

	/* free all vertices */
	for(vit = this->vertices.begin(); vit != this->vertices.end();
								vit++)
		delete (*vit).second;
	this->vertices.clear();

	/* free all triangles */
	for(tit = this->triangles.begin(); tit != this->triangles.end(); 
								tit++)
		delete (*tit);
	this->triangles.clear();
}

/* Triangulate a voxel grid */
int triangulation_t::generate(dgrid_t& g)
{
	map<voxel_t, voxel_state_t>::iterator it;
	voxel_t v, mc, vo;
	voxel_state_t s;
	int i, ret;
	int cube_description; /* the i'th bit is 1 
			iff the i'th corner is solid */

	/* iterate over the boundary voxels of g,
	 * which represent the corners of cubes used for
	 * the triangulation
	 *
	 * Every interesting cube has at least one boundary corner */
	for(it = g.voxels.begin(); it != g.voxels.end(); it++)
	{
		/* get current voxel */
		v = (*it).first;
		s = (*it).second;

		/* the only voxels stored in the grid should be boundary
		 * voxels, but check the state just in case */
		if(s == VOXEL_STATE_NONBOUNDARY)
			continue;

		/* each voxel can be used in up to eight cubes, denoted
		 * by each voxel corner.  Iterate over the corners of
		 * the current voxel, and determine if that cube is
		 * interesting */
		for(i = 0; i < NUM_CORNERS_PER_CUBE; i++)
		{
			/* this voxel corner defines a cube in the
			 * lattice used by marching cubes.  This cube
			 * will have v as one of its corners, but is
			 * more compactly represented by its minimum
			 * corner position. */
			mc.set(v.x_ind - 1 + voxel_corner_pos[i][0],
				v.y_ind - 1 + voxel_corner_pos[i][1],
				v.z_ind - 1 + voxel_corner_pos[i][2]);

			/* Analyze this cube, defined by corner i, to see if
			 * marching cubes should be computed here.
			 *
			 * find the current cube description */
			cube_description = get_cube_description(mc, v, g);

			/* check if we should process this cube */
			if(cube_description == 0)
				continue;

			/* we now have a complete cube description,
			 * and know that we want to process
			 * this cube now, so perform marching
			 * cubes on this cube. */
			this->do_cube(cube_description, mc, g);
		}
	}

	/* arrange neighbor map for vertices and triangles */
	ret = this->map_neighbors();
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	
	/* success */
	return 0;
}
	
int triangulation_t::get_cube_description(voxel_t& mc, voxel_t& v, 
							dgrid_t& g)
{
	int j, k, c, cube_description, verified;
	voxel_t vo;
	voxel_state_t so;
	
	/* set description for current cube to be default */
	cube_description = 0;

	/* set verified bits to be zero.  If a bit in verified
	 * is 1, that means that we are certain of the corresponding
	 * bit's value in cube_description. */
	verified = 0;

	/* check each corner of this cube (which is a voxel), 
	 * and record the state */
	for(j = 0; j < NUM_CORNERS_PER_CUBE; j++)
	{
		/* get the voxel that resides at the j'th corner
		 * of the cube */
		vo.set( mc.x_ind + voxel_corner_pos[j][0],
			mc.y_ind + voxel_corner_pos[j][1],
			mc.z_ind + voxel_corner_pos[j][2]);

		/* get the state of this voxel */
		so = g.get_voxel_state(vo);

		/* we only care about boundary voxels */
		if(so == VOXEL_STATE_NONBOUNDARY)
			continue;

		/* since each cube will be seen
		 * many times, only process this
		 * cube if v is the smallest
		 * voxel in it.  Thus, if vo is
		 * smaller, skip this cube */
		if(vo < v)
			return 0;

		/* we know that the current corner
		 * is solid (since it is a boundary) */
		cube_description |= (1 << j);
		verified |= (1 << j);

		/* check boundary voxel faces to 
		 * find other solid corners */
		for(k = 0; k < NUM_EDGES_PER_CORNER_PER_CUBE; k++)
		{
			/* get the bit number of the corner
			 * corresponding to the k'th edge */
			c = voxel_corner_traversal_table[j][
				NUM_EDGES_PER_CORNER_PER_CUBE + k];
			
			/* check if k'th face opposite corner j
			 * is inward-facing */
			if(!VOXEL_IS_FACE_BIT_INWARD(so, 
					voxel_corner_traversal_table[j][k]))
			{
				/* We know that the voxel at the
				 * corner of the cube in the direction
				 * of that face must also be solid */
				cube_description |= (1 << c);
			}
			verified |= (1 << c);
		}
	}

	/* check if there are any corners that remain unverified. */
	while(verified != MAX_BYTE) /* stop if all 8 corners are verified */
	{
		for(c = 0; c < NUM_CORNERS_PER_CUBE; c++)
		{
			/* check if the c'th corner is unverified */
			if(BITGET(verified, c))
				continue;

			/* we need to verify the c'th corner. We can
			 * do this by checking all of its neighboring
			 * corners.  If any of them are verified,
			 * corner c must have the same description as
			 * them. */
			for(j = 0; j < NUM_EDGES_PER_CORNER_PER_CUBE; j++)
			{
				/* get the neighboring corner number */
				k = voxel_corner_traversal_table[c][j
					+ NUM_EDGES_PER_CORNER_PER_CUBE];

				/* check if k is verified */
				if(BITGET(verified, k))
				{
					/* set corner c to have the
					 * same description as corner k */
					if(BITGET(cube_description, k))
						cube_description |= (1<<c);

					/* verify corner c */
					verified |= (1<<c);

					/* we're done with corner c */
					break;
				}
			}
		}
	}

	/* success, return the resultant description.  Note that
	 * the current cube description will yield triangles that
	 * are counter-clockwise facing outward, while we would
	 * prefer counter-clockwise facing inward, so invert all
	 * relevent bits */
	return MAX_BYTE & (~cube_description);
}

void triangulation_t::do_cube(int cube_description, voxel_t& min_corner,
							dgrid_t& g)
{	
	int i, j, en;
	vertex_t* verts[NUM_VERTS_PER_TRI];
	triangle_t* t;
	voxelface_t hash;
	map<voxelface_t, vertex_t*>::iterator vit;
	
	/* NOTE: should probably use enhanced marching cubes, found on:
	 * 	MarchingCubes.cpp:425 */	

	/* get triangles to add for this cube_description */
	for(i = 0; i < MARCHING_CUBES_CASE_LEN; i += NUM_VERTS_PER_TRI)
	{
		/* In the list casesClassic[cube_description][*],
		 * the indices i,i+1,i+2 define a single triangle */
		if(casesClassic[cube_description][i] < 0)
			return; /* no more triangles */
		
		/* initialize variables */
		for(j = 0; j < NUM_VERTS_PER_TRI; j++)
			verts[j] = NULL;

		/* add the corresponding vertices */
		for(j = 0; j < NUM_VERTS_PER_TRI; j++)
		{
			/* get current edge */
			en = casesClassic[cube_description][i+j];

			/* convert to vertex representation 
			 * based on the edge described */
			switch(en)
			{
				case 0:
					hash = voxelface_t(
						min_corner.x_ind,
						min_corner.y_ind,
						min_corner.z_ind,
							1);
					break;
				case 1:
					hash = voxelface_t(
						min_corner.x_ind+1,
						min_corner.y_ind,
						min_corner.z_ind,
							2);
					break;
				case 2:
					hash = voxelface_t(
						min_corner.x_ind,
						min_corner.y_ind+1,
						min_corner.z_ind,
							1);
					break;
				case 3:
					hash = voxelface_t(
						min_corner.x_ind,
						min_corner.y_ind,
						min_corner.z_ind,
							2);
					break;
				case 4:
					hash = voxelface_t(
						min_corner.x_ind,
						min_corner.y_ind,
						min_corner.z_ind+1,
							1);
					break;
				case 5:
					hash = voxelface_t(
						min_corner.x_ind+1,
						min_corner.y_ind,
						min_corner.z_ind+1,
							2);
					break;
				case 6:
					hash = voxelface_t(
						min_corner.x_ind,
						min_corner.y_ind+1,
						min_corner.z_ind+1,
							1);
					break;
				case 7:
					hash = voxelface_t(
						min_corner.x_ind,
						min_corner.y_ind,
						min_corner.z_ind+1,
							2);
					break;
				case 8:
					hash = voxelface_t(
						min_corner.x_ind,
						min_corner.y_ind,
						min_corner.z_ind,
						5);
					break;
				case 9:
					hash = voxelface_t(
						min_corner.x_ind+1,
						min_corner.y_ind,
						min_corner.z_ind,
						5);
					break;
				case 10:
					hash = voxelface_t(
						min_corner.x_ind+1,
						min_corner.y_ind+1,
						min_corner.z_ind,
						5);
					break;
				case 11:
					hash = voxelface_t(
						min_corner.x_ind,
						min_corner.y_ind+1,
						min_corner.z_ind,
						5);
					break;
				default:
					LOGI("[do_cube]"
						"\tBAD EDGE NUMBER: %d\n", 
						en);
					break;
			}

			/* check if vertex already exists 
			 * in the triangulation */
			vit = this->vertices.find(hash);
			if(vit == this->vertices.end())
			{
				/* create a new vertex and store
				 * at this location in vertex map */
				verts[j] = new vertex_t(hash, g);
				if(!verts[j])
				{
					LOG("[do_cube]\tnull vertex!\n");
				}
				
				/* insert into vertex map */
				this->vertices.insert(pair<voxelface_t,
					vertex_t*>(hash, verts[j]));
			}
			else
			{
				/* record existing vertex as part
				 * of this triangle */
				verts[j] = (*vit).second;
			}
		}
	
		/* Create the triangle, referencing these vertices.
		 * The look-up table creates triangles that are
		 * counter-clockwise facing empty. */
		t = new triangle_t(verts, this->triangles.size());
		if(!t)
		{
			LOG("[do_cube]\tnull triangle!\n");
		}
		this->triangles.push_back(t);
	}
}

int triangulation_t::map_neighbors()
{
	unsigned int i, vi, wi, si, n;
	map<voxelface_t, vertex_t*>::iterator vit;
	vector<triangle_t*>::iterator tit;
	vector<triangle_t*> ens;
	vector<triangle_t*>::iterator eit;
	triangle_t* t;

	/* clear all vertex sets */
	for(vit=this->vertices.begin(); vit!=this->vertices.end(); vit++)
		(*vit).second->mytris.clear();

	/* iterate over triangles, and record all triangles
	 * that contain each vertex */
	for(tit = this->triangles.begin(); tit != this->triangles.end();
								tit++)
	{
		/* store triangle in each of its vertices */
		for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			(*tit)->v[i]->mytris.push_back(*tit);
	}

	/* for each vertex, sort triangle pointers */
	for(vit = this->vertices.begin(); vit != this->vertices.end();
								vit++)
		sort((*vit).second->mytris.begin(), 
			(*vit).second->mytris.end());

	/* iterate over all triangles again to find triangle
	 * edge neighbor pointers */
	ens.clear();
	ens.resize(MAX_BYTE); /* only two triangles can share 
					one edge, but leave room in case 
					of error */
	for(tit = this->triangles.begin(); tit != this->triangles.end();
								tit++)
	{
		/* for each edge in this triangle, find
		 * other triangles that share this edge */
		for(vi = 0; vi < NUM_EDGES_PER_TRI; vi++)
		{
			/* v-w defines the current edge, where
			 * s is the only vertex not part of this edge */
			wi = (vi+1) % NUM_EDGES_PER_TRI;
			si = (vi+2) % NUM_EDGES_PER_TRI;

			/* get all triangles that intersect edge v-w */
			eit = set_intersection(
					(*tit)->v[vi]->mytris.begin(), 
					(*tit)->v[vi]->mytris.end(), 
					(*tit)->v[wi]->mytris.begin(), 
					(*tit)->v[wi]->mytris.end(), 
					ens.begin());

			n = eit - ens.begin();

			/* better be 2 for a watertight surface */
			if(n < 2)
			{
				PRINT_WARNING("[map_neighbors]\t"
						"BAD TRIANGULATION:");
				LOGI("\t\tedge intersects with n = %d"
					" triangles.\n", n);	
			
				/* debugging by adding color */
				(*tit)->v[vi]->red = 255;
				(*tit)->v[vi]->green = 255;
				(*tit)->v[vi]->blue = 0;
				(*tit)->v[wi]->red = 255;
				(*tit)->v[wi]->green = 255;
				(*tit)->v[wi]->blue = 0;
				
				continue;
			}
			if(n > 2)
			{
				PRINT_ERROR("[map_neighbors]\t"
						"BAD TRIANGULATION:");
				LOGI("\t\tedge intersects with n = %d"
					" triangles.\n", n);
				
				/* debugging by adding color */
				(*tit)->v[vi]->red = 0;
				(*tit)->v[vi]->green = 255;
				(*tit)->v[vi]->blue = 0;
				(*tit)->v[wi]->red = 0;
				(*tit)->v[wi]->green = 255;
				(*tit)->v[wi]->blue = 0;
				
				continue;
			}

			/* iterate over these triangles */
			for(i = 0; i < n; i++)
			{
				/* only add triangles that are not (*tit) */
				t = ens[i];
				if(t != (*tit))
					(*tit)->t[si] = t;
			}
		}
	}

	/* success */
	return 0;
}

void triangulation_t::index_vertices()
{
	int i;
	map<voxelface_t, vertex_t*>::iterator it;

	/* begin index counter */
	i = 0;
	
	/* iterate over all vertices */
	for(it = this->vertices.begin(); it != this->vertices.end(); it++)
	{
		/* give the current vertex a unique index */
		(*it).second->index = (i++);
	}
}

void triangulation_t::index_triangles()
{
	int i;
	vector<triangle_t*>::iterator it;

	/* begin index counter */
	i = 0;

	/* iterate through triangles */
	for(it = this->triangles.begin(); it != this->triangles.end(); it++)
	{
		/* give current triangle unique index */
		(*it)->index = (i++);
	}
}
	
void triangulation_t::reset_vertex_pos(dgrid_t& g)
{
	map<voxelface_t, vertex_t*>::iterator it;
	for(it = this->vertices.begin(); it != this->vertices.end(); it++)
		(*it).second->init_pos(g);
}
