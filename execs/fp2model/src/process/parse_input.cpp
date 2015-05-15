#include "parse_input.h"
#include "../io/config.h"
#include "../structs/building_model.h"
#include <util/error_codes.h>
#include <util/tictoc.h>

int parse_input(building_model_t& bim, const config_t& conf)
{
	int ret, i, n;
	tictoc_t clk;

	/* start timing */
	tic(clk);

	/* clear struct of any stored data */
	bim.clear();

	/* read the next floorplan */
	ret = bim.import_floorplan(conf.fp_infile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	bim.level_name = conf.level_name;

	/* read windows info, if available */
	n = conf.windows_infiles.size();
	for(i = 0; i < n; i++)
	{
		/* import windows from file */
		ret = bim.import_windows(conf.windows_infiles[i]);
		if(ret)
			return PROPEGATE_ERROR(-2, ret);
	}

	/* read lights info, if available */
	n = conf.lights_infiles.size();
	for(i = 0; i < n; i++)
	{
		/* import lights from file */
		ret = bim.import_lights(conf.lights_infiles[i]);
		if(ret)
			return PROPEGATE_ERROR(-3, ret);
	}
	
	/* read people info, if available */
	n = conf.people_infiles.size();
	for(i = 0; i < n; i++)
	{
		/* import people from file */
		ret = bim.import_people(conf.people_infiles[i]);
		if(ret)
			return PROPEGATE_ERROR(-4, ret);
	}
	
	/* read plugloads info, if available */
	n = conf.plugloads_infiles.size();
	for(i = 0; i < n; i++)
	{
		/* import plugloads from file */
		ret = bim.import_plugloads(conf.plugloads_infiles[i]);
		if(ret)
			return PROPEGATE_ERROR(-5, ret);
	}

	/* success */
	toc(clk, "Parsing input");
	return 0;
}
