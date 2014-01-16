#ifndef PARAMETERS_H
#define PARAMETERS_H

/*** GEOMETRY ***/

/* number of dimensions used in this code */
#define NUM_DIMS 3

/* define edge */
#define NUM_VERTS_PER_EDGE 2

/* define triangle */
#define NUM_VERTS_PER_TRI 3
#define NUM_EDGES_PER_TRI 3

/* define rectangle */
#define NUM_VERTS_PER_RECT 4

/*** UNITS ***/

/* convert from millimeters to meters */
#define MM2METERS(x) ( 0.001 * (x) )

/* the following macro converts from degrees to radians */
#define DEG2RAD(x) ( (x) * 3.14159265358979323846264 / 180.0 )

#endif
