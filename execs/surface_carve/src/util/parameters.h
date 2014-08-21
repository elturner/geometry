#ifndef PARAMETERS_H
#define PARAMETERS_H

/*********** FILE FORMATS ***************/

/* the following defines are meant to 
 * define the buffer used to store a single
 * line from an infile formatted as an ascii
 * XYZ file */
#define LINE_BUFFER_SIZE 256
#define NUM_ELEMENTS_PER_LINE 9
#define XYZ_FORMAT_STRING "%lf %lf %lf %d %d %d %d %lf %d"

/* the following specifies how many scans to read in
 * each chunk of a point-cloud file.  Each scan is represented
 * by up to thousands of points. 
 *
 * If recompiling this code on a different machine, you may
 * want to change this number to fit into your machine's memory */
#define NUM_SCANS_PER_FILE_CHUNK 8000
#define OVERLAP_PER_FILE_CHUNK 3

/*********** UNIT CONVERSIONS ****************/

/* converts from millimeters to meters */
#define MM2METERS(x) ( 0.001 * (x) )

/* how many bits in one byte */
#define BYTES2BITS(x) ( 8 * (x) )

/* get the i'th bit in a variable x */
#define BITGET(x,i) ( ((x) >> (i)) & 1 )

/* the maxiumum byte value (all ones) */
#define MAX_BYTE 255

/* convert from degrees to radians */
#define DEG2RAD(x) ( (x) * 3.14159265358979323846264 / 180.0 )

/*********** GEOMETRY ********************/

#define NUM_DIMS 3

/* Definition of an edge */
#define NUM_VERTS_PER_EDGE 2

/* Definition of triangles */
#define NUM_VERTS_PER_TRI 3
#define NUM_EDGES_PER_TRI 3

/* Definition of a cube */
#define NUM_FACES_PER_CUBE 6
#define NUM_CORNERS_PER_CUBE 8
#define NUM_FACES_PER_CORNER_PER_CUBE 3
#define NUM_EDGES_PER_CORNER_PER_CUBE 3

/* Definition of a square */
#define NUM_VERTS_PER_SQUARE 4
#define NUM_EDGES_PER_SQUARE 4

/* parameters used by marching cubes implementation */
#define MAX_TRIANGLES_PER_CUBE 5
#define MARCHING_CUBES_CASE_LEN ((NUM_VERTS_PER_TRI \
			* MAX_TRIANGLES_PER_CUBE) + 1)

/* cos(theta) threshold to consider theta "very small" */
#define PARALLEL_THRESHOLD 0.97 /* 0.9998 corresponds to ~ 1 degree */
#define PERPENDICULAR_THRESHOLD 0.09 /* 0.087 corresponds to ~ 85 degrees */
#define APPROX_ZERO 0.000001 /* really small number */

/* a polygon composed of less than this many triangles
 * can only have at most one interior vertex */
#define MIN_NUM_TRIS_PER_REGION 4

/********** SCANNING SYSTEM *****************/

/* The following is the default distance that range
 * measurements are limited to.  By default, any range
 * measurements further from the scanner than this distance
 * will only be carved up to this distance */
#define DEFAULT_MAX_SCAN_DISTANCE_SQ 225 /* units: meters^2 */

/* The following is the default size of a voxel, in meters */
#define DEFAULT_VOXEL_RESOLUTION 0.05

/* The following describes how much to truncate a scan point when carving,
 * proportional to voxel resolution size */
#define VOXEL_BIAS_FRACTION 1.0

/*********** ARCHITECTURE ********************/

/* How many neighbors a voxel must have to survive cleanup */
#define GRID_CLEANUP_FACE_THRESHOLD 3
#define DEGENERATE_FACE_THRESHOLD 3

/* how many voxel sizes a geometry element can be from a plane,
 * and still be the discretization of that plane */
#define VOXEL_FACE_MAX_ERR_THRESHOLD 1
#define VOXEL_FACE_MAX_ERR_BOUNDARY_THRESHOLD 3

/* small connected components in mesh defined by this threshold */
#define MIN_MESH_UNION_SIZE 10000
#define MIN_SNAP_REGION_SIZE 100

/* How many rounds of smoothing to do after simplification */
#define SIMPLIFICATION_SMOOTHING_ROUNDS 10

/* for coalescing region definitions (but not changing geometry),
 * this cos(angle) is used as a merging threshold */
#define COALESCE_REGIONS_THRESHOLD 0

#endif
