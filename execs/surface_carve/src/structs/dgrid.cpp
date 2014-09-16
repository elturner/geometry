#include "dgrid.h"
#include <fstream>
#include <queue>
#include <set>
#include <map>
#include <vector>
#include <math.h>
#include <string.h>
#include "point.h"
#include "pose.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

using namespace std;

/****** VOXEL_T FUNCTIONS ******/

voxel_t::voxel_t(int xi, int yi, int zi)
{
	this->x_ind = xi;
	this->y_ind = yi;
	this->z_ind = zi;
}
	
voxel_t::voxel_t(double x, double y, double z, double vs)
{
	this->x_ind = (int) floor(x / vs); 
	this->y_ind = (int) floor(y / vs); 
	this->z_ind = (int) floor(z / vs); 
}

void voxel_t::set(int xi, int yi, int zi)
{
	this->x_ind = xi;
	this->y_ind = yi;
	this->z_ind = zi;
}

void voxel_t::set(double x, double y, double z, double vs)
{
	this->x_ind = (int) floor(x / vs); 
	this->y_ind = (int) floor(y / vs); 
	this->z_ind = (int) floor(z / vs); 
}

int voxel_t::set_to_mirror(voxel_t& v, int f)
{
	/* switch based on the face being mirrored across */
	switch(f)
	{
		case VOXEL_FACE_XMINUS:
			/* decrement x-coordinate */
			this->set(v.x_ind-1, v.y_ind, v.z_ind);
			return VOXEL_FACE_XPLUS;

		case VOXEL_FACE_XPLUS:
			/* increment x-coordinate */
			this->set(v.x_ind+1, v.y_ind, v.z_ind);
			return VOXEL_FACE_XMINUS;

		case VOXEL_FACE_YMINUS:
			/* decrement y-coordinate */
			this->set(v.x_ind, v.y_ind-1, v.z_ind);
			return VOXEL_FACE_YPLUS;
		
		case VOXEL_FACE_YPLUS:
			/* increment y-coordinate */
			this->set(v.x_ind, v.y_ind+1, v.z_ind);
			return VOXEL_FACE_YMINUS;
		
		case VOXEL_FACE_ZMINUS:
			/* decrement z-coordinate */
			this->set(v.x_ind, v.y_ind, v.z_ind-1);
			return VOXEL_FACE_ZPLUS;
		
		case VOXEL_FACE_ZPLUS:
			/* increment z-coordinate */
			this->set(v.x_ind, v.y_ind, v.z_ind+1);
			return VOXEL_FACE_ZMINUS;

		default:
			/* error! not a valid face. */
			return -1;
	}
}
	
bool voxel_t::intersects_segment_at_face(point_t& p, point_t& s,
							int f, double vs)
{
	double x, y, z, t;

	/* compute based on which voxel face */
	switch(f)
	{
		case VOXEL_FACE_YMINUS: /* negative-y face */

			/* get plane of face */	
			y = vs * (y_ind);

			/* check edge case */
			if(p.y == s.y)
				return (p.y == y);

			/* determine where the segment intersects */
			t = (y - p.y) / (s.y - p.y);
			x = p.x + t * (s.x - p.x);
			z = p.z + t * (s.z - p.z);
			return ( vs*x_ind <= x && x <= vs*(x_ind+1) )
				&& ( vs*z_ind <= z && z <= vs*(z_ind+1) );

		case VOXEL_FACE_XPLUS: /* pos-x */

			/* get plane of face */	
			x = vs * (x_ind + 1);

			/* check edge case */
			if(p.x == s.x)
				return (p.x == x);

			/* determine where the segment intersects */
			t = (x - p.x) / (s.x - p.x);
			y = p.y + t * (s.y - p.y);
			z = p.z + t * (s.z - p.z);
			return ( vs*y_ind <= y && y <= vs*(y_ind+1) )
				&& ( vs*z_ind <= z && z <= vs*(z_ind+1) );
		
		case VOXEL_FACE_YPLUS: /* pos-y */
		
			/* get plane of face */	
			y = vs * (y_ind + 1);

			/* check edge case */
			if(p.y == s.y)
				return (p.y == y);

			/* determine where the segment intersects */
			t = (y - p.y) / (s.y - p.y);
			x = p.x + t * (s.x - p.x);
			z = p.z + t * (s.z - p.z);
			return ( vs*x_ind <= x && x <= vs*(x_ind+1) )
				&& ( vs*z_ind <= z && z <= vs*(z_ind+1) );

		case VOXEL_FACE_XMINUS: /* neg-x */
			
			/* get plane of face */	
			x = vs * (x_ind);

			/* check edge case */
			if(p.x == s.x)
				return (p.x == x);

			/* determine where the segment intersects */
			t = (x - p.x) / (s.x - p.x);
			y = p.y + t * (s.y - p.y);
			z = p.z + t * (s.z - p.z);
			return ( vs*y_ind <= y && y <= vs*(y_ind+1) )
				&& ( vs*z_ind <= z && z <= vs*(z_ind+1) );
		
		case VOXEL_FACE_ZMINUS: /* neg-z */
			
			/* get plane of face */	
			z = vs * (z_ind);

			/* check edge case */
			if(p.z == s.z)
				return (p.z == z);

			/* determine where the segment intersects */
			t = (z - p.z) / (s.z - p.z);
			x = p.x + t * (s.x - p.x);
			y = p.y + t * (s.y - p.y);
			return ( vs*x_ind <= x && x <= vs*(x_ind+1) )
				&& ( vs*y_ind <= y && y <= vs*(y_ind+1) );
		
		case VOXEL_FACE_ZPLUS: /* pos-z */
			
			/* get plane of face */	
			z = vs * (z_ind + 1);

			/* check edge case */
			if(p.z == s.z)
				return (p.z == z);

			/* determine where the segment intersects */
			t = (z - p.z) / (s.z - p.z);
			x = p.x + t * (s.x - p.x);
			y = p.y + t * (s.y - p.y);
			return ( vs*x_ind <= x && x <= vs*(x_ind+1) )
				&& ( vs*y_ind <= y && y <= vs*(y_ind+1) );

		default:
			LOGI("[intersects_segment_at_face]\t"
					"bad face num: %d\n", f);
			return false;
	}
}

void voxel_t::get_center(point_t& c, double vs)
{
	c.x = vs * (this->x_ind + 0.5);
	c.y = vs * (this->y_ind + 0.5);
	c.z = vs * (this->z_ind + 0.5);
}

/****** DGRID_T FUNCTIONS ******/

dgrid_t::dgrid_t(double v)
{
	this->vs = v;
	this->voxels.clear();
	this->points.clear();
}

void dgrid_t::init(double v)
{
	this->vs = v;
	this->voxels.clear();
	this->points.clear();
}

void dgrid_t::clear()
{
	this->voxels.clear();
	this->points.clear();
}

int dgrid_t::populate_points_from_xyz(char* filename, vector<pose_t>& pl, 
						double range_limit_sq)
{
	ifstream infile;
	char buf[LINE_BUFFER_SIZE];
	point_t p;
	int ret, i, r, g, b, seriel, id;
	double d;

	/* check arguments */
	if(!filename)
		return -1;

	/* open file for reading */
	infile.open(filename);
	if(!infile.is_open())
		return -2;

	/* read lines of points from file */
	while(!infile.eof())
	{
		/* read next line */
		infile.getline(buf, LINE_BUFFER_SIZE);

		/* parse line to make sure it is valid */
		if(strlen(buf) < 2*NUM_ELEMENTS_PER_LINE - 1)
			continue; /* bad line */

		/* assume this line defines a scan point */
		ret = sscanf(buf, XYZ_FORMAT_STRING,
			&(p.x), &(p.y), &(p.z), &(r), &(g), &(b),
			&(id), &(p.timestamp), &(seriel));

		/* check format */
		if(ret != NUM_ELEMENTS_PER_LINE)
			continue; /* bad line */

		/* convert units to meters */
		p.x = MM2METERS(p.x);
		p.y = MM2METERS(p.y);
		p.z = MM2METERS(p.z);

		/* determine which pose this point is associated with */
		i = poselist_closest_index(pl, p.timestamp);
		
		/* get distance of scan point from pose */
		d = pose_point_dist_sq(pl[i], p);
		if(d > range_limit_sq)
		{
			/* ignore this point, and do not include it
			 * in the points set.  If a point is this
			 * far away from its corresponding pose, then
			 * we cannot accurately judge it as a inlier
			 * sample of a solid object. */
			continue;
		}

		/* add this point to our points set, by finding the
		 * voxel that contains this point */
		points.insert(voxel_t(p.x, p.y, p.z, this->vs));
	}

	/* clean up */
	infile.close();
	return 0;
}

voxel_state_t dgrid_t::get_voxel_state(voxel_t& v)
{
	map<voxel_t, voxel_state_t>::iterator it;

	/* see if the given voxel is contained in the map */
	it = this->voxels.find(v);
	if(it == this->voxels.end())
		return VOXEL_STATE_NONBOUNDARY; /* no entry means zero */

	/* return the found voxel state */
	return it->second;
}

void dgrid_t::set_voxel_state(voxel_t& v, voxel_state_t s)
{
	map<voxel_t, voxel_state_t>::iterator it;

	/* check if this voxel is already present in the map */
	it = this->voxels.find(v);
	if(it == this->voxels.end())
	{
		/* only add a new element if it's a boundary */
		if(s != VOXEL_STATE_NONBOUNDARY)
		{
			/* add the new element with these values */
			this->voxels.insert(pair<voxel_t, 
						voxel_state_t>(v,s));
		}
	}
	else
	{
		/* if we are setting this voxel to be a non-boundary,
		 * just remove it */
		if(s == VOXEL_STATE_NONBOUNDARY)
			this->voxels.erase(it);
		else /* update the value of the existing key-value pair */
			(*it).second = s;
	}
}

void dgrid_t::carve_voxel(voxel_t& v)
{
	map<voxel_t, voxel_state_t>::iterator vi, wi;
	voxel_t w;
	voxel_state_t s;
	unsigned int i, j;

	/* check the edge case of this being the first-ever carved
	 * voxel. */
	if(this->voxels.empty())
	{
		/* initialize the grid by adding the six surrounding
		 * boundary voxels to the given voxel. */
		
		w.set(v.x_ind, v.y_ind, v.z_ind+1);
		this->voxels.insert(pair<voxel_t, voxel_state_t>(w, 
				VOXEL_GET_FACE_BIT(VOXEL_FACE_ZMINUS)));
		
		w.set(v.x_ind, v.y_ind, v.z_ind-1);
		this->voxels.insert(pair<voxel_t, voxel_state_t>(w, 
				VOXEL_GET_FACE_BIT(VOXEL_FACE_ZPLUS)));
		
		w.set(v.x_ind, v.y_ind+1, v.z_ind);
		this->voxels.insert(pair<voxel_t, voxel_state_t>(w, 
				VOXEL_GET_FACE_BIT(VOXEL_FACE_YMINUS)));
		
		w.set(v.x_ind, v.y_ind-1, v.z_ind);
		this->voxels.insert(pair<voxel_t, voxel_state_t>(w, 
				VOXEL_GET_FACE_BIT(VOXEL_FACE_YPLUS)));
		
		w.set(v.x_ind+1, v.y_ind, v.z_ind);
		this->voxels.insert(pair<voxel_t, voxel_state_t>(w, 
				VOXEL_GET_FACE_BIT(VOXEL_FACE_XMINUS)));
		
		w.set(v.x_ind-1, v.y_ind, v.z_ind);
		this->voxels.insert(pair<voxel_t, voxel_state_t>(w, 
				VOXEL_GET_FACE_BIT(VOXEL_FACE_XPLUS)));

		/* The dgrid now contains exactly one empty voxel
		 * at exactly v.  We don't need to do anything else. */
		return;
	}

	/* In the general case, carving needs to occur incrementally
	 * outward, so that carving can only be completed if the voxel
	 * in question is either inside (in which case it's a no-op),
	 * or if the voxel in question is on the border. 
	 *
	 * We need to determine which of those cases is occurring, by
	 * looking at the state of voxel v */
	vi = this->voxels.find(v);
	if(vi == this->voxels.end())
		return; /* no carving necessary (not boundary) */
	s = (*vi).second;
	if(s == VOXEL_STATE_NONBOUNDARY)
	{
		/* no carving necessary, and in fact this element
		 * of the map is wasting space, since deleting it
		 * will represent the same thing. */
		this->voxels.erase(vi);
		return; 
	}

	/* Now we know we are carving a boundary voxel, so we must
	 * modify all non-interior neighbors of this voxel to become
	 * boundary voxels with an inward-face towards this voxel. */
	for(i = 0; i < NUM_FACES_PER_CUBE; i++)
	{
		/* only proceed if outward face */
		if(VOXEL_IS_FACE_BIT_INWARD(s, i))
			continue;

		/* get position of neighboring voxel, and the face to set */
		j = w.set_to_mirror(v, i);
		wi = this->voxels.find(w);

		/* update this neighboring voxel to incorporate v being
		 * interior */
		if(wi == this->voxels.end())
			this->voxels.insert(pair<voxel_t, voxel_state_t>(w,
						VOXEL_GET_FACE_BIT(j)));
		else
			VOXEL_SET_FACE_BIT_INWARD((*wi).second, j);
	}

	/* set this voxel to be inside, which can be done by removing
	 * the voxel element from the map. */
	this->voxels.erase(vi);
}
	
void dgrid_t::fill_voxel(voxel_t& v)
{
	voxel_t w;
	voxel_state_t s, sw;
	unsigned int i, j;

	/* Note that we can only fill interior (empty) voxels, so
	 * first we must check the state of the voxel v */
	s = this->get_voxel_state(v);
	if(s != VOXEL_STATE_NONBOUNDARY)
		return;

	/* Now we know we are filling, so we must
	 * modify all neighbors that are boundary voxels to update
	 * their face bits, and update the face bits of v to reflect
	 * all interior neighbors.  Note that there cannot be any 
	 * non-boundary exterior neighbors, since that violates the
	 * assumptions of this function. */
	for(i = 0; i < NUM_FACES_PER_CUBE; i++)
	{
		/* get position of neighboring voxel, and 
		 * the intersecting face */
		j = w.set_to_mirror(v, i);
		sw = this->get_voxel_state(w);

		/* check if neighbor w is on the boundary */
		if(sw == VOXEL_STATE_NONBOUNDARY)
		{
			/* Neighbor is NOT on the boundary, so we must
			 * assume that w is an interior voxel.  We don't
			 * need to change w at all, but we do need to
			 * update v's state to reflect that it is 
			 * a solid voxel neighboring an inside voxel */
			VOXEL_SET_FACE_BIT_INWARD(s, i);
		}
		else
		{
			/* w is a boundary voxel, which means we do not
			 * need to modify v's state, but we do need to
			 * modify w's state to reflect that v is no
			 * longer interior.
			 *
			 * First, check to make sure that w currently
			 * views v as an interior voxel */
			if(!VOXEL_IS_FACE_BIT_INWARD(sw, j))
			{
				/* w does NOT view v as inside, which
				 * means that this function was called
				 * on an exterior voxel.  It's already
				 * solid, so we can't fill it.  Just
				 * return without doing anything. */
				return;
			}
			
			/* update the state of w to show that v is now
			 * a solid, exterior voxel. */
			VOXEL_SET_FACE_BIT_OUTWARD(sw, j);
			this->set_voxel_state(w, sw);
		}
	}

	/* Set the voxel v to be a solid voxel, with state s */
	this->set_voxel_state(v, s);
}
	
void dgrid_t::carve_segment(point_t& p, point_t& s, bool force)
{
	voxel_t pi, si; /* current, dest voxel pos */
	int dx, dy, dz, fnx, fny, fnz;
	point_t c;
	double x, y, z;

	/* start at pose location */
	pi.set(p.x, p.y, p.z, this->vs);

	/* want to end up at scan location */
	si.set(s.x, s.y, s.z, this->vs);

	/* get general directions of movement.  Each value
	 * specifies a face number of the current voxel to
	 * traverse to get to the next voxel */
	if(p.x < s.x)
	{
		/* moving in +x dir */
		dx = 1;
		fnx = VOXEL_FACE_XPLUS;
	}
	else
	{
		/* moving in -x dir */
		dx = -1;
		fnx = VOXEL_FACE_XMINUS;
	}
	if(p.y < s.y)
	{
		/* moving in +y dir */
		dy = 1;
		fny = VOXEL_FACE_YPLUS;
	}
	else
	{
		/* moving in -y dir */
		dy = -1;
		fny = VOXEL_FACE_YMINUS;
	}
	if(p.z < s.z)
	{
		/* moving in +z dir */
		dz = 1;
		fnz = VOXEL_FACE_ZPLUS;
	}
	else
	{
		/* moving in -z dir */
		dz = -1;
		fnz = VOXEL_FACE_ZMINUS;
	}

	/* iterate over voxels until we reach destination */
	while(pi != si)
	{
		/* check if we should watch out for point-voxels */
		if(!force)
		{
			/* if we hit a voxel that contains scan points,
			 * stop carving, since that voxel should be
			 * treated as a wall */
			if(this->points.count(pi) > 0)
				return; /* stop carving */
		}

		/* mark current voxel as empty */
		this->carve_voxel(pi);
		
		/* move to next voxel intersected by segment */
		if(pi.intersects_segment_at_face(p, s, fnx, this->vs))
		{
			pi.x_ind += dx;
			continue;
		}
		if(pi.intersects_segment_at_face(p, s, fny, this->vs))
		{
			pi.y_ind += dy;
			continue;
		}
		if(pi.intersects_segment_at_face(p, s, fnz, this->vs))
		{
			pi.z_ind += dz;
			continue;
		}

		/* should never get here */
		PRINT_ERROR("[dgrid.carve_segment]:\tgot off track!");
		PRINT_ERROR("\tattempting to correct...");

		/* attempt to correct track by using basic heuristic to
		 * do ray-tracing */
		pi.get_center(c, this->vs); /* find center of curr voxel */
		
		/* find disp. to destination */
		x = fabs(c.x - s.x);
		y = fabs(c.y - s.y);
		z = fabs(c.z - s.z);

		/* move by one voxel in the correct direction */
		if(x >= y && x >= z)
			pi.x_ind += (c.x < s.x) ? 1 : -1;
		else if(y >= z)
			pi.y_ind += (c.y < s.y) ? 1 : -1;
		else
			pi.z_ind += (c.z < s.z) ? 1 : -1;
	}

	/* carve final destination voxel */
	this->carve_voxel(si);
}
	
void dgrid_t::remove_outliers()
{
	map<voxel_t, voxel_state_t>::iterator it;
	queue<voxel_t> locs_to_check;
	voxel_t v, vo;
	voxel_state_t s, so;
	int i, k, n;

	/* do one initial run through the entire grid, checking
	 * for voxel locations that could be outliers */
	for(it = this->voxels.begin(); it != this->voxels.end(); it++)
	{
		/* get current voxel */
		v = (*it).first;
		s = (*it).second;

		/* first, check if the current voxel is an outlier,
		 * by counting how many neighboring voxels are
		 * also 'outside' or 'solid' */
		n = 0;
		for(i = 0; i < NUM_FACES_PER_CUBE; i++)
			if(!VOXEL_IS_FACE_BIT_INWARD(s,i))
				n++;

		/* check if outlier */
		if(n < GRID_CLEANUP_FACE_THRESHOLD)
		{
			/* we have an outlier.  prepare to carve it */
			locs_to_check.push(v);
		}
		
		/* note that iterating through boundary voxels only
		 * gets us the outliers that are 'solid'.  There may also
		 * be interior outliers, so we should check any
		 * interior voxels that are adjacent to boundary voxels. */
		for(i = 0; i < NUM_FACES_PER_CUBE; i++)
		{
			/* only care about interior neighbors */
			if(!VOXEL_IS_FACE_BIT_INWARD(s, i))
				continue;
				
			/* get position of interior voxel */
			k = vo.set_to_mirror(v, i);
			
			/* we'll want to check this position */
			locs_to_check.push(vo);
		}
	}

	/* now we need to do the actual outlier removal.  Keep
	 * going as long as we have elements to check */
	while(!locs_to_check.empty())
	{
		/* get the next voxel location to check */
		v = locs_to_check.front();
		locs_to_check.pop();

		/* get the state of this element */
		s = this->get_voxel_state(v);

		/* check if this element is an outlier */
		if(s == VOXEL_STATE_NONBOUNDARY)
		{
			/* since a non-boundary voxel contains no
			 * direct information, need to check each of its
			 * neighbors */
			n = 0; /* n counts the number of empty neighbors */
			for(i = 0; i < NUM_FACES_PER_CUBE; i++)
			{
				/* get position of neighbor voxel */
				k = vo.set_to_mirror(v, i);

				/* determine if neighbor of v is
				 * solid (read as: boundary) */
				so = this->get_voxel_state(vo);
				if(so == VOXEL_STATE_NONBOUNDARY)
					n++;
				else
				{
					/* neighbor is boundary, which
					 * means solid.  But we should
					 * double-check that this
					 * boundary recognizes v as an
					 * interior voxel */
					if(!VOXEL_IS_FACE_BIT_INWARD(so,k))
					{
						/* OH NO! Neighbor voxel
						 * thinks v is exterior,
						 * which means we don't
						 * need to carve it. */
						n = NUM_FACES_PER_CUBE;
						break;
					}
				}
			}
			
			/* check if there are few enough neighbors
			 * to count v as an outlier */
			if(n < GRID_CLEANUP_FACE_THRESHOLD)
			{
				/* flip voxel */
				this->fill_voxel(v);
				
				/* it's neighbors may be outliers, so
				 * we need to check those as well */
				vo.set(v.x_ind-1, v.y_ind, v.z_ind);
				locs_to_check.push(vo);
				
				vo.set(v.x_ind+1, v.y_ind, v.z_ind);
				locs_to_check.push(vo);
				
				vo.set(v.x_ind, v.y_ind-1, v.z_ind);
				locs_to_check.push(vo);
				
				vo.set(v.x_ind, v.y_ind+1, v.z_ind);
				locs_to_check.push(vo);
				
				vo.set(v.x_ind, v.y_ind, v.z_ind-1);
				locs_to_check.push(vo);
				
				vo.set(v.x_ind, v.y_ind, v.z_ind+1);
				locs_to_check.push(vo);
			}
		}
		else
		{
			/* this is a boundary (solid) voxel, which means
			 * that it is an outlier if strictly less than
			 * GRID_CLEANUP_FACE_THRESHOLD neighbors that
			 * are also solid */
			n = 0;
			for(i = 0; i < NUM_FACES_PER_CUBE; i++)
				if(!VOXEL_IS_FACE_BIT_INWARD(s,i))
					n++;

			/* if outlier, flip voxel condition */
			if(n < GRID_CLEANUP_FACE_THRESHOLD)
			{
				/* flip this voxel */
				this->carve_voxel(v);
				
				/* it's neighbors may be outliers, so
				 * we need to check those as well */
				vo.set(v.x_ind-1, v.y_ind, v.z_ind);
				locs_to_check.push(vo);
				
				vo.set(v.x_ind+1, v.y_ind, v.z_ind);
				locs_to_check.push(vo);
				
				vo.set(v.x_ind, v.y_ind-1, v.z_ind);
				locs_to_check.push(vo);
				
				vo.set(v.x_ind, v.y_ind+1, v.z_ind);
				locs_to_check.push(vo);
				
				vo.set(v.x_ind, v.y_ind, v.z_ind-1);
				locs_to_check.push(vo);
				
				vo.set(v.x_ind, v.y_ind, v.z_ind+1);
				locs_to_check.push(vo);
			}
		}
	}
}
	
int dgrid_t::write_to_obj(char* filename)
{
	ofstream outfile;
	map<voxel_t, voxel_state_t>::iterator it;
	voxel_t v;
	voxel_state_t s;
	int num_verts_written;

	/* check arguments */
	if(!filename)
		return -1;

	/* open file for writing */
	outfile.open(filename);
	if(!outfile.is_open())
		return -2;

	/* iterate through boundary voxels */
	num_verts_written = 0;
	for(it = this->voxels.begin(); it != this->voxels.end(); it++)
	{
		/* get current voxel */
		v = (*it).first;
		s = (*it).second;

		/* iterate over the faces of this boundary voxel */
		
		if(VOXEL_IS_FACE_BIT_INWARD(s, VOXEL_FACE_XMINUS))
		{
			outfile << "v " 
			        << (this->vs*(v.x_ind)) << " "
				<< (this->vs*(v.y_ind)) << " "
				<< (this->vs*(v.z_ind)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind)) << " "
				<< (this->vs*(v.y_ind)) << " "
				<< (this->vs*(v.z_ind+1)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind)) << " "
				<< (this->vs*(v.y_ind+1)) << " "
				<< (this->vs*(v.z_ind+1)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind)) << " "
				<< (this->vs*(v.y_ind+1)) << " "
				<< (this->vs*(v.z_ind)) << endl;
			outfile << "f "
			        << (num_verts_written+1) << " "
				<< (num_verts_written+2) << " "
				<< (num_verts_written+3) << " "
				<< (num_verts_written+4) << endl;
			num_verts_written += 4;
		}
		if(VOXEL_IS_FACE_BIT_INWARD(s, VOXEL_FACE_XPLUS))
		{
			outfile << "v " 
			        << (this->vs*(v.x_ind+1)) << " "
				<< (this->vs*(v.y_ind)) << " "
				<< (this->vs*(v.z_ind)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind+1)) << " "
				<< (this->vs*(v.y_ind+1)) << " "
				<< (this->vs*(v.z_ind)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind+1)) << " "
				<< (this->vs*(v.y_ind+1)) << " "
				<< (this->vs*(v.z_ind+1)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind+1)) << " "
				<< (this->vs*(v.y_ind)) << " "
				<< (this->vs*(v.z_ind+1)) << endl;
			outfile << "f "
			        << (num_verts_written+1) << " "
				<< (num_verts_written+2) << " "
				<< (num_verts_written+3) << " "
				<< (num_verts_written+4) << endl;
			num_verts_written += 4;
		}
		if(VOXEL_IS_FACE_BIT_INWARD(s, VOXEL_FACE_YMINUS))
		{
			outfile << "v " 
			        << (this->vs*(v.x_ind)) << " "
				<< (this->vs*(v.y_ind)) << " "
				<< (this->vs*(v.z_ind)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind+1)) << " "
				<< (this->vs*(v.y_ind)) << " "
				<< (this->vs*(v.z_ind)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind+1)) << " "
				<< (this->vs*(v.y_ind)) << " "
				<< (this->vs*(v.z_ind+1)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind)) << " "
				<< (this->vs*(v.y_ind)) << " "
				<< (this->vs*(v.z_ind+1)) << endl;
			outfile << "f "
			        << (num_verts_written+1) << " "
				<< (num_verts_written+2) << " "
				<< (num_verts_written+3) << " "
				<< (num_verts_written+4) << endl;
			num_verts_written += 4;
		}
		if(VOXEL_IS_FACE_BIT_INWARD(s, VOXEL_FACE_YPLUS))
		{
			outfile << "v " 
			        << (this->vs*(v.x_ind)) << " "
				<< (this->vs*(v.y_ind+1)) << " "
				<< (this->vs*(v.z_ind)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind)) << " "
				<< (this->vs*(v.y_ind+1)) << " "
				<< (this->vs*(v.z_ind+1)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind+1)) << " "
				<< (this->vs*(v.y_ind+1)) << " "
				<< (this->vs*(v.z_ind+1)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind+1)) << " "
				<< (this->vs*(v.y_ind+1)) << " "
				<< (this->vs*(v.z_ind)) << endl;
			outfile << "f "
			        << (num_verts_written+1) << " "
				<< (num_verts_written+2) << " "
				<< (num_verts_written+3) << " "
				<< (num_verts_written+4) << endl;
			num_verts_written += 4;
		}
		if(VOXEL_IS_FACE_BIT_INWARD(s, VOXEL_FACE_ZMINUS))
		{
			outfile << "v " 
			        << (this->vs*(v.x_ind)) << " "
				<< (this->vs*(v.y_ind)) << " "
				<< (this->vs*(v.z_ind)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind)) << " "
				<< (this->vs*(v.y_ind+1)) << " "
				<< (this->vs*(v.z_ind)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind+1)) << " "
				<< (this->vs*(v.y_ind+1)) << " "
				<< (this->vs*(v.z_ind)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind+1)) << " "
				<< (this->vs*(v.y_ind)) << " "
				<< (this->vs*(v.z_ind)) << endl;
			outfile << "f "
			        << (num_verts_written+1) << " "
				<< (num_verts_written+2) << " "
				<< (num_verts_written+3) << " "
				<< (num_verts_written+4) << endl;
			num_verts_written += 4;
		}
		if(VOXEL_IS_FACE_BIT_INWARD(s, VOXEL_FACE_ZPLUS))
		{
			outfile << "v " 
			        << (this->vs*(v.x_ind)) << " "
				<< (this->vs*(v.y_ind)) << " "
				<< (this->vs*(v.z_ind+1)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind+1)) << " "
				<< (this->vs*(v.y_ind)) << " "
				<< (this->vs*(v.z_ind+1)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind+1)) << " "
				<< (this->vs*(v.y_ind+1)) << " "
				<< (this->vs*(v.z_ind+1)) << endl;
			outfile << "v " 
			        << (this->vs*(v.x_ind)) << " "
				<< (this->vs*(v.y_ind+1)) << " "
				<< (this->vs*(v.z_ind+1)) << endl;
			outfile << "f "
			        << (num_verts_written+1) << " "
				<< (num_verts_written+2) << " "
				<< (num_verts_written+3) << " "
				<< (num_verts_written+4) << endl;
			num_verts_written += 4;
		}
	}

	/* clean up */
	outfile.close();
	return 0;
}
