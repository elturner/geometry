#include "region_io.h"
#include <vector>
#include <set>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include "../structs/triangulation.h"
#include "../triangulate/region_growing.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

using namespace std;

int writeply_with_regions(char* filename, triangulation_t& tri, 
				vector<planar_region_t>& rl, bool ascii)
{
	ofstream outfile;
	map<voxelface_t, vertex_t*>::iterator vit;
	vector<triangle_t*>::iterator tit;
	set<triangle_t*>::iterator stit;
	set<edge_t>::iterator eit;
	vector<planar_region_t>::iterator rit;
	stringstream ss;
	union {
		float f;
		int i;
		uint32_t b;
		char arr[sizeof(uint32_t)];
	} buf;
	uint8_t c;
	unsigned int i, num_regs;
	double px, py, pz;
	
	/* In order to export this mesh, we need to index the elements. */
	tri.index_vertices();
	prune_invalid_triangles_from_regions(rl, tri);

	/* we also only want to export the non-empty regions, so count
	 * how many of those there are */
	num_regs = 0;
	for(rit = rl.begin(); rit != rl.end(); rit++)
		if((*rit).tris.size() > 0)
			num_regs++;

	/* open file for writing */
	outfile.open(filename, ios::out | ios::binary);
	if(!outfile.is_open())
		return -1;

	/* write header in ascii */
	ss << "ply\n" 
	   << "format "
	   << (ascii ? "ascii" : "binary_little_endian")
	   << " 1.0\n"
	   << "element vertex "
	   << tri.vertices.size()
	   << "\nproperty float x\n"
	   << "property float y\n"
	   << "property float z\n"
	   << "element face "
	   << tri.triangles.size()
	   << "\nproperty list uchar int vertex_index\n"
	   << "element region "
	   << num_regs
	   << "\n"
	   << "property float nx\n"
	   << "property float ny\n"
	   << "property float nz\n"
	   << "property float px\n"
	   << "property float py\n"
	   << "property float pz\n"
	   << "property list int int triangle_index\n"
	   << "property list int int edge_pair_index\n"
	   << "end_header\n";
	outfile.write(ss.str().data(), ss.str().length());

	/* write vertices to disk */
	for(vit = tri.vertices.begin(); vit != tri.vertices.end(); vit++)
	{
		/* determine output format */
		if(ascii)
		{
			ss.str("");
			ss << (*vit).second->x << " "
			   << (*vit).second->y << " "
			   << (*vit).second->z << " "
			   << "\n";
			outfile.write(ss.str().data(), ss.str().length());
		}
		else
		{
			/* write x coordinate */
			buf.f = (float) ( (*vit).second->x );
			buf.b = (buf.b); /* host already little endian */
			outfile.write(buf.arr, sizeof(uint32_t));
	
			/* write y coordinate */
			buf.f = (float) ( (*vit).second->y );
			buf.b = (buf.b); /* host already little endian */
			outfile.write(buf.arr, sizeof(uint32_t));
	
			/* write z coordinate */
			buf.f = (float) ( (*vit).second->z );
			buf.b = (buf.b); /* host already little endian */
			outfile.write(buf.arr, sizeof(uint32_t));	
		}
	}

	/* write triangles to disk (*.ply's index from 0) */
	c = (uint8_t) NUM_VERTS_PER_TRI;
	for(tit = tri.triangles.begin(); tit != tri.triangles.end(); tit++)
	{
		/* determine format */
		if(ascii)
		{
			/* write how many vertices, and then vertex
			 * indices */
			ss.str("");
			ss << NUM_VERTS_PER_TRI;
			for(i = 0; i < NUM_VERTS_PER_TRI; i++)
				ss << " " << (*tit)->v[i]->index;
			ss << "\n";
			outfile.write(ss.str().data(), ss.str().length());
		}
		else
		{
			/* record number of vertices */
			outfile.write((char*) &c, sizeof(uint8_t));
	
			/* record index of each vertex */
			for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			{
				buf.i = (*tit)->v[i]->index;
				buf.b = (buf.b); /* host is little endian */
				outfile.write(buf.arr, sizeof(uint32_t));
			}
		}
	}

	/* write regions to disk */
	for(rit = rl.begin(); rit != rl.end(); rit++)
	{
		/* ignore empty regions */
		if((*rit).tris.size() == 0)
			continue;

		/* find the average point on this region */
		px = (*rit).avg_pos.x;
		py = (*rit).avg_pos.y;
		pz = (*rit).avg_pos.z;
		
		/* check the format */
		if(ascii)
		{
			ss.str("");
			
			/* export normal of region (nx,ny,nz) */
			ss << (*rit).avg_norm.x << " ";
			ss << (*rit).avg_norm.y << " ";
			ss << (*rit).avg_norm.z << " ";

			/* export point on the region (px,py,pz) by
			 * taking the average */
			ss << px << " " << py << " " << pz << " " ;

			/* export the size of this region */
			ss << (*rit).tris.size();
		
			/* export the triangle indices */
			for(stit = (*rit).tris.begin(); 
					stit != (*rit).tris.end(); stit++)
			{
				/* write triangle index to file */
				ss << " " << (*stit)->index;
			}

			/* export edges */
			ss << " " << 2 * (*rit).boundary.size();
			for(eit = (*rit).boundary.begin(); 
					eit != (*rit).boundary.end(); eit++)
			{
				/* write edge */
				ss << " " << (*eit).start->index
				   << " " << (*eit).end->index;
			}
			ss << "\n";

			/* write to file */
			outfile.write(ss.str().data(), ss.str().length());
		}
		else
		{
			/* export normal of region */
			
			/* write x coordinate */
			buf.f = (float) ( (*rit).avg_norm.x );
			buf.b = (buf.b); /* host is little-endian */
			outfile.write(buf.arr, sizeof(uint32_t));
	
			/* write y coordinate */
			buf.f = (float) ( (*rit).avg_norm.y );
			buf.b = (buf.b); /* host is little-endian */
			outfile.write(buf.arr, sizeof(uint32_t));
	
			/* write z coordinate */
			buf.f = (float) ( (*rit).avg_norm.z );
			buf.b = (buf.b); /* host is little-endian */
			outfile.write(buf.arr, sizeof(uint32_t));	

			/* export point on the region */
			
			/* write x coordinate */
			buf.f = (float) px;
			buf.b = (buf.b); /* host is little-endian */
			outfile.write(buf.arr, sizeof(uint32_t));
	
			/* write y coordinate */
			buf.f = (float) py;
			buf.b = (buf.b); /* host is little-endian */
			outfile.write(buf.arr, sizeof(uint32_t));
	
			/* write z coordinate */
			buf.f = (float) pz;
			buf.b = (buf.b); /* host is little-endian */
			outfile.write(buf.arr, sizeof(uint32_t));	

			/* export the size of this region */
			buf.i = (*rit).tris.size();
			buf.b = (buf.b); /* host is little-endian */
			outfile.write(buf.arr, sizeof(uint32_t));

			/* export the triangle indices of this region */
			for(stit = (*rit).tris.begin(); 
					stit != (*rit).tris.end(); stit++)
			{
				/* export index in binary */
				buf.i = (*stit)->index;
				
				/* verify triangle is valid */
				if(buf.i > (int)tri.triangles.size()
						|| buf.i < 0)
					continue;

				/* write triangle index to file */
				buf.b = (buf.b); /* host is little-endian */
				outfile.write(buf.arr, sizeof(uint32_t));
			}

			/* export number of edges (times two, since
			 * each edge represented by two ints */
			buf.i = 2 * (*rit).boundary.size();
			buf.b = (buf.b); /* host is little-endian */
			outfile.write(buf.arr, sizeof(uint32_t));

			/* export boundary edges */
			for(eit = (*rit).boundary.begin();
					eit != (*rit).boundary.end();
						eit++)
			{
				/* export index of edge start vertex */
				buf.i = (*eit).start->index;
				buf.b = (buf.b); /* host is little-endian */
				outfile.write(buf.arr, sizeof(uint32_t));

				/* export index of edge end vertex */
				buf.i = (*eit).end->index;
				buf.b = (buf.b); /* host is little-endian */
				outfile.write(buf.arr, sizeof(uint32_t));
			}
		}
	}

	/* clean up */
	outfile.close();
	return 0;
}
