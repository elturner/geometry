#include "mesher.h"
#include <fstream>
#include <map>
#include <set>
#include <queue>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "dgrid.h"
#include "../math/mathlib.h"
#include "../math/eigenwrapper.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"
#include "../util/progress_bar.h"

using namespace std;

/******* FACE_T FUNCTIONS **********/

face_t::face_t()
{
	/* store default values */
	this->v.x_ind = 0;
	this->v.y_ind = 0;
	this->v.z_ind = 0;
	this->f = -1;
}

face_t::face_t(voxel_t& vv, int ff)
{
	/* copy over info.  Since faces
	 * can be described by either voxel
	 * the intersect, it is assumed that
	 * vv is the voxel on the outside of the
	 * mesh */
			
	/* record as-is */
	this->v.x_ind = vv.x_ind;
	this->v.y_ind = vv.y_ind;
	this->v.z_ind = vv.z_ind;
	this->f = ff;
}

face_t::~face_t() {}
	
void face_t::copy(face_t& other)
{
	this->v.x_ind = other.v.x_ind;
	this->v.y_ind = other.v.y_ind;
	this->v.z_ind = other.v.z_ind;
	this->f = other.f;
}
	
void face_t::swap(face_t& other)
{
	int t;

	t = other.v.x_ind;
	other.v.x_ind = this->v.x_ind;
	this->v.x_ind = t;

	t = other.v.y_ind;
	other.v.y_ind = this->v.y_ind;
	this->v.y_ind = t;

	t = other.v.z_ind;
	other.v.z_ind = this->v.z_ind;
	this->v.z_ind = t;

	t = other.f;
	other.f = this->f;
	this->f = t;
}
	
int face_t::get_center(point_t& p) const
{
	/* the center depends on which direction
	 * this face is oriented. */
	switch(this->f)
	{
		case VOXEL_FACE_XMINUS:
			p.x = this->v.x_ind;
			p.y = this->v.y_ind + 0.5;
			p.z = this->v.z_ind + 0.5;
			return 0;

		case VOXEL_FACE_XPLUS:
			p.x = this->v.x_ind + 1;
			p.y = this->v.y_ind + 0.5;
			p.z = this->v.z_ind + 0.5;
			return 0;

		case VOXEL_FACE_YMINUS:
			p.x = this->v.x_ind + 0.5;
			p.y = this->v.y_ind;
			p.z = this->v.z_ind + 0.5;
			return 0;
		
		case VOXEL_FACE_YPLUS:
			p.x = this->v.x_ind + 0.5;
			p.y = this->v.y_ind + 1;
			p.z = this->v.z_ind + 0.5;
			return 0;
		
		case VOXEL_FACE_ZMINUS:
			p.x = this->v.x_ind + 0.5;
			p.y = this->v.y_ind + 0.5;
			p.z = this->v.z_ind;
			return 0;
		
		case VOXEL_FACE_ZPLUS:
			p.x = this->v.x_ind + 0.5;
			p.y = this->v.y_ind + 0.5;
			p.z = this->v.z_ind + 1;
			return 0;
		
		default:
			/* an error occurred */
			return -1;
	}
}
	
bool face_t::faces_outward(normal_t& n)
{
	/* evaluation is entirely dependant on the
	 * face number of this face_t */
	switch(this->f)
	{
		case VOXEL_FACE_XMINUS:
			return (n.x < 0);
		case VOXEL_FACE_XPLUS:
			return (n.x > 0);
		case VOXEL_FACE_YMINUS:
			return (n.y < 0);
		case VOXEL_FACE_YPLUS:
			return (n.y > 0);
		case VOXEL_FACE_ZMINUS:
			return (n.z < 0);
		case VOXEL_FACE_ZPLUS:
			return (n.z > 0);
		default:
			/* an error occurred */
			return false;
	}
}

/******** FACESTATE_T FUNCTIONS ***********/

face_state_t::face_state_t()
{
	/* set to default values */
	this->init();
}

face_state_t::~face_state_t() {}

void face_state_t::init()
{
	int i;

	/* assume no valid region yet */
	this->region_id = -1;

	/* set the four neighboring faces to
	 * be invalid. */
	for(i = 0; i < NUM_EDGES_PER_SQUARE; i++)
		this->neighbors[i].f = -1;
}

/********* VERTEX_STATE_T FUNCTIONS ********/

vertex_state_t::vertex_state_t(voxel_t& v)
{
	/* set the position of this vertex to be the
	 * same as the voxel bottom corner */
	this->p.x = v.x_ind;
	this->p.y = v.y_ind;
	this->p.z = v.z_ind;

	/* currently no regions defined */
	reg_inds.clear();
}

vertex_state_t::~vertex_state_t()
{
	reg_inds.clear();
}

/********* REGION_T FUNCTIONS ***********/

region_t::region_t(face_t& seed)
{
	/* add the seed to the set of faces */
	this->faces.clear();
	this->faces.insert(seed);

	/* initialize the norm and pos to represent this face */
	switch(seed.f)
	{
		case VOXEL_FACE_XMINUS:
			this->norm.x = -1;
			this->norm.y = 0;
			this->norm.z = 0;
			this->pos.x = seed.v.x_ind;
			this->pos.y = seed.v.y_ind;
			this->pos.z = seed.v.z_ind;
			break;
		case VOXEL_FACE_XPLUS:
			this->norm.x = 1;
			this->norm.y = 0;
			this->norm.z = 0;
			this->pos.x = seed.v.x_ind+1;
			this->pos.y = seed.v.y_ind+1;
			this->pos.z = seed.v.z_ind+1;
			break;
		case VOXEL_FACE_YMINUS:
			this->norm.x = 0;
			this->norm.y = -1;
			this->norm.z = 0;
			this->pos.x = seed.v.x_ind;
			this->pos.y = seed.v.y_ind;
			this->pos.z = seed.v.z_ind;
			break;
		case VOXEL_FACE_YPLUS:
			this->norm.x = 0;
			this->norm.y = 1;
			this->norm.z = 0;
			this->pos.x = seed.v.x_ind+1;
			this->pos.y = seed.v.y_ind+1;
			this->pos.z = seed.v.z_ind+1;
			break;
		case VOXEL_FACE_ZMINUS:
			this->norm.x = 0;
			this->norm.y = 0;
			this->norm.z = -1;
			this->pos.x = seed.v.x_ind;
			this->pos.y = seed.v.y_ind;
			this->pos.z = seed.v.z_ind;
			break;
		case VOXEL_FACE_ZPLUS:
			this->norm.x = 0;
			this->norm.y = 0;
			this->norm.z = 1;
			this->pos.x = seed.v.x_ind+1;
			this->pos.y = seed.v.y_ind+1;
			this->pos.z = seed.v.z_ind+1;
			break;
	}

	/* currently no error, since singleton face is planar */
	this->max_err = 0;

	/* no neighbors yet */
	this->neighbors.clear();
}

region_t::~region_t() {}
	
int region_t::find_center()
{
	set<face_t>::iterator it;
	point_t p;
	int ret, num_faces;

	/* reset center pos position */
	this->pos.x = 0;
	this->pos.y = 0;
	this->pos.z = 0;
	num_faces = 0;

	/* iterate over each face in the region,
	 * find the average of the centers of all faces,
	 * weighting each face the same amount */
	for(it = this->faces.begin(); it != this->faces.end(); it++)
	{
		/* get center of current face */
		ret = (*it).get_center(p);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);

		/* add to average */
		this->pos.x += p.x;
		this->pos.y += p.y;
		this->pos.z += p.z;
		num_faces++;
	}

	/* check edge case */
	if(num_faces == 0)
		return -2;

	/* compute average */
	this->pos.x /= num_faces;
	this->pos.y /= num_faces;
	this->pos.z /= num_faces;

	/* success */
	return 0;
}

int region_t::find_dominant_face()
{
	double ax, ay, az;

	/* get the magnitude of projection in each dimension */
	ax = fabs( this->norm.x );
	ay = fabs( this->norm.y );
	az = fabs( this->norm.z );

	/* find principle direction */
	if( ax > ay && ax > az )
	{
		/* X dimension is principle component */
		if( this->norm.x > 0 )
			return VOXEL_FACE_XPLUS;
		return VOXEL_FACE_XMINUS;
	}
	else if( ay > az )
	{
		/* Y dimension is principle component */
		if( this->norm.y > 0)
			return VOXEL_FACE_YPLUS;
		return VOXEL_FACE_YMINUS;
	}

	/* if got here, then Z is principle component */
	if( this->norm.z > 0 )
		return VOXEL_FACE_ZPLUS;
	return VOXEL_FACE_ZMINUS;
}
	
void region_t::verify_normal()
{
	set<face_t>::iterator sit;
	int num_aligned, majority;
	face_t f;

	/* iterate over all faces in this region,
	 * counting how many faces are aligned with
	 * the current normal vector */
	num_aligned = 0;
	for(sit = this->faces.begin(); sit != this->faces.end(); sit++)
	{	
		f = *sit;
		if(f.faces_outward(this->norm))
			num_aligned++;
	}

	/* check if we should flip */
	majority = 1 + this->faces.size()/2;
	if(num_aligned < majority)
	{
		/* flip each component */
		this->norm.x *= -1;
		this->norm.y *= -1;
		this->norm.z *= -1;
	}
}

double region_t::find_inf_radius()
{
	set<face_t>::iterator sit;
	point_t c;
	double r, x, y, z;
	int ret;

	/* set initial guess of radius */
	r = 0;

	/* iterate over all faces */
	for(sit = this->faces.begin(); sit != this->faces.end(); sit++)
	{
		/* get center of the current face */
		ret = (*sit).get_center(c);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);

		/* compute abs. diff in each dimension */
		x = fabs(c.x - this->pos.x);
		y = fabs(c.y - this->pos.y);
		z = fabs(c.z - this->pos.z);
		if(r < x)
			r = x;
		if(r < y)
			r = y;
		if(r < z)
			r = z;
	}

	/* success */
	return r;
}

double region_t::height_of_point(point_t& p)
{
	normal_t q;

	/* get displacement */
	q.x = p.x - this->pos.x;
	q.y = p.y - this->pos.y;
	q.z = p.z - this->pos.z;
	
	/* return the height */
	return NORMAL_DOT(this->norm, q);
}

double region_t::height_of_voxel(voxel_t& v)
{
	normal_t q;

	/* get displacement */
	q.x = v.x_ind - this->pos.x;
	q.y = v.y_ind - this->pos.y;
	q.z = v.z_ind - this->pos.z;

	/* return the height */
	return NORMAL_DOT(this->norm, q);
}

/********* MESHER_T FUNCTIONS ***********/

mesher_t::mesher_t()
{
	/* empty graph */
	this->graph.clear();
	this->regions.clear();
	this->verts.clear();
}

mesher_t::~mesher_t() {}

int mesher_t::init(dgrid_t& dg)
{
	map<voxel_t, voxel_state_t>::iterator it;
	map<face_t, face_state_t>::iterator git;
	voxel_t v;
	voxel_state_t s;
	face_t ff;
	int f, ret;
	
	/* Add each voxel face from the dgrid
	 * into this data structure.
	 *
	 * Iterate over each voxel in dg */
	for(it = dg.voxels.begin(); it != dg.voxels.end(); it++)
	{
		/* get the state of this voxel */
		v = (*it).first;
		s = (*it).second;

		/* iterate over the faces of the current voxel */
		for(f = 0; f < NUM_FACES_PER_CUBE; f++)
			if(VOXEL_IS_FACE_BIT_INWARD(s,f))
			{
				/* add this face to the structure */
				this->graph[ face_t(v, f) ] 
						= face_state_t();
			}
	}

	/* Now that all the faces have been inserted into the graph,
	 * form the connections between neighboring faces */
	for(git = this->graph.begin(); git != this->graph.end(); git++)
	{
		/* get the face */
		ff = (*git).first;
		
		/* find the neighbors of the current face */
		ret = this->find_neighbors_for(ff);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
	}

	/* success */
	return 0;
}
	
int mesher_t::region_flood_fill()
{
	map<face_t, face_state_t>::iterator it, it2;
	face_t f, g;
	queue<face_t> flooder;
	int i, r, n, ret;

	/* iterate through all faces */
	for(it = this->graph.begin(); it != this->graph.end(); it++)
	{
		/* get the current face */
		f = (*it).first;

		/* if this face is not yet part of a region,
		 * start a new region by flood filling from this seed */
		if((*it).second.region_id >= 0)
			continue;

		/* start a new region and a queue for flood filling */
		this->regions.push_back(region_t(f));
		r = this->regions.size() - 1; /* current region index */
		flooder.push(f);

		/* keep flooding until the queue is empty */
		while(!flooder.empty())
		{
			/* get the next face */
			g = flooder.front();
			flooder.pop();

			/* check if g already assigned to current region */
			it2 = this->graph.find(g);
			if(it2 == this->graph.end())
				return -1;
			if(it2->second.region_id == r)
				continue; /* already part of region */

			/* add this face to the current region */
			it2->second.region_id = r;
			this->regions[r].faces.insert(g);

			/* iterate over neighbors of g, check if they
			 * are in the same plane */
			for(i = 0; i < NUM_EDGES_PER_SQUARE; i++)
				if(it2->second.neighbors[i].f == g.f)
					flooder.push(
						it2->second.neighbors[i]);
		}
	}

	/* now, iterate through all the regions that were found, and
	 * finalize their processing */
	n = this->regions.size();
	for(r = 0; r < n; r++)
	{
		/* compute the set of neighboring regions
		 * for this region */
		ret = this->compute_neighbors_of(r);
		if(ret)
			return PROPEGATE_ERROR(-3, ret);

		/* find the center of this region */
		ret = this->regions[r].find_center();
		if(ret)
			return PROPEGATE_ERROR(-4, ret);
	}

	/* success */
	return 0;
}

int mesher_t::coalesce_regions()
{
	set<int>::iterator it;
	set< pair<int,int> > flexible_regions; /* <size, id> */
	int r, ro, n, ret, orig_pb, next_pb, delta_pb;
	double e, e_best, d;
	normal_t norm, norm_best;
	point_t p, p_best;

	/* initialize flexible region indices */
	n = this->regions.size();
	for(r = 0; r < n; r++)
		flexible_regions.insert(make_pair<int, int>(
				this->regions[r].neighbors.size(), r));

	/* user (aka Eric) wants a progress bar */
	reserve_progress_bar();
	next_pb = flexible_regions.size();
	orig_pb = next_pb;
	delta_pb = next_pb / 200;

	/* keep coalescing while there are available regions */
	while(!flexible_regions.empty())
	{
		/* user is impatient, show progress bar occasionally */
		if((int) flexible_regions.size() <= next_pb)
		{
			progress_bar("coalescing",
				((double) (orig_pb - next_pb)) / orig_pb);
			next_pb -= delta_pb;
		}

		/* get the first region to examine */
		r = flexible_regions.begin()->second;
		
		/* check edge case of empty region */
		if(this->regions[r].faces.empty())
		{
			flexible_regions.erase(flexible_regions.begin());
			continue;
		}

		/* verify we don't have degenerate neighbors */
		this->regions[r].neighbors.erase(r);

		/* find the neighbor who will coalesce with the
		 * minimum error */
		ro = -1;
		e = e_best = DBL_MAX;
		p_best.x = 0; p_best.y = 0; p_best.z = 0;
		norm_best.x = 0; norm_best.y = 0; norm_best.z = 0;
		for(it = this->regions[r].neighbors.begin();
				it != this->regions[r].neighbors.end();
					it++)
		{
			/* check if the regions aren't facing
			 * opposite directions. */
			d = NORMAL_DOT(this->regions[r].norm,
						this->regions[*it].norm);
			if(d < -PERPENDICULAR_THRESHOLD)
				continue; /* parallel but opposite */

			/* get coalesce properties for this neighbor */
			ret = this->find_combined_properties(
					this->regions[r], 
					this->regions[*it], p, norm, e);
			if(ret)
				return PROPEGATE_ERROR(-1, ret);

			/* check against best found so far */
			if(e < e_best)
			{
				/* record information for best neighbor */
				ro = *it;
				e_best = e;
				p_best.x = p.x;
				p_best.y = p.y;
				p_best.z = p.z;
				norm_best.x = norm.x;
				norm_best.y = norm.y;
				norm_best.z = norm.z;
			
				/* check if we've found one that is
				 * good enough. */
				if(e_best < VOXEL_FACE_MAX_ERR_THRESHOLD)
					break;
			}
		}

		/* verify that best error is within our tolerances */
		if(e_best >= VOXEL_FACE_MAX_ERR_THRESHOLD || ro < 0) 
		{
			/* if not, then remove current index from list */
			flexible_regions.erase(flexible_regions.begin());
			continue;
		}

		/* merge these two regions */
		ret = merge_regions(r, ro, p_best, norm_best, e_best);
		if(ret < 0)
			return PROPEGATE_ERROR(-2, ret);
	
		/* verify that the normal vector is pointed in
		 * the correct direction. */
		this->regions[ret].verify_normal();
	}

	/* success */
	delete_progress_bar();
	return 0;
}

int mesher_t::coalesce_regions_lax()
{
	set<int>::iterator it;
	set< pair<int,int> > flexible_regions; /* <size, id> */
	int r, ro, n, ret;
	double e, d, d_best;
	normal_t norm;
	point_t p;

	/* initialize flexible region indices */
	n = this->regions.size();
	for(r = 0; r < n; r++)
		flexible_regions.insert(make_pair<int, int>(
				this->regions[r].neighbors.size(), r));

	/* keep coalescing while there are available regions */
	while(!flexible_regions.empty())
	{
		/* get the first region to examine */
		r = flexible_regions.begin()->second;
		
		/* check edge case of empty region */
		if(this->regions[r].faces.empty())
		{
			flexible_regions.erase(flexible_regions.begin());
			continue;
		}

		/* verify we don't have degenerate neighbors */
		this->regions[r].neighbors.erase(r);

		/* find the neighbor who will coalesce with the
		 * minimum error */
		ro = -1;
		d_best = -1;
		for(it = this->regions[r].neighbors.begin();
				it != this->regions[r].neighbors.end();
					it++)
		{
			/* check how parallel the current neighbor is
			 * to region r */
			d = NORMAL_DOT(this->regions[r].norm,
						this->regions[*it].norm);

			/* check against best found so far */
			if(d > d_best)
			{
				/* record information for best neighbor */
				ro = *it;
				d_best = d;
			}
		}

		/* verify that ro is within our tolerances */
		if(d_best < PARALLEL_THRESHOLD || ro < 0) 
		{
			/* if not, then remove current index from list */
			flexible_regions.erase(flexible_regions.begin());
			continue;
		}
		
		/* get coalesce properties for this neighbor */
		ret = this->find_combined_properties(this->regions[r], 
					this->regions[ro], p, norm, e);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);

		/* merge these two regions */
		ret = merge_regions(r, ro, p, norm, e);
		if(ret < 0)
			return PROPEGATE_ERROR(-2, ret);
	
		/* verify the new region */
		this->regions[ret].verify_normal();
	}

	/* success */
	return 0;
}

int mesher_t::reassign_degenerate_regions()
{
	map<face_t, face_state_t>::iterator git;
	set<face_t>::iterator sit;
	queue<face_t> faces_to_check;
	face_t f;
	int i, r, ro, n;

	/* initially check all faces */
	for(git = this->graph.begin(); git != this->graph.end(); git++)
	{
		f = git->first;
		if(face_is_degenerate(f) >= 0)
			faces_to_check.push(git->first);
	}

	/* keep reassigning as long as the queue is non-empty */
	while(!(faces_to_check.empty()))
	{
		/* pop the next face */
		f = faces_to_check.front();
		faces_to_check.pop();

		/* check if it is degenerate */
		r = face_is_degenerate(f);
		if(r < 0)
			continue;

		/* move face to region r */
		git = this->graph.find(f);
		if(git == this->graph.end())
			return -1;

		this->regions[git->second.region_id].faces.erase(f);
		git->second.region_id = r;
		this->regions[r].faces.insert(f);

		/* now that we've changed region associations,
		 * check the neighbors of this face */
		for(i = 0; i < NUM_EDGES_PER_SQUARE; i++)
			faces_to_check.push(git->second.neighbors[i]);
	}

	/* next, iterate through regions, checking if any are
	 * degenerate.  This includes regions that are completely
	 * surrounded by another region. */
	n = this->regions.size();
	for(r = 0; r < n; r++)
	{
		/* check the number of neighboring regions to rit */
		if(this->regions[r].neighbors.size() == 1)
		{
			/* move this region's elements into the other */
			ro = *(this->regions[r].neighbors.begin());
			this->regions[ro].faces.insert(
					this->regions[r].faces.begin(),
					this->regions[r].faces.end());
			
			/* reassign each face */
			for(sit = this->regions[r].faces.begin();
				sit != this->regions[r].faces.end();
						sit++)
			{
				/* get face state */
				git = this->graph.find(*sit);
				if(git == this->graph.end())
					return -2;

				/* reassign region id */
				git->second.region_id = ro;
			}

			/* clear out this region */
			this->regions[r].faces.clear();
			this->regions[r].neighbors.clear();
			
			/* delete from neighbor's records */
			this->regions[ro].neighbors.erase(r);
		}
	}

	/* success */
	return 0;
}

int mesher_t::coalesce_regions_small()
{
	set<int>::iterator nit, it;
	int r, ro, n, ret;
	bool is_critical;
	double d, d_best;
	
	/* iterate over all regions.  Given a "small" region,
	 * will attempt to join it with its larger neighbors. */
	n = this->regions.size();
	for(r = 0; r < n; r++)
	{
		/* check the size of this region */
		if(this->regions[r].faces.size() >= MIN_SNAP_REGION_SIZE)
			continue;

		/* this region is small enough, but we want to make sure
		 * it is not a critical feature.  This can be determined
		 * by checking that all of r's neighbors can see all of
		 * its other neighbors.
		 *
		 * First, iterate over all of r's neighbors */
		is_critical = false;
		for(nit = this->regions[r].neighbors.begin();
				!is_critical &&
				nit != this->regions[r].neighbors.end();
						nit++)
		{
			/* get this neighbor */
			ro = *nit;

			/* check if this neighbor has all of 
			 * r's neighbors */
			for(it = this->regions[r].neighbors.begin();
				!is_critical &&
				it != this->regions[r].neighbors.end();
					it++)
			{
				/* ignore edge case */
				if(*it == ro || *it == r)
					continue;

				/* check if ro has the neighbor *it */
				is_critical |= !(this->regions[ro
						].neighbors.count(*it));
			}
		}

		/* check if this region is critical */
		if(is_critical)
			continue;

		/* we can merge this region, so find the best neighbor
		 * with which to merge r */
		ro = -1;
		d_best = -DBL_MAX;
		for(nit = this->regions[r].neighbors.begin();
				nit != this->regions[r].neighbors.end();
						nit++)
		{
			/* make sure region is big enough */
			if(this->regions[*nit].faces.size() 
						< MIN_SNAP_REGION_SIZE)
				continue;

			/* get the angular difference between regions */
			d = NORMAL_DOT(this->regions[r].norm,
						this->regions[*nit].norm);
			if(d > d_best)
			{
				d_best = d;
				ro = *nit;
			}
		}

		/* merge region r into region ro */
		if(ro < 0)
			continue;
		ret = this->merge_regions(r, ro, this->regions[ro].pos,
						this->regions[ro].norm,
						this->regions[ro].max_err);
		if(ret < 0)
			return PROPEGATE_ERROR(-1, ret);
	}

	/* success */
	return 0;
}

int mesher_t::reassign_boundary_faces()
{
	map<face_t, face_state_t>::iterator git, oit;
	set<face_t>::iterator sit;
	queue<map<face_t, face_state_t>::iterator> faces_to_check;
	face_t f;
	point_t p;
	int ret, i, r, ro, r_best;
	double e, e_best;

	/* first, populate queue with all boundary faces */
	for(git = this->graph.begin(); git != this->graph.end(); git++)
	{
		/* get my info */
		r = git->second.region_id;

		/* get info of neighbors */
		for(i = 0; i < NUM_EDGES_PER_SQUARE; i++)
		{
			/* get neighbor info */
			oit = this->graph.find(git->second.neighbors[i]);
			if(oit == this->graph.end())
				continue;
	
			/* check neighbor region compared to ours */
			if(oit->second.region_id != r)
			{
				/* ah ha! Current face is boundary */
				faces_to_check.push(git);
				break;
			}
		}
	}

	/* now that we have all boundary faces, check each face
	 * in the queue to see if it would fit better with one of the
	 * neighboring regions. */
	while(!(faces_to_check.empty()))
	{
		/* get next face */
		git = faces_to_check.front();
		faces_to_check.pop();
		
		/* get characteristics about this face */
		r = git->second.region_id;
		ret = git->first.get_center(p);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);

		/* get error from current region */
		r_best = r;
		e_best = fabs(height_from_plane(p, this->regions[r].norm,
						this->regions[r].pos));

		/* check regions of the neighboring faces */
		for(i = 0; i < NUM_EDGES_PER_SQUARE; i++)
		{
			/* get neighbor info */
			oit = this->graph.find(git->second.neighbors[i]);
			if(oit == this->graph.end())
				continue;
	
			/* check neighbor region compared to ours */
			ro = oit->second.region_id;
			if(ro != r)
			{
				/* check error from this other region */
				e = fabs(height_from_plane(p, 
						this->regions[ro].norm,
						this->regions[ro].pos));
				if(e < e_best)
				{
					r_best = ro;
					e_best = e;
				}
			}
		}

		/* check if we need to change this face */
		if(r_best == r)
			continue;
	
		/* move this face into the other region */
		git->second.region_id = r_best;
		this->regions[r].faces.erase(git->first);
		this->regions[r_best].faces.insert(git->first);

		// TODO may need to edit region neighbor sets

		/* since we made a change to this face, recheck its
		 * neighbors */
		for(i = 0; i < NUM_EDGES_PER_SQUARE; i++)
		{
			/* get neighbor info */
			oit = this->graph.find(git->second.neighbors[i]);
			if(oit == this->graph.end())
				continue;
		
			/* add to queue */
			faces_to_check.push(oit);
		}
	}

	// TODO, to make this K-means, put above in a larger loop, which
	// reevaluates the plane equations for each region

	/* success */
	return 0;
}

int mesher_t::compute_verts()
{
	map<face_t, face_state_t>::iterator git;
	map<voxel_t, vertex_state_t>::iterator vit;
	voxel_t v;
	int i, ret;

	/* clear any existing vertices */
	this->verts.clear();

	/* iterate over all the defined faces of this mesh */
	for(git = this->graph.begin(); git != this->graph.end(); git++)
	{
		/* iterate over the corners of the next face */
		for(i = 0; i < NUM_VERTS_PER_SQUARE; i++)
		{
			/* get the position of this corner in space,
			 * defined by the bottom corner of a voxel */
			v.x_ind = git->first.v.x_ind 
				+ voxel_corner_pos[voxel_corner_by_face[
					git->first.f][i]][0];
			v.y_ind = git->first.v.y_ind
				+ voxel_corner_pos[voxel_corner_by_face[
					git->first.f][i]][1];
			v.z_ind = git->first.v.z_ind
				+ voxel_corner_pos[voxel_corner_by_face[
					git->first.f][i]][2];

			/* check if this vertex is already defined */
			vit = this->verts.find(v);
			if(vit == this->verts.end())
			{
				/* add a new entry to the verts map */
				vit = this->verts.insert(
							make_pair<voxel_t, 
							vertex_state_t>(v, 
							vertex_state_t(v))
						).first;
			}

			/* add the regions for this face to this vertex */
			vit->second.reg_inds.insert(git->second.region_id);
		}
	}

	/* At this stage, all vertices should be defined appropriately.
	 * Now we can compute their position based on plane intersections,
	 * subject to the constraint that this doesn't perturb their
	 * position by more than a grid size */
	for(vit = this->verts.begin(); vit != this->verts.end(); vit++)
	{
		ret = this->project_vertex(vit->second);
		if(ret < 0)
			return PROPEGATE_ERROR(-1, ret);
	}

	/* success */
	return 0;
}

int mesher_t::write_to_obj(char* filename)
{
	ofstream outfile;
	map<face_t, face_state_t>::iterator it;
	face_t f;
	int num_verts_written;
	int r, g, b, i;
	voxel_t corner;
	map<voxel_t, vertex_state_t>::iterator vit;
	point_t q;

	/* check arguments */
	if(!filename)
		return -1;

	/* open file for writing */
	outfile.open(filename);
	if(!outfile.is_open())
		return -2;

	/* iterate through faces */
	num_verts_written = 0;
	for(it = this->graph.begin(); it != this->graph.end(); it++)
	{
		/* get current face */
		f = (*it).first;

		/* make a color for this face */
		srand(it->second.region_id);
		r = rand() % MAX_BYTE;
		g = rand() % MAX_BYTE;
		b = rand() % MAX_BYTE;

		/* place a vertex at each corner of this face */
		for(i = 0; i < NUM_VERTS_PER_SQUARE; i++)
		{
			/* get corner, defined by a voxel */
			corner.x_ind = f.v.x_ind + voxel_corner_pos[
					voxel_corner_by_face[f.f][i]][0];
			corner.y_ind = f.v.y_ind + voxel_corner_pos[
					voxel_corner_by_face[f.f][i]][1];
			corner.z_ind = f.v.z_ind + voxel_corner_pos[
					voxel_corner_by_face[f.f][i]][2];

			/* get position of current vertex */
			vit = this->verts.find(corner);
			if(vit == this->verts.end())
			{
				/* store position of corner */
				q.x = corner.x_ind;
				q.y = corner.y_ind;
				q.z = corner.z_ind;
			}
			else
			{
				/* get vertex position */
				q.x = vit->second.p.x;
				q.y = vit->second.p.y;
				q.z = vit->second.p.z;
			}

			/* write to file */
			outfile << "v " 
			       	<< q.x << " " << q.y << " " << q.z << " "
				<< r << " " << g << " " << b << endl;
		}

		/* write face indices */
		outfile << "f "
		        << (num_verts_written+1) << " "
			<< (num_verts_written+2) << " "
			<< (num_verts_written+3) << endl;
		outfile << "f "
			<< (num_verts_written+1) << " "
			<< (num_verts_written+3) << " "
			<< (num_verts_written+4) << endl;
		num_verts_written += 4;
	}

	/* clean up */
	outfile.close();
	return 0;
}

int mesher_t::find_neighbors_for(face_t& f)
{
	face_t fo;
	map<face_t, face_state_t>::iterator fit, it;
	int d, i;

	/* find the specified face in the map */
	fit = this->graph.find(f);
	if(fit == this->graph.end())
		return -1; /* invalid argument face */
	fo.copy(f);

	/* determine if facing in the + or - direction along axis */
	switch(f.f)
	{
		/* minus direction? */
		case VOXEL_FACE_XMINUS:
		case VOXEL_FACE_YMINUS:
		case VOXEL_FACE_ZMINUS:
			d = -1;
			break;

		/* plus direction? */
		case VOXEL_FACE_XPLUS:
		case VOXEL_FACE_YPLUS:
		case VOXEL_FACE_ZPLUS:
			d = 1;
			break;

		/* error */
		default:
			PRINT_ERROR("[mesher_t.find_neighbors_for]\t"
					"bad face_t given");
			LOGI("\t\tf.f = %d\n", f.f);
			return -2;
	}

	/* determine which axis the face is orthogonal to */
	switch(f.f)
	{
		/* x-axis */
		case VOXEL_FACE_XMINUS:
		case VOXEL_FACE_XPLUS:
			
			/* There are twelve possible neighbors */

			/* four of them are faces of f.v */
			fo.f = VOXEL_FACE_YMINUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[2].copy(fo);
			
			fo.f = VOXEL_FACE_YPLUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[0].copy(fo);
			
			fo.f = VOXEL_FACE_ZMINUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[3].copy(fo);
			
			fo.f = VOXEL_FACE_ZPLUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[1].copy(fo);
		
			/* four of them would be parallel to f */
			fo.f = f.f;
			fo.v.y_ind = f.v.y_ind-1;
			if(this->graph.count(fo))
				(*fit).second.neighbors[2].copy(fo);
		
			fo.v.y_ind = f.v.y_ind+1;
			if(this->graph.count(fo))
				(*fit).second.neighbors[0].copy(fo);
		
			fo.v.y_ind = f.v.y_ind;
			fo.v.z_ind = f.v.z_ind-1;
			if(this->graph.count(fo))
				(*fit).second.neighbors[3].copy(fo);
			
			fo.v.z_ind = f.v.z_ind+1;
			if(this->graph.count(fo))
				(*fit).second.neighbors[1].copy(fo);
	
			/* four of them would be above f */
			fo.v.x_ind = d + f.v.x_ind;
			fo.v.y_ind = f.v.y_ind-1;
			fo.v.z_ind = f.v.z_ind;
			fo.f = VOXEL_FACE_YPLUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[2].copy(fo);
	
			fo.v.y_ind = f.v.y_ind+1;
			fo.f = VOXEL_FACE_YMINUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[0].copy(fo);
	
			fo.v.y_ind = f.v.y_ind;
			fo.v.z_ind = f.v.z_ind-1;
			fo.f = VOXEL_FACE_ZPLUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[3].copy(fo);
			
			fo.v.z_ind = f.v.z_ind+1;
			fo.f = VOXEL_FACE_ZMINUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[1].copy(fo);
	
			break;

		/* y-axis */
		case VOXEL_FACE_YMINUS:
		case VOXEL_FACE_YPLUS:
			
			/* There are twelve possible neighbors */
			
			/* four of them are faces of f.v */
			fo.f = VOXEL_FACE_XMINUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[0].copy(fo);
			
			fo.f = VOXEL_FACE_XPLUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[2].copy(fo);
			
			fo.f = VOXEL_FACE_ZMINUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[3].copy(fo);
			
			fo.f = VOXEL_FACE_ZPLUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[1].copy(fo);
		
			/* four of them would be parallel to f */
			fo.f = f.f;
			fo.v.x_ind = f.v.x_ind-1;
			if(this->graph.count(fo))
				(*fit).second.neighbors[0].copy(fo);
		
			fo.v.x_ind = f.v.x_ind+1;
			if(this->graph.count(fo))
				(*fit).second.neighbors[2].copy(fo);
		
			fo.v.x_ind = f.v.x_ind;
			fo.v.z_ind = f.v.z_ind-1;
			if(this->graph.count(fo))
				(*fit).second.neighbors[3].copy(fo);
			
			fo.v.z_ind = f.v.z_ind+1;
			if(this->graph.count(fo))
				(*fit).second.neighbors[1].copy(fo);
	
			/* four of them would be above f */
			fo.v.x_ind = f.v.x_ind-1;
			fo.v.y_ind = d + f.v.y_ind;
			fo.v.z_ind = f.v.z_ind;
			fo.f = VOXEL_FACE_XPLUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[0].copy(fo);
	
			fo.v.x_ind = f.v.x_ind+1;
			fo.f = VOXEL_FACE_XMINUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[2].copy(fo);
	
			fo.v.x_ind = f.v.x_ind;
			fo.v.z_ind = f.v.z_ind-1;
			fo.f = VOXEL_FACE_ZPLUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[3].copy(fo);
			
			fo.v.z_ind = f.v.z_ind+1;
			fo.f = VOXEL_FACE_ZMINUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[1].copy(fo);
	
			break;

		/* z-axis */
		case VOXEL_FACE_ZMINUS:
		case VOXEL_FACE_ZPLUS:
			
			/* There are twelve possible neighbors */
			
			/* four of them are faces of f.v */
			fo.f = VOXEL_FACE_YMINUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[3].copy(fo);
			
			fo.f = VOXEL_FACE_YPLUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[1].copy(fo);
			
			fo.f = VOXEL_FACE_XMINUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[2].copy(fo);
			
			fo.f = VOXEL_FACE_XPLUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[0].copy(fo);
		
			/* four of them would be parallel to f */
			fo.f = f.f;
			fo.v.y_ind = f.v.y_ind-1;
			if(this->graph.count(fo))
				(*fit).second.neighbors[3].copy(fo);
		
			fo.v.y_ind = f.v.y_ind+1;
			if(this->graph.count(fo))
				(*fit).second.neighbors[1].copy(fo);
		
			fo.v.y_ind = f.v.y_ind;
			fo.v.x_ind = f.v.x_ind-1;
			if(this->graph.count(fo))
				(*fit).second.neighbors[2].copy(fo);
			
			fo.v.x_ind = f.v.x_ind+1;
			if(this->graph.count(fo))
				(*fit).second.neighbors[0].copy(fo);
	
			/* four of them would be above f */
			fo.v.x_ind = f.v.x_ind;
			fo.v.y_ind = f.v.y_ind-1;
			fo.v.z_ind = d + f.v.z_ind;
			fo.f = VOXEL_FACE_YPLUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[3].copy(fo);
	
			fo.v.y_ind = f.v.y_ind+1;
			fo.f = VOXEL_FACE_YMINUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[1].copy(fo);
	
			fo.v.y_ind = f.v.y_ind;
			fo.v.x_ind = f.v.x_ind-1;
			fo.f = VOXEL_FACE_XPLUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[2].copy(fo);
			
			fo.v.x_ind = f.v.x_ind+1;
			fo.f = VOXEL_FACE_XMINUS;
			if(this->graph.count(fo))
				(*fit).second.neighbors[0].copy(fo);
	
			break;

		/* error */
		default:
			/* REALLY should not be able to get here */
			return -3;
	}
	
	/* check if face points in negative direction */
	if(d < 0)
		(*fit).second.neighbors[1].swap(
					(*fit).second.neighbors[3]);

	/* there should be four neighbors now, verify this */
	for(i = 0; i < NUM_EDGES_PER_SQUARE; i++)
		if((*fit).second.neighbors[i].f < 0)
			return -i;

	/* success */
	return 0;
}

int mesher_t::compute_neighbors_of(int r)
{
	map<face_t, face_state_t>::iterator git, git2;
	set<face_t>::iterator it;
	int i;

	/* clear the neighbor set */
	this->regions[r].neighbors.clear();

	/* iterate over the faces */
	for(it = this->regions[r].faces.begin();
				it != this->regions[r].faces.end(); it++)
	{
		/* find this face in the graph */
		git = this->graph.find(*it);
		if(git == this->graph.end())
			return -1;

		/* iterate over neighboring faces to the current face */
		for(i = 0; i < NUM_EDGES_PER_SQUARE; i++)
		{
			/* get the region id of this neighboring face */
			git2 = this->graph.find(git->second.neighbors[i]);
			if(git2 == this->graph.end())
				return -2;

			/* add the region id of the neighbor to the set */
			if(git2->second.region_id != r)
				this->regions[r].neighbors.insert(
						git2->second.region_id);
		}
	}

	/* success */
	return 0;
}

int mesher_t::find_combined_properties(region_t& ra, region_t& rb, 
						point_t& center,
						normal_t& norm,
						double& err)
{
	set<face_t>::iterator it;
	double cov_mat[NUM_DIMS * NUM_DIMS];
	double min_eig_vect[NUM_DIMS];
	int ret, r, i, n, na, nb, n_total;
	double d;
	point_t p;
	normal_t q;
	region_t* rs[2];

	/* initialize covariance matrix to all zeros */
	for(i = 0; i < NUM_DIMS * NUM_DIMS; i++)
		cov_mat[i] = 0;

	/* get region information */
	na = ra.faces.size();
	nb = rb.faces.size();
	n_total = na + nb;

	/* check edge case */
	if(n_total == 0)
		return -1;

	/* compute the average point by performing a weighted average
	 * of the two centers of the the regions, weighted based on
	 * number of faces */
	center.x = ( na*(ra.pos.x) + nb*(rb.pos.x) ) / n_total;
	center.y = ( na*(ra.pos.y) + nb*(rb.pos.y) ) / n_total;
	center.z = ( na*(ra.pos.z) + nb*(rb.pos.z) ) / n_total;
	
	/* put regions in iterable array */
	rs[0] = &ra;
	rs[1] = &rb;

	/* iterate through all faces */
	n = 0;
	for(r = 0; r < 2; r++)
	{
		for(it = rs[r]->faces.begin(); it != rs[r]->faces.end();
								it++)
		{
			/* iterate through each vertex of this face */
			for(i = 0; i < NUM_VERTS_PER_SQUARE; i++)
			{
				/* get vertex position */
				q.x = (*it).v.x_ind + voxel_corner_pos[
					voxel_corner_by_face[(*it).f][i]
					][0] - center.x;
				q.y = (*it).v.y_ind + voxel_corner_pos[
					voxel_corner_by_face[(*it).f][i]
					][1] - center.y;
				q.z = (*it).v.z_ind + voxel_corner_pos[
					voxel_corner_by_face[(*it).f][i]
					][2] - center.z;

				/* build covariance matrix */
				cov_mat[0*NUM_DIMS + 0] += q.x*q.x;
				cov_mat[0*NUM_DIMS + 1] += q.x*q.y;
				cov_mat[0*NUM_DIMS + 2] += q.x*q.z;
				cov_mat[1*NUM_DIMS + 0] += q.y*q.x;
				cov_mat[1*NUM_DIMS + 1] += q.y*q.y;
				cov_mat[1*NUM_DIMS + 2] += q.y*q.z;
				cov_mat[2*NUM_DIMS + 0] += q.z*q.x;
				cov_mat[2*NUM_DIMS + 1] += q.z*q.y;
				cov_mat[2*NUM_DIMS + 2] += q.z*q.z;
				n++;
			}
		}
	}
	
	/* normalize cov matrix */
	for(i = 0; i < NUM_DIMS * NUM_DIMS; i++)
		cov_mat[i] /= n;

	/* find eigenvectors and eigenvalues of covariance matrix */
	ret = svd3_min_vect(min_eig_vect, cov_mat);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* set the normal vector to be this eigenvector */
	norm.x = min_eig_vect[0];
	norm.y = min_eig_vect[1];
	norm.z = min_eig_vect[2];

	/* update max_err parameter */
	err = 0;
	for(r = 0; r < 2; r++)
	{
		for(it = rs[r]->faces.begin(); it != rs[r]->faces.end(); 
								it++)
		{
			/* represent the face by its center */
			ret = (*it).get_center(p);
			if(ret)
				return PROPEGATE_ERROR(-2, ret);

			/* get distance from combined plane */
			q.x = p.x - center.x;
			q.y = p.y - center.y;
			q.z = p.z - center.z;

			/* get error from plane */
			d = fabs(NORMAL_DOT(q, norm));
	
			/* check if this error above current err */
			if(d > err)
				err = d;
		}
	}

	/* success */
	return 0;
}
	
int mesher_t::merge_regions(int r1, int r2,
			point_t& p, normal_t& norm, double err)
{
	map<face_t, face_state_t>::iterator git;
	set<face_t>::iterator sit;
	set<int>::iterator it;
	int ret;

	/* determine which region is larger (it will receive the
	 * faces of the smaller region) */
	if(this->regions[r1].faces.size() < this->regions[r2].faces.size())
	{
		/* swap */
		ret = r1;
		r1 = r2;
		r2 = ret;
	}
		
	/* merge the smaller of these regions into the larger */
	for(sit = this->regions[r2].faces.begin(); 
				sit != this->regions[r2].faces.end();
								sit++)
	{
		/* find this face in the graph */
		git = this->graph.find(*sit);
		if(git == this->graph.end())
			return -1;

		/* change the region id of this face */
		git->second.region_id = r1;

		/* add to larger region */
		this->regions[r1].faces.insert(git->first);
	}

	/* update region properties */
	this->regions[r1].pos.x = p.x;
	this->regions[r1].pos.y = p.y;
	this->regions[r1].pos.z = p.z;
	this->regions[r1].norm.x = norm.x;
	this->regions[r1].norm.y = norm.y;
	this->regions[r1].norm.z = norm.z;
	this->regions[r1].max_err = err;

	/* the neighbor set is the union of the two original
	 * neighbor sets */
	for(it = this->regions[r2].neighbors.begin();
				it != this->regions[r2].neighbors.end();
					it++)
	{
		/* add this neighbor to the merged region */
		this->regions[r1].neighbors.insert(*it);

		/* make sure this neighbor region has r1 and a
		 * neighbor and forgets about r2 */
		this->regions[*it].neighbors.erase(r2);
		this->regions[*it].neighbors.insert(r1);
	}

	/* remove r1 and r2 from neighbor set */
	this->regions[r1].neighbors.erase(r2);
	this->regions[r1].neighbors.erase(r1);
		
	/* clear out the smaller region */
	this->regions[r2].faces.clear();
	this->regions[r2].neighbors.clear();

	/* success */
	return r1;
}

int mesher_t::face_is_degenerate(face_t& f)
{
	map<face_t, face_state_t>::iterator it, oit;
	int i, r, s, c;

	/* retrieve info about face */
	it = this->graph.find(f);
	if(it == this->graph.end())
		return -1;

	/* get my info */
	r = it->second.region_id;

	/* get info of neighbors */
	s = -1;
	c = 0;
	for(i = 0; i < NUM_EDGES_PER_SQUARE; i++)
	{
		/* get neighbor info */
		oit = this->graph.find(it->second.neighbors[i]);
		if(oit == this->graph.end())
			return -2;

		/* check neighbor region compared to ours */
		if(oit->second.region_id != r)
		{
			c++;
			if(s < 0)
				s = oit->second.region_id;
			else if(s != oit->second.region_id)
				return -3;
		}
	}

	/* If got here, either this face is completely surrounded
	 * by its own region, or surrounded by faces from its own
	 * region or one other region.  If its touching three or
	 * more faces from another region, it is degenerate */
	if(c >= DEGENERATE_FACE_THRESHOLD)
		return s;
	return -4;
}
	
bool mesher_t::face_is_boundary(face_t& f)
{
	map<face_t, face_state_t>::iterator it, oit;
	map<voxel_t, vertex_state_t>::iterator vit;
	voxel_t v;
	int i, r;

	/* retrieve info about face */
	it = this->graph.find(f);
	if(it == this->graph.end())
		return false;

	/* get my info */
	r = it->second.region_id;

	/* get info of neighbors */
	for(i = 0; i < NUM_EDGES_PER_SQUARE; i++)
	{
		/* get neighbor info */
		oit = this->graph.find(it->second.neighbors[i]);
		if(oit == this->graph.end())
			continue;

		/* check neighbor region compared to ours */
		if(oit->second.region_id != r)
			return true;
	}

	/* also check vertices */
	for(i = 0; i < NUM_VERTS_PER_SQUARE; i++)
	{
		/* get location of vertex */
		v.x_ind = f.v.x_ind + voxel_corner_pos[
			voxel_corner_by_face[f.f][i]][0];
		v.y_ind = f.v.y_ind + voxel_corner_pos[
			voxel_corner_by_face[f.f][i]][1];
		v.z_ind = f.v.z_ind + voxel_corner_pos[
			voxel_corner_by_face[f.f][i]][2];
	
		/* get info about vertex */
		vit = this->verts.find(v);
		if(vit != this->verts.end() 
				&& vit->second.reg_inds.size() > 1)
			return true; /* this is also boundary */
	}

	/* no neighbors are different */
	return false;
}
	
int mesher_t::project_vertex(vertex_state_t& v)
{
	set<int>::iterator it;
	voxel_t vv;
	int ret, r1, r2, r3, i, num_planes, f;
	double d, cosang, c12, c23, c13, bound;
	point_t p;
	bool b12, b23, b13;

	/* determine how many planes adjoin this vertex */
	num_planes = 0;
	cosang = 0;
	switch(v.reg_inds.size())
	{
		case 1:
			/* only one plane.  Project vertex onto
			 * this plane. */
			r1 = *(v.reg_inds.begin());
			f = this->regions[r1].find_dominant_face();
			
			/* project the voxel's position onto the plane
			 * using the dominant axis-aligned direction */
			vv.x_ind = (int) round(v.p.x);
			vv.y_ind = (int) round(v.p.y);
			vv.z_ind = (int) round(v.p.z);
			ret = this->undo_plane_projection(p,vv,r1,f);
			if(ret)
				return PROPEGATE_ERROR(-1, ret);

			num_planes = 1;
			break;
			
		case 2:
			/* two planes correspond to a line.
			 * Find these planes, then find the
			 * projection onto this line. */
			r1 = *(v.reg_inds.begin());
			r2 = *(v.reg_inds.rbegin());
				
			/* check if problem ill-conditioned */
			cosang = fabs(NORMAL_DOT(this->regions[r1].norm, 
					this->regions[r2].norm)); 
			if(cosang > PARALLEL_THRESHOLD)
			{
				/* project to bigger plane */
				i = (this->regions[r1].faces.size()
					< this->regions[r2].faces.size()) 
						? r2 : r1;
				
				project_point_to_plane(p, v.p,
					this->regions[i].norm,
					this->regions[i].pos);
				num_planes = 1;
			}
			else
			{
				/* project to intersection line */
				project_point_to_plane_plane(p, v.p,
					this->regions[r1].norm,
					this->regions[r1].pos,
					this->regions[r2].norm,
					this->regions[r2].pos);
				num_planes = 2;
			}
			break;

		case 3:
			/* three planes intersect at a point, but
			 * verify that none are too parallel */
			it = v.reg_inds.begin();
			r1 = *it; it++;
			r2 = *it; it++;
			r3 = *it;
		
			/* check if planes parallel */
			c12 = fabs(NORMAL_DOT(this->regions[r1].norm, 
					this->regions[r2].norm));
			b12 = (c12 > PARALLEL_THRESHOLD);
			c23 = fabs(NORMAL_DOT(this->regions[r2].norm, 
					this->regions[r3].norm));
			b23 = (c23 > PARALLEL_THRESHOLD);
			c13 = fabs(NORMAL_DOT(this->regions[r1].norm, 
					this->regions[r3].norm));
			b13 = (c13 > PARALLEL_THRESHOLD);
			
			/* get the worst-case dihedral angle */
			cosang = (c12 > c23)
				? (c12 > c13 ? c12 : c13) 
				: (c23 > c13 ? c23 : c13);

			if(!(b12 || b23 || b13))
			{
				/* find the intersection point */
				intersect_three_planes(p,
						this->regions[r1].norm,
						this->regions[r1].pos,
						this->regions[r2].norm,
						this->regions[r2].pos,
						this->regions[r3].norm,
						this->regions[r3].pos);
				num_planes = 3;
			}
			else if(!b12)
			{
				/* project onto these two planes */
				project_point_to_plane_plane(p, v.p,
					this->regions[r1].norm,
					this->regions[r1].pos,
					this->regions[r2].norm,
					this->regions[r2].pos);
				num_planes = 2;
			}
			else if(!b23)
			{
				/* project onto these two planes */
				project_point_to_plane_plane(p, v.p,
					this->regions[r2].norm,
					this->regions[r2].pos,
					this->regions[r3].norm,
					this->regions[r3].pos);
				num_planes = 2;
			}
			else if(!b13)
			{
				/* project onto these two planes */
				project_point_to_plane_plane(p, v.p,
					this->regions[r1].norm,
					this->regions[r1].pos,
					this->regions[r3].norm,
					this->regions[r3].pos);
				num_planes = 2;
			}
			else
			{
				/* project to biggest plane */
				i = (this->regions[r1].faces.size()
					< this->regions[r2].faces.size()) 
					? 
					( this->regions[r2].faces.size()
					< this->regions[r3].faces.size()
					? r3 : r2) : r1;
				
				project_point_to_plane(p, v.p,
					this->regions[i].norm,
					this->regions[i].pos);
				num_planes = 1;
			}
			
			break;

		default:
			/* in this case, the problem may be
			 * ill-conditioned, so just leave the
			 * point as is */
			return 0;
	}

	/* check if p, which contians the projected point,
	 * is too far away from the starting location.
	 *
	 * Note that the current units are normalized so
	 * that a grid is unit length. */
	d = dist_sq(p, v.p);
	if(!isfinite(d))
	{
		/* projection went bad.  Don't move point */
		return 0;
	}
	else if(num_planes > 1)
	{
		/* determine the bound on the projection distance
		 * based on the dihedral angle found.  Be more leniant
		 * if this angle is close to 90 degrees */
		bound = VOXEL_FACE_MAX_ERR_BOUNDARY_THRESHOLD;
		if(cosang > PERPENDICULAR_THRESHOLD)
			bound = VOXEL_FACE_MAX_ERR_THRESHOLD 
				* (1 - cosang) * (1 - cosang);
		if(d > bound*bound)
		{
			/* find closest point to p within this range */
			d = (bound / sqrt(d));
			p.x = ((p.x - v.p.x) * d) + v.p.x;
			p.y = ((p.y - v.p.y) * d) + v.p.y;
			p.z = ((p.z - v.p.z) * d) + v.p.z;	
		}
	}

	/* store point in vertex state */
	v.p.x = p.x;
	v.p.y = p.y;
	v.p.z = p.z;
	return num_planes;
}

int mesher_t::point_axis_projected_to(double& u, double& v,
						point_t& p, int fn)
{
	/* project this center point based on face */
	switch(fn)
	{
		/* if facing x direction, care about y,z position */
		case VOXEL_FACE_XMINUS:
			u = p.z;
			v = p.y;
			break;
		case VOXEL_FACE_XPLUS:
			u = p.y;
			v = p.z;
			break;

		/* if facing y direction, care about x,z */
		case VOXEL_FACE_YMINUS:
			u = p.x;
			v = p.z;
			break;
		case VOXEL_FACE_YPLUS:
			u = p.z;
			v = p.x;
			break;

		/* if facing z direction, care about x,y */
		case VOXEL_FACE_ZMINUS:
			u = p.y;
			v = p.x;
			break;
		case VOXEL_FACE_ZPLUS:
			u = p.x;
			v = p.y;
			break;

		default:
			/* input argument not valid */
			return -1;
	}

	/* success */
	return 0;
}
	
int mesher_t::undo_point_axis_projection(voxel_t& p, int u, int v,
						int fn, voxel_t& c)
{
	/* project this center point based on face */
	switch(fn)
	{
		/* if facing x direction, care about y,z position */
		case VOXEL_FACE_XMINUS:
			p.z_ind = u + c.z_ind;
			p.y_ind = v + c.y_ind;
			p.x_ind = c.x_ind;
			break;
		case VOXEL_FACE_XPLUS:
			p.y_ind = u + c.y_ind;
			p.z_ind = v + c.z_ind;
			p.x_ind = c.x_ind;
			break;

		/* if facing y direction, care about x,z */
		case VOXEL_FACE_YMINUS:
			p.x_ind = u + c.x_ind;
			p.z_ind = v + c.z_ind;
			p.y_ind = c.y_ind;
			break;
		case VOXEL_FACE_YPLUS:
			p.z_ind = u + c.z_ind;
			p.x_ind = v + c.x_ind;
			p.y_ind = c.y_ind;
			break;

		/* if facing z direction, care about x,y */
		case VOXEL_FACE_ZMINUS:
			p.y_ind = u + c.y_ind;
			p.x_ind = v + c.x_ind;
			p.z_ind = c.z_ind;
			break;
		case VOXEL_FACE_ZPLUS:
			p.x_ind = u + c.x_ind;
			p.y_ind = v + c.y_ind;
			p.z_ind = c.z_ind;
			break;

		default:
			/* input argument not valid */
			return -2;
	}

	/* success */
	return 0;
}

int mesher_t::undo_plane_projection(point_t& p, voxel_t& vp,
						int r, int fn)
{
	double h0;

	/* first, find the height of this point from the plane */
	h0 = this->regions[r].height_of_voxel(vp);

	/* project the voxel to the plane */
	switch(fn)
	{
		/* x direction */
		case VOXEL_FACE_XMINUS:
		case VOXEL_FACE_XPLUS:

			/* y and z coordinates are preserved */
			p.y = vp.y_ind;
			p.z = vp.z_ind;

			/* find x */
			p.x = vp.x_ind - h0 / (this->regions[r].norm.x);
			break;

		/* y direction */
		case VOXEL_FACE_YMINUS:
		case VOXEL_FACE_YPLUS:

			/* x and z coordinates are preserved */
			p.x = vp.x_ind;
			p.z = vp.z_ind;

			/* find y */
			p.y = vp.y_ind - h0 / (this->regions[r].norm.y);
			break;

		/* z direction */
		case VOXEL_FACE_ZMINUS:
		case VOXEL_FACE_ZPLUS:

			/* x and y coordinates are preserved */
			p.x = vp.x_ind;
			p.y = vp.y_ind;

			/* find z */
			p.z = vp.z_ind - h0 / (this->regions[r].norm.z);
			break;

		/* invalid input */
		default:
			return -1;
	}
	
	/* success */
	return 0;
}
