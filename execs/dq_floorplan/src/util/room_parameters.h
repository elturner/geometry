#ifndef ROOM_PARAMETERS_H
#define ROOM_PARAMETERS_H

/*** ADA Compliance Requirements ***/

/* the following denote dimensional constraints on
 * doors and rooms by ADA compliance standards
 *
 * sources:
 *
 *	www.trustile.com/techinfo/ada.asp
 *	www.access-board.gov/adaag/html/adaag.htm
 *
 *	ANSI A117.1-1980
 */
#define ADA_MIN_DOOR_HEIGHT        2.032  /* units: meters (80 inches) */
#define ADA_MIN_DOOR_OPENING_WIDTH 0.8128 /* units: meters (32 inches) */
#define ADA_MAX_DOOR_OPENING_WIDTH 1.2192 /* units: meters (48 inches) */
#define ADA_MIN_HALLWAY_WIDTH      1.2192 /* units: meters (48 inches) */
#define ADA_CLEAR_FLOOR_MIN_LENGTH 1.2192 /* units: meters (48 inches) */
#define ADA_WHEELCHAIR_TURN_SPACE  1.524  /* units: meters (60 inches) */
#define ADA_PASSING_SPACE          1.524  /* units: meters (60 inches) */

/*** ROOM PARTITIONING PARAMETERS ***/

/* the following denotes the minimum perimeter of a room (in meters) */
#define MIN_ROOM_PERIMETER (ADA_PASSING_SPACE + ADA_PASSING_SPACE \
				+ ADA_PASSING_SPACE + ADA_PASSING_SPACE )

/* The minimum valid column in a building should have at least this
 * perimeter
 *
 * http://books.google.com/books?id=92PwXW0f81QC&lpg=PA266&ots=3BWptrM8xp&dq=building%20codes%20minimum%20column&pg=PA266#v=onepage&q=building%20codes%20minimum%20column&f=false
 * */
#define MIN_COLUMN_PERIMETER 1.016 /* units: m (4 sides * 10 inches/side) */

/* the following denotes the smallest circumradius a triangle
 * can have and still be considered a seed for room classification
 *
 * This assumes the local max circumcircle will be at least half the
 * size of the smallest valid room. */
#define MIN_LOCAL_MAX_CIRCUMRADIUS (ADA_PASSING_SPACE/4)

/* the following denotes the maximum width of a door.  Any boundary
 * between two rooms that is longer than this denotes that the rooms
 * should be merged into one. 
 *
 * This number chosen since some rooms have double doors */
#define MAX_DOOR_WIDTH (2*ADA_MAX_DOOR_OPENING_WIDTH)

#endif
