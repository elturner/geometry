#include "mesh_io.h"
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <queue>
#include <stdint.h>
#include "../structs/triangulation.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

using namespace std;

int writeobj(char* filename, triangulation_t& tri)
{
	ofstream outfile;
	map<voxelface_t, vertex_t*>::iterator vit;
	vector<triangle_t*>::iterator tit;
	int i;

	/* In order to export this mesh, we need to index the vertices.
	 * Since this is an ordered map of vertices, the iterator will 
	 * go through them in order. */
	tri.index_vertices();

	/* open file for writing */
	outfile.open(filename);
	if(!outfile.is_open())
		return -1;

	/* write description at top of file */
	outfile << "# This file generated using Surface Carving" << endl;
	outfile << "#" << endl;
	outfile << "# Vertices:  " << tri.vertices.size() << endl;
	outfile << "# Triangles: " << tri.triangles.size() << endl;
	outfile << "#" << endl;
	
	/* write vertices to disk */
	for(vit = tri.vertices.begin(); vit != tri.vertices.end(); vit++)
		outfile << "v " << (*vit).second->x 
			<<  " " << (*vit).second->y
			<<  " " << (*vit).second->z 
			<<  " " << ( (int) ( (*vit).second->red) )
			<<  " " << ( (int) ( (*vit).second->green) )
			<<  " " << ( (int) ( (*vit).second->blue) )
			<< endl;

	/* write triangles to disk (*.obj's index from 1) */
	for(tit = tri.triangles.begin(); tit != tri.triangles.end(); tit++)
	{
		/* write current triangle */
		outfile << "f";
		for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			outfile << " " << (1 + (*tit)->v[i]->index);
		outfile << endl;
	}

	/* clean up */
	outfile.close();
	return 0;
}

int writeobj_sorted(char* filename, triangulation_t& tri)
{
	ofstream outfile;
	map<voxelface_t, vertex_t*>::iterator vit;
	vector<triangle_t*>::iterator tit;
	queue<triangle_t*> to_write;
	triangle_t* current;
	int i;
	bool more_to_write;

	/* In order to export this mesh, we need to index the vertices.
	 * Since this is an ordered map of vertices, the iterator will 
	 * go through them in order. */
	tri.index_vertices();

	/* open file for writing */
	outfile.open(filename);
	if(!outfile.is_open())
		return -1;

	/* write description at top of file */
	outfile << "# This file generated using Surface Carving" << endl;
	outfile << "#" << endl;
	outfile << "# Vertices:  " << tri.vertices.size() << endl;
	outfile << "# Triangles: " << tri.triangles.size() << endl;
	outfile << "#" << endl;
	
	/* write vertices to disk */
	for(vit = tri.vertices.begin(); vit != tri.vertices.end(); vit++)
		outfile << "v " << (*vit).second->x 
			<<  " " << (*vit).second->y
			<<  " " << (*vit).second->z 
			<<  " " << ( (int) ( (*vit).second->red) )
			<<  " " << ( (int) ( (*vit).second->green) )
			<<  " " << ( (int) ( (*vit).second->blue) )
			<< endl;

	/* initialize triangle index parameters to zero.  For the following
	 * code, index values will be used to determine if the triangle
	 * has been written to disk (1) or if it has not been written to
	 * disk (0). */
	for(tit = tri.triangles.begin(); tit != tri.triangles.end(); tit++)
		(*tit)->index = 0;
	
	/* write triangles to disk.
	 * Search across mesh to write triangles in a semi-sorted order. */
	while(true)
	{
		/* start at first triangle that has not been written yet */
		more_to_write = false;
		for(tit = tri.triangles.begin(); 
				tit != tri.triangles.end(); tit++)
			if(((*tit)->index) == 0)
			{
				/* we found the next triangle to write */
				current = (*tit);
				more_to_write = true;
				break;
			}

		/* make sure we want to continue */
		if(!more_to_write)
			break;

		/* add current triangle to queue */
		to_write.push(current);

		/* keep going as long as queue is not empty */
		while(!to_write.empty())
		{
			/* get next triangle from queue */
			current = to_write.front();
			to_write.pop();

			/* make sure we want to write it */
			if(current->index > 0)
				continue;

			/* write current triangle (*.obj's index from 1) */
			outfile << "f";
			for(i = 0; i < NUM_VERTS_PER_TRI; i++)
				outfile << " " << (1+current->v[i]->index);
			outfile << endl;

			/* record that this triangle has been written */
			current->index++;

			/* add neighboring triangles to the queue */
			for(i = 0; i < NUM_EDGES_PER_TRI; i++)
				to_write.push( current->t[i] );
		}
	}

	/* clean up */
	outfile.close();
	return 0;
}

int writeply(char* filename, triangulation_t& tri, bool ascii)
{
	ofstream outfile;
	map<voxelface_t, vertex_t*>::iterator vit;
	vector<triangle_t*>::iterator tit;
	stringstream ss;
	union {
		float f;
		int i;
		uint32_t b;
		char arr[sizeof(uint32_t)];
	} buf;
	uint8_t c;
	unsigned int i;

	/* In order to export this mesh, we need to index the vertices.
	 * Since this is an ordered map of vertices, the iterator will 
	 * go through them in order. */
	tri.index_vertices();

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
	   << "property uchar red\n"
	   << "property uchar green\n"
	   << "property uchar blue\n"
	   << "element face "
	   << tri.triangles.size()
	   << "\nproperty list uchar int vertex_index\n"
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
			   << (int) ((*vit).second->red) << " "
			   << (int) ((*vit).second->green) << " "
			   << (int) ((*vit).second->blue) << "\n";
			outfile.write(ss.str().data(), ss.str().length());
		}
		else
		{
			/* write x coordinate */
			buf.f = (float) ( (*vit).second->x );
			buf.b = htole32(buf.b);
			outfile.write(buf.arr, sizeof(uint32_t));
	
			/* write y coordinate */
			buf.f = (float) ( (*vit).second->y );
			buf.b = htole32(buf.b);
			outfile.write(buf.arr, sizeof(uint32_t));
	
			/* write z coordinate */
			buf.f = (float) ( (*vit).second->z );
			buf.b = htole32(buf.b);
			outfile.write(buf.arr, sizeof(uint32_t));	
			
			/* write colors */
			outfile.write((char*) &((*vit).second->red), 
					sizeof(unsigned char));	
			outfile.write((char*) &((*vit).second->green), 
					sizeof(unsigned char));	
			outfile.write((char*) &((*vit).second->blue), 
					sizeof(unsigned char));	
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
				buf.b = htole32(buf.b);
				outfile.write(buf.arr, sizeof(uint32_t));
			}
		}
	}

	/* clean up */
	outfile.close();
	return 0;
}
