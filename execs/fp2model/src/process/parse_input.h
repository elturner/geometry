#ifndef PARSE_INPUT_H
#define PARSE_INPUT_H

/* parse_input.h:
 *
 * 	Will read the input
 * 	files specified on the
 * 	command-line into the
 * 	given structs.
 */

#include "../io/config.h"
#include "../structs/building_model.h"

/* parse_input:
 *
 * 	Given a configuration struct, generated
 * 	from the command-line, will read a floorplan
 * 	into the specified struct.
 *
 * argument:
 *
 *	bim -		Where to store the building info
 *	conf -		The configuration to use.
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int parse_input(building_model_t& bim, const config_t& conf);

#endif
