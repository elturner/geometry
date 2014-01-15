#ifndef PARAMETERS_H
#define PARAMETERS_H

/*** DEFAULT SETTINGS ***/

/* the following specifies the default units for pointcloud files (mm) */
#define XYZ_DEFAULT_UNITS 1000

/* the default resolution to use, in units of meters */
#define DEFAULT_RESOLUTION 0.05

/* the default floor height represents the minimum spacing between floors,
 * expressed in units of meters */
#define DEFAULT_MIN_FLOOR_HEIGHT 2.0

/*** GEOMETRY ***/

/* number of dimensions used in this code */
#define NUM_DIMS 3

/*** UNITS ***/

/* convert from millimeters to meters */
#define MM2METERS(x) ( 0.001 * (x) )

/* the following macro converts from degrees to radians */
#define DEG2RAD(x) ( (x) * 3.14159265358979323846264 / 180.0 )

#endif
