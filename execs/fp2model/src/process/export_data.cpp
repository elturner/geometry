#include "export_data.h"
#include "../io/config.h"
#include "../structs/building_model.h"
#include "../util/error_codes.h"
#include "../util/tictoc.h"

int export_data(building_model_t& bim, config_t& conf)
{
	tictoc_t clk;
	int ret;

	/* start the clock */
	tic(clk);

	/* export specified files */
	if(conf.obj_outfile)
	{
		/* write extruded mesh to obj format */
		ret = bim.export_obj(conf.obj_outfile);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
	}

	/* success */
	toc(clk, "Exporting data");
	return 0;
}
