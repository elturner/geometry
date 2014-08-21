#include "create_mesh.h"
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <math.h>
#include "postprocessing.h"
#include "../io/config.h"
#include "../structs/point.h"
#include "../structs/normal.h"
#include "../structs/dgrid.h"
#include "../structs/mesher.h"
#include "../structs/triangulation.h"
#include "../structs/quadtree.h"
#include "../triangulate/union_find_faces.h"
#include "../triangulate/region_growing.h"
#include "../math/mathlib.h"
#include "../util/tictoc.h"
#include "../util/error_codes.h"

using namespace std;

/* convert from a voxel grid to a triangulated mesh */
int create_mesh(triangulation_t& tri, vector<planar_region_t>& regions,
					dgrid_t& grid, config_t& conf)
{
	mesher_t mesh;
	tictoc_t clk;
	int ret;

	/* check if we should process the triangulation uniformly */
	if(conf.uniform)
	{
		/* triangulate to get a mesh */
		tic(clk);
		ret = tri.generate(grid);
		if(ret)
			return PROPEGATE_ERROR(-10, ret);
		toc(clk, "Triangulating mesh");

		/* improve mesh with post-processing */
		ret = post_processing(tri, regions, conf);
		if(ret)
			return PROPEGATE_ERROR(-11, ret);

		/* success */
		return 0;
	}

	/* If got here, then we process the voxel grid into regions
	 * directly, then triangulate those regions.  This result
	 * is more accurate than the uniform approach, from above,
	 * but may result in some self-intersections on the surface. */

	/* initialize the faces from the dgrid structure */
	tic(clk);
	ret = mesh.init(grid);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	toc(clk, "Initializing voxel face mesh");

	/* for memory savings, clear the grid (we don't need it anymore) */
	grid.clear();

	/* first, remove any disjoint unions in this mesh */
	tic(clk);
	remove_small_unions_faces(mesh, MIN_MESH_UNION_SIZE);
	toc(clk, "Removing small unions");
	
	/* Stage #0 region growing */
	tic(clk);
	ret = mesh.region_flood_fill();
	if(ret)
		return PROPEGATE_ERROR(-2, ret);
	toc(clk, "Flood-fill on mesh");
	
	/* Stage #1 region growing */
	tic(clk);
	ret = mesh.coalesce_regions();
	if(ret)
		return PROPEGATE_ERROR(-3, ret);
	toc(clk, "Coalescing voxel regions");
	
	/* Fix edge cases after region growing */
	tic(clk);
	ret = mesh.reassign_degenerate_regions();
	if(ret)
		return PROPEGATE_ERROR(-4, ret);
	toc(clk, "Reassigning degenerate regions");
	
	/* Stage #2 region growing */
	tic(clk);
	ret = mesh.coalesce_regions_lax();
	if(ret)
		return PROPEGATE_ERROR(-5, ret);
	toc(clk, "Merging parallel regions");

	/* Fix edge cases after region growing */
	tic(clk);
	ret = mesh.reassign_degenerate_regions();
	if(ret)
		return PROPEGATE_ERROR(-6, ret);
	toc(clk, "Reassigning degenerate regions");

	/* Try to merge small regions in this mesh */
	tic(clk);
	ret = mesh.coalesce_regions_small();
	if(ret)
		return PROPEGATE_ERROR(-7, ret);
	toc(clk, "Merging small regions");

	/* Snap vertex positions for each voxel corner */
	tic(clk);
	ret = mesh.compute_verts();
	if(ret)
		return PROPEGATE_ERROR(-8, ret);
	toc(clk, "Computing snapped vertices");

	/* Triangulate each planar region (this will destroy
	 * the mesh data structure, for memory efficiency
	 * reasons) */
	tic(clk);
	ret = triangulate_regions(tri, regions, mesh, grid);
	if(ret)
		return PROPEGATE_ERROR(-9, ret);
	
	/* reduce some artifacts in the triangulation */
	ret = remove_double_surfacing(tri);
	if(ret)
		return PROPEGATE_ERROR(-10, ret);
	prune_invalid_triangles_from_regions(regions, tri);
	
	// TODO: this is commented because it breaks:
	/* ret = tri.map_neighbors();
	 * if(ret)
	 *	return PROPEGATE_ERROR(-11, ret);
	 */
	toc(clk, "Forming triangles on regions");

	/* for visualization purposes, color by region */
	coalesce_small_regions(regions, conf.min_region_area);
	color_by_region(regions);
	
	/* success */
	return 0;
}

bool verts_are_degenerate(vertex_t** verts)
{
	if(!verts[0] || !verts[1] || !verts[2])
		return true;
	return (verts[0] == verts[1]) || (verts[0] == verts[2])
		|| (verts[1] == verts[2]);
}

void organize_boundary_face_verts(vertex_t** verts)
{
	int i, j;
	vertex_t* p;

	/* find index of smallest vert */
	i = 0;
	for(j = 1; j < NUM_VERTS_PER_SQUARE; j++)
		if(verts[j]->hash < verts[i]->hash)
			i = j;

	/* check if we need to rotate.  Only rotate if
	 * the minimum vertex is not an even index */
	if(i % 2 == 0)
		return;

	/* rotate the vertices by one */
	p = verts[0];
	for(j = 0; j < NUM_VERTS_PER_SQUARE-1; j++)
		verts[j] = verts[j+1];
	verts[NUM_VERTS_PER_SQUARE-1] = p;
}

bool are_tris_duplicate(triangle_t* a, triangle_t* b)
{
	int ai, bi;
	bool dup;

	/* iterate through the vertices of a.  For each 
	 * of these, try to find the same vertex in b */
	for(ai = 0; ai < NUM_VERTS_PER_TRI; ai++)
	{
		/* try to see if the vertex ai is duplicated
		 * somewhere in b */
		dup = false;
		for(bi = 0; bi < NUM_VERTS_PER_TRI; bi++)
			if(a->v[ai] == b->v[bi])
			{
				dup = true;
				break;
			}

		/* check if dup found */
		if(!dup)
			return false;
	}

	/* found a duplicate for all vertices */
	return true;
}

/* for the following function, there are two types of vertices
 * that are generated in the triangulation:  Those on the boundary
 * between regions, and those from the quadtree.  These are denoted
 * by the following values */
#define QUADTREE_VERTEX_ID VOXEL_FACE_XMINUS
#define BOUNDARY_VERTEX_ID VOXEL_FACE_YMINUS

/* implementation of helper function defined above */
int triangulate_regions(triangulation_t& tri, 
				vector<planar_region_t>& regions,
					mesher_t& mesh, dgrid_t& grid)
{
	map<voxel_t, vertex_state_t>::iterator vit;
	map<voxelface_t, vertex_t*>::iterator mit;
	set<face_t>::iterator sit;
	vector<quadtri_t>::iterator tit;
	vertex_t* verts[NUM_VERTS_PER_SQUARE];
	triangle_t* tri_elem;
	face_t ff;
	point_t p;
	voxel_t c, pv, w;
	voxelface_t key;
	int ret, f, s, i, r, n;
	double sd, u, v;
	bool b;

	/* iterate over each region */
	n = mesh.regions.size();
	for(r = 0; r < n; r++)
	{
		/* store this planar region */
		regions.push_back(planar_region_t());
		regions.back().avg_norm.x = mesh.regions[r].norm.x;
		regions.back().avg_norm.y = mesh.regions[r].norm.y;
		regions.back().avg_norm.z = mesh.regions[r].norm.z;
		regions.back().avg_pos.x = mesh.regions[r].pos.x * grid.vs;
		regions.back().avg_pos.y = mesh.regions[r].pos.y * grid.vs;
		regions.back().avg_pos.z = mesh.regions[r].pos.z * grid.vs;

		/* check if this region still valid */
		if(mesh.regions[r].faces.empty())
			continue;

		/* When triangulating a region, we want to take advantage
		 * of the fact that the faces are already on a grid, so
		 * find the axis-aligned subspace */
		f = mesh.regions[r].find_dominant_face();

		/* get size of bounding box */
		sd = mesh.regions[r].find_inf_radius();
		if(sd < 0)
			return PROPEGATE_ERROR(-1, (int) sd);
		s = 2 * next_largest_base_2((int) sd);

		/* In order to triangulate this region, we want to
		 * triangulate grid for this maximum dimension.  Create
		 * a quadtree oriented to this direction. */
		quadtree_t tree(s);
		
		/* we are interested in the position of this
		 * face relative to the region's center.  But
		 * to keep everything aligned to the same grid,
		 * set region center to be an integer value on
		 * grid. */
		c.x_ind = (int) round(mesh.regions[r].pos.x);
		c.y_ind = (int) round(mesh.regions[r].pos.y);
		c.z_ind = (int) round(mesh.regions[r].pos.z);

		/* add each face of this region to the quadtree -- but
		 * only if it is not a boundary face and if it is
		 * aligned in the same direction */
		for(sit = mesh.regions[r].faces.begin();
				sit != mesh.regions[r].faces.end();
								sit++)
		{
			/* check if this face is a boundary */
			ff = *sit;
			b = mesh.face_is_boundary(ff);
			if(b)
			{
				/* Write the face as-is to the output */
				for(i = 0; i < NUM_VERTS_PER_SQUARE; i++)
				{
					/* try to find the key for
					 * this vertex by first finding
					 * the voxel location */
					w.x_ind = ff.v.x_ind 
						+ voxel_corner_pos[
						voxel_corner_by_face[ff.f
						][i]][0];
					w.y_ind = ff.v.y_ind 
						+ voxel_corner_pos[
						voxel_corner_by_face[ff.f
						][i]][1];
					w.z_ind = ff.v.z_ind 
						+ voxel_corner_pos[
						voxel_corner_by_face[ff.f
						][i]][2];	
					
					/* get the properties about this
					 * vertex. */
					vit = mesh.verts.find(w);
					if(vit == mesh.verts.end())
						return -2;
					
					/* Even though this vertex is on
					 * a boundary voxel face, it may
					 * not actually be a boundary
					 * vertex, so we want to check
					 * to see which hash scheme needs
					 * to be used */
					if(vit->second.reg_inds.size() <= 1)
					{
						/* use the hash scheme for
						 * non-boundary vertices */
						p.x = w.x_ind - c.x_ind;
						p.y = w.y_ind - c.y_ind;
						p.z = w.z_ind - c.z_ind;
						ret = mesh.
						point_axis_projected_to(u, 
								v, p, f);
						if(ret)
						return PROPEGATE_ERROR(-3, 
								ret);
						key.x_ind = r;
						key.y_ind = u;
						key.z_ind = v;
						key.facenum = 
							QUADTREE_VERTEX_ID;
					}
					else
					{
						/* use the boundary hashing
						 * scheme */
						key.x_ind = w.x_ind;
						key.y_ind = w.y_ind;
						key.z_ind = w.z_ind;
						key.facenum = 
							BOUNDARY_VERTEX_ID;
					}
					
					/* check if we've already calculated
					 * this vertex. */
					mit = tri.vertices.find(key);
					if(mit == tri.vertices.end())
					{
						/* create a new vertex */
						verts[i] = new vertex_t(
							key, grid);
						if(!verts[i])
						{
							PRINT_ERROR("COULD"
							" NOT ALLOCATE"
							"BOUNDARY VERTEX");
						}

						/* Find the correct
						 * position for
						 * this vertex */
						verts[i]->x
							= vit->second.p.x
								* grid.vs;
						verts[i]->y 
							= vit->second.p.y
								* grid.vs;
						verts[i]->z 
							= vit->second.p.z
								* grid.vs;

						/* for debugging purposes,
						 * show the boundary
						 * vertices in red */
						if(key.facenum ==
							BOUNDARY_VERTEX_ID)
						{
							verts[i]->boundary
								= true;
							verts[i]->red 
								= 255;
							verts[i]->green
								= 0;
							verts[i]->blue
								= 0;
						}
					
						/* insert into vertex map */
						tri.vertices.insert(
							pair<voxelface_t,
							vertex_t*>(key, 
							verts[i]));
					}
					else
					{
						/* record the existing
						 * vertex */
						verts[i] = (*mit).second;
					}
				}

				/* prepare to triangulate this face */
				organize_boundary_face_verts(verts);
			
				/* push boundary edges into this region */
				regions.back().add_boundary_edges(verts,
						NUM_VERTS_PER_SQUARE);

				/* add the triangles for these vertices */
				if(!verts_are_degenerate(verts))
				{
					tri_elem = new triangle_t(verts,
						tri.triangles.size());
					if(!tri_elem)
					{
						PRINT_ERROR(
							"COULD NOT ALLOCATE"
							" BOUNDARY TRIANGLE"
							);
					}
					else
					{
						tri_elem->region_id = r;
						tri.triangles.push_back(
								tri_elem);
						regions.back().tris.insert(
								tri_elem);
					}
				}

				/* make second triangle for this face */
				verts[1] = verts[2];
				verts[2] = verts[3];
				if(!verts_are_degenerate(verts))
				{
					tri_elem = new triangle_t(verts,
						tri.triangles.size());
					if(!tri_elem)
					{
						PRINT_ERROR(
							"COULD NOT ALLOCATE"
							"SECOND BOUNDARY "
							"TRIANGLE");
					}
					else
					{
						tri_elem->region_id = r;
						tri.triangles.push_back(
								tri_elem);
						regions.back().tris.insert(
								tri_elem);
					}
				}
			}

			/* ignore if ff not in the same direction as f */
			if(ff.f != f)
				continue;

			/* get the center of this face projected onto
			 * the subspace defined by f */
			ret = ff.get_center(p);
			if(ret)
				return PROPEGATE_ERROR(-4, ret);
			
			/* get position relative to center of region */
			p.x -= c.x_ind;
			p.y -= c.y_ind;
			p.z -= c.z_ind;

			/* get projected 2D coordinates */
			ret = mesh.point_axis_projected_to(u, v, p, f);
			if(ret)
				return PROPEGATE_ERROR(-5, ret);
			
			/* update the quadtree with this location */
			tree.fill_point(u, v, !b);
		}

		/* now that the tree contains the full projection of
		 * the interior of this region, triangulate. */
		tree.triangulate();
		
		/* convert triangulation to 3D */
		for(tit = tree.triangles.begin();
					tit != tree.triangles.end(); tit++)
		{
			/* for each vertex of this triangle, get the
			 * position in 3D space. */
			for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			{	
				/* determine the key of this vertex
				 * in the triangulation */
				key.x_ind = r;
				key.y_ind = (*tit).v[i].x;
				key.z_ind = (*tit).v[i].y;
				key.facenum = QUADTREE_VERTEX_ID;

				/* check if we need to store this
				 * vertex in the triangulation */
				mit = tri.vertices.find(key);
				if(mit == tri.vertices.end())
				{
					/* create this vertex by
					 * projecting the voxel */
				
					/* get position of this vertex */
					ret = mesh.
					undo_point_axis_projection(pv,
							(*tit).v[i].x, 
							(*tit).v[i].y,
								f, c);
					if(ret)
						return PROPEGATE_ERROR(-6,
								ret);
				
					/* undo projection onto plane */
					ret = mesh.undo_plane_projection(p,
								pv, r, f);
					if(ret)
						return PROPEGATE_ERROR(-7,
								ret);

					/* create a new vertex to store
					 * in the triangulation */
					verts[i] = new vertex_t(key, grid);
					if(!verts[i])
					{
						PRINT_ERROR("COULD NOT "
							"ALLOCATE VERTEX");
					}
				
					/* set vertex position */
					verts[i]->x = p.x * grid.vs;
					verts[i]->y = p.y * grid.vs;
					verts[i]->z = p.z * grid.vs;

					/* insert into vertex map */
					tri.vertices.insert(
							pair<voxelface_t,
							vertex_t*>(key, 
							verts[i]));
				}
				else
				{
					/* get the existing vertex */
					verts[i] = (*mit).second;
				}
			}
	
			/* add a triangle with these vertices to the
			 * triangulation structure */
			if(!verts_are_degenerate(verts))
			{
				tri_elem = new triangle_t(verts,
						tri.triangles.size());
				if(!tri_elem)
				{
					PRINT_ERROR("COULD NOT ALLOCATE "
								"TRIANGLE");
				}
				else
				{
					tri_elem->region_id = r;
					tri.triangles.push_back(tri_elem);
					regions.back().tris.insert(
								tri_elem);
				}
			}
		}

		/* at this point, we are done with this region.  To save
		 * memory, clear the elements of the region. */
		mesh.regions[r].faces.clear();
		mesh.regions[r].neighbors.clear();
	}

	/* success */
	return 0;
}

/* This is the implementation of the helper function defined above */
int remove_double_surfacing(triangulation_t& tri)
{
	vector<triangle_t*>::iterator tit;
	map<voxelface_t, vertex_t*>::iterator vit;
	set<triangle_t*> neighs, to_delete;
	set<triangle_t*>::iterator nit;
	normal_t norm, neigh_norm;
	int i;
	double d;

	/* iterate over triangles, and record all triangles
	 * that contain each vertex */
	for(tit = tri.triangles.begin(); tit != tri.triangles.end();
								tit++)
	{
		/* store triangle in each of its vertices */
		for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			(*tit)->v[i]->mytris.push_back(*tit);
	}

	/* Now check all pairs of triangles. If any pair have normals in
	 * opposite directions, tri is an indication of double surfacing
	 * on the mesh. */
	to_delete.clear();
	for(tit = tri.triangles.begin(); tit != tri.triangles.end();
								tit++)
	{
		/* determine the normal for this triangle */
		normal_of_tri(norm, *tit);

		/* initialize list of neighboring triangles */
		neighs.clear();

		/* iterate over vertices, adding neighbors */
		for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			neighs.insert((*tit)->v[i]->mytris.begin(),
					(*tit)->v[i]->mytris.end());

		/* remove self from neighbor list */
		neighs.erase(*tit);

		/* iterate over neighbors, checking angle between norms */
		for(nit = neighs.begin(); nit != neighs.end(); nit++)
		{
			/* check if these are duplicate triangles */
			if(are_tris_duplicate(*tit, *nit))
			{
				to_delete.insert(*tit);
				to_delete.insert(*nit);
				continue;
			}

			/* get normal for neighbor */
			normal_of_tri(neigh_norm, *nit);

			/* check angle between norms */
			d = NORMAL_DOT(norm, neigh_norm);
			if(d < -1 + APPROX_ZERO)
			{
				/* for debugging color their verts */
				for(i = 0; i < NUM_VERTS_PER_TRI; i++)
				{
					(*tit)->v[i]->red = 255;
					(*tit)->v[i]->green = 0;
					(*tit)->v[i]->blue = 255;
					(*nit)->v[i]->red = 255;
					(*nit)->v[i]->green = 0;
					(*nit)->v[i]->blue = 255;
				}
			}
		}
	}

	/* remove the triangles marked for deletion */
	for(i = tri.triangles.size() - 1; i >= 0; i--)
	{
		/* check if i'th triangle should be deleted */
		if(to_delete.count(tri.triangles[i]))
		{
			delete tri.triangles[i];
			tri.triangles.erase(tri.triangles.begin() + i);
		}
	}

	/* success */
	return 0;
}

void coalesce_small_regions(vector<planar_region_t>& rl, 
					double min_surface_area)
{
	vector<triangle_t*>::iterator ti;
	set<triangle_t*>::iterator it;
	set<edge_t>::iterator eit;
	queue<int> small_reg_inds;
	int i, vi, n, ri, ri_best;
	double m, m_best;

	/* verify input */
	if(min_surface_area <= 0)
		return; /* can't do anything */

	/* find all the regions that are too small */
	n = rl.size();
	for(i = 0; i < n; i++)
		if(rl[i].area() < min_surface_area)
			small_reg_inds.push(i);

	/* check for the edge case where every region is too small */
	if(((int) small_reg_inds.size()) >= n)
		return; /* stop now, can't be done */

	/* keep check regions as long as there exists one that 
	 * is too small */
	while(!small_reg_inds.empty())
	{
		/* get index of current region */
		i = small_reg_inds.front();
		small_reg_inds.pop();

		/* find the neighboring region that best supports
		 * the geometry of this region */
		ri_best = -1;
		m_best = -1;
		for(it = rl[i].tris.begin(); it != rl[i].tris.end(); it++)
		{
			/* ignore bad triangles */
			if((*it) == NULL)
				continue;

			/* iterate over vertices of this triangle */
			for(vi = 0; vi < NUM_VERTS_PER_TRI; vi++)
			{
				/* ignore bad vertices */
				if((*it)->v[vi] == NULL)
					continue;

				/* check if vertex is shared by other
				 * regions. */
				if(!((*it)->v[vi]->boundary))
					continue;
			
				/* determine what other regions share
				 * this vertex by iterating over the
				 * triangles that contain it. */
				for(ti = (*it)->v[vi]->mytris.begin(); 
					ti != (*it)->v[vi]->mytris.end(); 
					ti++)
				{
					/* is this triangle part of
					 * another region? */
					ri = (*ti)->region_id;
					if(ri == i || ri < 0 
						|| ri >= (int) rl.size())
						continue;
					
					/* is this region valid? */
					if(rl[ri].tris.size() == 0)
						continue;

					/* does this triangle share an
					 * edge with the query triangle? */
					if(!((*it)->shares_edge_with(*ti)))
						continue;

					/* verify that this other region
					 * is not pointing in the opposite
					 * direction. */
					m = NORMAL_DOT(rl[ri].avg_norm,
						rl[i].avg_norm);
					if(m <= COALESCE_REGIONS_THRESHOLD)
						continue;

					/* is this other region the
					 * biggest we've seen so far? */
					if(m > m_best)
					{
						m_best = m;
						ri_best = ri;
					}
				}
			}
		}

		/* coalesce with largest neighbor */
		if(ri_best < 0 || ri_best == i 
				|| m_best <= COALESCE_REGIONS_THRESHOLD)
			continue;

		/* iterate over triangles of current region, add them
		 * to bigger region */
		for(it = rl[i].tris.begin(); it != rl[i].tris.end(); it++)
		{
			/* update region id */
			(*it)->region_id = ri_best;
			rl[ri_best].tris.insert(*it);
		}

		/* iterate over edges of current region, and add them
		 * to the bigger region */
		for(eit = rl[i].boundary.begin(); 
				eit != rl[i].boundary.end(); eit++)
		{
			/* add this edge to the bigger region.  This will
			 * not only expand its border, but also cancel
			 * out the edges that are in between these
			 * two regions. */
			rl[ri_best].add_boundary_edge(*eit);
		}

		/* update area of new region */
		rl[ri_best].my_area += rl[i].my_area;

		/* clear this region */
		rl[i].tris.clear();
		rl[i].boundary.clear();
		rl[i].my_area = 0;
	}
}
