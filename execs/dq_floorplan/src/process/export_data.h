#ifndef EXPORT_DATA_H
#define EXPORT_DATA_H

/* export_data.h:
 *
 * This file contains functions used for exporting the
 * information processed in this program to various file
 * formats.
 */

#include "../structs/cell_graph.h"
#include "../rooms/tri_rep.h"
#include "../io/config.h"

/* export_data:
 *
 * 	Will export the specified graph to the output file
 * 	defined in the configuration.  The format exported
 * 	depends on the parsed type of the file.
 *
 * arguments:
 *
 * 	graph -		The graph information to export.
 * 	trirep -	The triangulation information of this graph.
 * 	conf -		The configuration to use.
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int export_data(cell_graph_t& graph, tri_rep_t& trirep, config_t& conf);

#endif
