#include "parse_input.h"
#include "../io/config.h"
#include "../structs/building_model.h"
#include "../util/error_codes.h"
#include "../util/tictoc.h"

int parse_input(building_model_t& bim, config_t& conf)
{
	int ret;
	tictoc_t clk;

	/* start timing */
	tic(clk);

	/* clear struct of any stored data */
	bim.clear();

	/* read the next floorplan */
	ret = bim.import_floorplan(conf.fp_infile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* read windows info, if available */
	if(conf.windows_infile)
	{
		ret = bim.import_windows(conf.windows_infile);
		if(ret)
			return PROPEGATE_ERROR(-2, ret);
	}

	/* success */
	toc(clk, "Parsing input");
	return 0;
}
