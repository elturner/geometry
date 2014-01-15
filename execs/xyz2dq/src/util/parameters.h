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

/********** QUADTREE DEFINES *************/

#define DEFAULT_QUADTREE_RESOLUTION 0.05 /* meters */

#define NUM_DIMS 2
#define CHILDREN_PER_NODE 4

#define DEFAULT_MIN_WALL_HEIGHT 2.0 /* meters */
#define VERTICAL_BIN_SIZE 0.1 /* meters */
#define DEFAULT_MIN_NUM_POINTS_PER_WALL_SAMPLE 50

#endif
