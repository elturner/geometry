#include "generate_boundary.h"

#include <string>
#include <vector>
#include <map>
#include <set>
#include <float.h>

#include <geometry/system_path.h>
#include <geometry/transform.h>

#include "../structs/cell_graph.h"
#include "../structs/quadtree.h"
#include "../structs/normal.h"
#include "../structs/parameters.h"
#include "../delaunay/insertion.h"
#include "../delaunay/reordering.h"
#include "../delaunay/triangulation/triangulation.h"
#include "../rooms/tri_rep.h"
#include "../util/error_codes.h"
#include "../util/tictoc.h"
#include "../util/constants.h"

using namespace std;

int generate_boundary(cell_graph_t& graph, tri_rep_t& trirep,
				quadtree_t& tree, system_path_t& path,
				bool carve_through)
{
	set<triple_t> interior;
	set<triple_t> visited;
	tictoc_t clk;
	int ret;

	/* create the graph */
	tic(clk);
	ret = graph.populate(tree);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	toc(clk, "Forming graph");

	/* get delaunay triangulation of remaining cells */
	tic(clk);
	ret = triangulate_graph(trirep.tri, graph);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);
	toc(clk, "Triangulating");

	/* determine which triangles are inside geometry, based
	 * on raytracing */
	tic(clk);
	ret = label_triangulation(interior, visited, 
				path, graph, trirep.tri, tree,
				carve_through);
	if(ret)
		return PROPEGATE_ERROR(-3, ret);
	toc(clk, "Carving interior");

	/* label rooms and remove outlier triangles */
	tic(clk);
	trirep.init(interior);
	do
	{
		/* convert the interior triangles to a tri_rep object */
		trirep.find_local_max();
		do
		{
			trirep.reset_roots();
			trirep.flood_rooms();
		}
		while(trirep.unlabel_extra_rooms() > 0);
	
		/* remove unvisited rooms from this structure */
	}
	while(trirep.remove_unvisited_rooms(visited) > 0);
	toc(clk, "Labeling rooms");

	/* add room labels to graph cells */
	tic(clk);
	ret = trirep.add_room_labels_to_graph();
	if(ret < 0)
		return PROPEGATE_ERROR(-4, ret);
	graph.num_rooms = ret;

	/* find boundary edges */
	ret = add_boundary_edges_to_graph(trirep, trirep.tri);
	if(ret)
		return PROPEGATE_ERROR(-5, ret);
	toc(clk, "Generating boundary");

	/* adjust heights based on room labels */
	tic(clk);
	trirep.populate_room_heights();
	toc(clk, "Adjusting room heights");

	/* success */
	return 0;
}

int triangulate_graph(triangulation_t& tri, cell_graph_t& graph)
{
	set<cell_t>::iterator it;
	vertex_t v;
	int i, N, ret;

	/* initialize this triangulation */
	tri_init(&tri);

	/* add the cells as vertices to this triangulation */
	tri.starting_index = 0;
	for(it = graph.V.begin(); it != graph.V.end(); it++)
	{
		/* get current vertex position */
		for(i = 0; i < NUM_DIMS; i++)
			v.pos[i] = (*it).get_data()->average.get(i);
		v.orig_data = (void*) (&(*it));

		/* add vertex to triangulation */
		ret = tri_add_vertex(&tri, &v);
		if(ret < 0)
		{
			tri_cleanup(&tri);
			return PROPEGATE_ERROR(-1, ret);
		}
	}
	N = tri.num_verts;

	/* reorder points to improve insertion efficiency */
	ret = reorder_BRIO(&tri);
	if(ret)
	{
		tri_cleanup(&tri);
		return PROPEGATE_ERROR(-2, ret);
	}

	/* set indices in cells */
	for(i = 1; i <= N; i++)
		((cell_t*) (TRI_VERTEX_POS(&tri, 
				i)->orig_data))->vertex_index = i;

	/* initialize first triangle */
	ret = begin_triangulation(&tri);
	if(ret)
	{
		tri_cleanup(&tri);
		return PROPEGATE_ERROR(-3, ret);
	}

	/* insert the remaining vertices into triangulation.
	 * The first two vertices have already been inserted,
	 * along with the ghost vertex.  Here i indexes from 1,
	 * since the ghost vertex is '0'. */
	for(i = 3; i <= N; i++)
	{
		ret = insert_vertex(&tri, i);
		if(ret)
		{
			tri_cleanup(&tri);
			return PROPEGATE_ERROR(-4, ret);
		}
	}

	/* the triangulation is complete! */
	return 0;
}

int label_triangulation(set<triple_t>& interior, set<triple_t>& visited,
				system_path_t& path,
				cell_graph_t& graph, triangulation_t& tri,
				quadtree_t& tree, bool carve_through)
{
	map<string, transform_t*>::const_iterator tit; 
	pose_t* pose;
	transform_t system2world, sensor2world;
	
	vector< set<cell_t*> > pose_map;
	set<cell_t>::iterator it;
	set<cell_t*>::iterator cit;
	set<unsigned int>::iterator pit;	
	vector<quaddata_t*> xings;
	vector<quaddata_t*>::iterator xit;
	unsigned int i, n;
	vertex_t start, end;
	point_t pp, sensor_point;
	normal_t ray;
	triple_t st, et;
	double d, d_min, min_z, max_z;
	int ret;

	/* check arguments */
	n = path.num_poses();
	if(n == 0)
		return -1;
	interior.clear();
	visited.clear();

	/* get bounds on z from the graph, and use those
	 * to filter invalid poses that are from out-of-bounds */
	graph.compute_height_bounds(min_z, max_z);

	/* iterate over the cells in graph, to populate the pose_map,
	 * which denotes which cells each pose sees */
	pose_map.resize(n);
	for(it = graph.V.begin(); it != graph.V.end(); it++)
	{
		for(pit = (*it).get_data()->pose_inds.begin();
				pit != (*it).get_data()->pose_inds.end();
								pit++)
		{
			/* prepare variables */
			i = *pit;
			if(i >= n)
				continue; /* references non-existant pose */
			pose = path.get_pose(i);
			
			/* check if pose in bounds of this level */
			if(pose->T(2) < min_z || pose->T(2) > max_z)
				continue;

			/* check if pose is in black-listed timezone */
			if(path.is_blacklisted(pose->timestamp))
				continue;

			/* add to map */
			pose_map[i].insert((cell_t*) (&(*it)));
		}
	}

	/* iterate over path */
	st.init(-1,-1,-1);
	for(i = 0; i < n; i++)
	{
		/* check if we care about the current pose */
		if(pose_map[i].empty())
		{
			/* skip this pose, and don't carve the
			 * pose-to-pose line segments */
			continue;
		}
		
		/* get pose information */
		pose = path.get_pose(i);
		system2world.T = pose->T;
		system2world.R = pose->R.toRotationMatrix();

		/* get position of i'th pose and vertex and as point */
		vertex_set(&start, pose->T(0), pose->T(1));
		pp.set(0, pose->T(0));
		pp.set(1, pose->T(1));

		/* iterate over the cells seen by this pose */
		for(cit = pose_map[i].begin(); cit != pose_map[i].end(); 
								cit++)
		{
			/* get cell pos as vertex end point */
			vertex_set(&end, (*cit)->get_data()->average.get(0),
					(*cit)->get_data()->average.get(1));
			
			/* prepare to ray trace through quadtree, to
			 * determine if any occlusions occur */
			xings.clear();

			/* perform ray-tracing from pose to cell */ 
			tree.raytrace(xings,pp,(*cit)->get_data()->average);
		
			/* Check if extrinsics were provided for the
			 * sensors.  If they were, then we should trace
			 * from all available positions on the backpack,
			 * not just the center position. 
			 *
			 * This is important to try to avoid "over-carving",
			 * where an occluding object is missed because
			 * we carved from the wrong spot. */
			for(tit = path.begin_transforms(); 
					tit != path.end_transforms(); tit++)
			{
				/* get the world-position of each scanner at
				 * this pose */
				sensor2world = *(tit->second);
				sensor2world.cat(system2world);

				/* convert transform to a point we can
				 * trace with */
				sensor_point.set(0, sensor2world.T(0));
				sensor_point.set(1, sensor2world.T(1));

				/* trace from this point to the wall
				 * sample, in order to check from additional
				 * occlusions from this angle */
				tree.raytrace(xings,sensor_point,
					(*cit)->get_data()->average);
			}

			/* get line-of-sight for this trace */
			if(carve_through)
			{
				/* set end-point to be original cell */
				vertex_set(&end,
					(*cit)->get_data()->average.get(0), 
					(*cit)->get_data()->average.get(1));
			}
			else
			{
				/* get the occluding cell that is closest to
				 * the origin of the ray (i.e. the pose) */
				d_min = DBL_MAX;
				for(xit = xings.begin(); xit != xings.end();
				                         xit++)
				{
					/* check distance */
					d = pp.dist_sq((*xit)->average);
					if(d >= d_min)
						continue;
					
					/* compute normalized direction 
					 * of this ray */
					ray.dir(pp, (*xit)->average);
				
					/* set the endpoint vertex to be 
					 * the location of this closer 
					 * cell.
					 *
					 * Since we want the ray
					 * tracing to be inclusive at
					 * the start but exclusive
					 * at the end, subtract some
					 * epsilon distance from the ray
					 */
					vertex_set(&end, 
						(*xit)->average.get(0)
						- ray.get(0)*APPROX_ZERO, 
					        (*xit)->average.get(1)
						- ray.get(1)*APPROX_ZERO);
					d_min = d;
				}
			}
			
			/* perform ray-tracing through 
			 * triangulation */
			ret = raytrace_triangulation(interior, tri,
						start, end, st, et);
			if(ret)
				return PROPEGATE_ERROR(-3, ret);
		}

		/* next we update to the next pose, so skip this
		 * step for the end of the path */
		if(i+1 >= n)
			continue;
		if(pose_map[i+1].empty())
			continue;
		pose = path.get_pose(i+1);

		/* ray-trace path from i to i+1 */
		vertex_set(&end, pose->T(0), pose->T(1));
		ret = raytrace_triangulation(interior, tri,
				start, end, st, et);
		if(ret)
			return PROPEGATE_ERROR(-4, ret);

		/* update start triangle */
		visited.insert(st);
		st = et;
	}

	/* success */
	return 0;
}

int raytrace_triangulation(set<triple_t>& found_tris, triangulation_t& tri,
					vertex_t& start, vertex_t& end,
					triple_t& st, triple_t& et)
{
	bool valid_start, reached_end;
	int ret, i;
	linkring_t* lrt;
	unsigned int stemp, sa_old, sb_old;
	unsigned int s0, s1, s2;
	
	/* get proposed starting triangle */
	s0 = st.i;
	s1 = st.j;
	s2 = st.k;

	/* determine if given valid starting triangle */
	valid_start = false;
	if(s0 <= tri.num_verts && s0 > 0)
	{
		lrt = tri.links + s0;
		i = linkring_find(lrt, s1);
		if(i >= 0 && LINKRING_NEXT_VAL(lrt, i) == s2)
			valid_start = true;
	}

	/* if not given a valid starting triangle, find the
	 * correct triangle */
	if(!valid_start)
	{
		ret = tri_locate(&tri, &start, tri.num_verts+1, 0, 0,
					&s0, &s1, &s2);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);

		/* save this triangle */
		st.init(s0, s1, s2);
	}

	/* Next, traverse the graph from triangle to triangle to
	 * find v */
	sa_old = sb_old = 0;
	reached_end = false;
	while(!reached_end)
	{
		/* add current triangle to set of found triangles, but
		 * only if it does not contain the ghost vertex */
		if(s0 != GHOST_VERTEX && s1 != GHOST_VERTEX 
				&& s2 != GHOST_VERTEX)
			found_tris.insert(triple_t(s0,s1,s2));
		
		/* get directions from current triangle to destination */
		i = tri_get_directions(&tri, &start, &end, s0, s1, s2);
	
		/* i will tell which edge of triangle to cross
		 * in order to move closer to vertex v.  For
		 * a given value of i, keep the other two
		 * corners of the triangle, and use the triangle
		 * whose third corner is the apex of the kept
		 * vertices */
		switch(i)
		{
			default:
				return PROPEGATE_ERROR(-2, i);
			
			case 3:
				/* current triangle contains end */
				reached_end = true;
				break;
			
			case 0:
				i = tri_get_apex(&tri,s2,s1);
				if(i < 0)
					return PROPEGATE_ERROR(-3, i);

				/* check if we're looping */
				if(sa_old == s1 && sb_old == s2)
				{
					PRINT_ERROR("LOOPING!");
					et.init(s0,s1,s2);
					return 0;
				}
				sa_old = s2;
				sb_old = s1;

				/* cross this edge */
				s0 = i;
				stemp = s1;
				s1 = s2;
				s2 = stemp;
				break;
		
			case 1:
				i = tri_get_apex(&tri,s0,s2);
				if(i < 0)
					return PROPEGATE_ERROR(-4, i);

				/* check if we're looping */
				if(sa_old == s2 && sb_old == s0)
				{
					PRINT_ERROR("LOOPING!");
					et.init(s0,s1,s2);
					return 0;
				}
				sa_old = s0;
				sb_old = s2;

				/* cross this edge */
				s1 = i;
				stemp = s0;
				s0 = s2;
				s2 = stemp;
				break;
			
			case 2:
				i = tri_get_apex(&tri,s1,s0);
				if(i < 0)
					return PROPEGATE_ERROR(-5, i);

				/* check if we're looping */
				if(sa_old == s0 && sb_old == s1)
				{
					PRINT_ERROR("LOOPING!");
					et.init(s0,s1,s2);
					return 0;
				}
				sa_old = s1;
				sb_old = s0;

				/* cross this edge */
				s2 = i;
				stemp = s1;
				s1 = s0;
				s0 = stemp;
				break;
		}
	}

	/* save final triangle to given pointers */
	et.init(s0,s1,s2);
	return 0;
}

int add_boundary_edges_to_graph(tri_rep_t& trirep,
					triangulation_t& tri)
{
	unsigned int vi, j, vj, n, m;
	linkring_t* lrt;
	cell_t* ci, *cj;
	triple_t t1, t2;
	int vk;

	/* iterate through vertices of triangulation */
	n = tri.num_verts;
	for(vi = 1; vi <= n; vi++)
	{
		/* get linkring of this vertex */
		lrt = TRI_GET_LINKRING(&tri, vi);
		if(lrt == NULL)
			return -1;

		/* iterate through neighboring vertices */
		m = lrt->len;
		for(j = 0; j < m; j++)
		{
			/* get vertex at this position in the link ring */
			vj = lrt->vertices[j];
		
			/* we only care about this edge in one direction,
			 * as to avoid duplicates */
			if(vi > vj)
				continue;

			/* check the triangles on either side of this
			 * edge.  If one is interior and one is exterior,
			 * then this edge is a boundary */
			vk = tri_get_apex(&tri, vi, vj);
			if(vk < 0)
				return PROPEGATE_ERROR(-2, vk);
			t1.init(vi, vj, vk);

			vk = tri_get_apex(&tri, vj, vi);
			if(vk < 0)
				return PROPEGATE_ERROR(-3, vk);
			t2.init(vi, vj, vk);

			/* check if t1 and t2 have different labels */
			if(trirep.contains(t1) == trirep.contains(t2))
				continue;
			
			/* boundary edge, add to graph */
			ci = (cell_t*) (TRI_VERTEX_POS(&tri,vi)->orig_data);
			cj = (cell_t*) (TRI_VERTEX_POS(&tri,vj)->orig_data);
			ci->add_edge(cj);
		}
	}

	/* success */
	return 0;
}
