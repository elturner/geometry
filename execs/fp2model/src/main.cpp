/* main.cpp:
 *
 *	This program converts a .fp file along with additional building
 *	information into models of various formats.
 */

#include <iostream>
#include <vector>
#include "structs/building_model.h"
#include "io/config.h"
#include "process/parse_input.h"
#include "process/export_data.h"

using namespace std;

int main(int argc, char** argv)
{
	config_t conf;
	building_model_t bim;
	int ret;

	/* read command-line args */
	ret = conf.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": Unable to parse "
		     << "command-line" << endl;
		return 1;
	}

	/* parse the input floorplans */
	ret = parse_input(bim, conf);
	if(ret)
	{
		cerr << "ERROR: unable to parse input, ret: "
		     << ret << endl;
		return 2;
	}
	
	/* write the data to specified output */
	ret = export_data(bim, conf);
	if(ret)
	{
		cerr << "[main]\tERROR " << ret << ": unable to "
		     << "export data" << endl;
		return 3;
	}

	/* success */
	return 0;
}
