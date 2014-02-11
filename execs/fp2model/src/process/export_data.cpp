#include "export_data.h"
#include "../io/config.h"
#include "../io/filetypes.h"
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
	switch(conf.output_type)
	{
		/* wavefront obj files */
		case obj_file:
			/* write extruded mesh to obj format */
			ret = bim.export_obj(conf.outfile);
			if(ret)
				return PROPEGATE_ERROR(-1, ret);
			break;

		/* vmrl files */
		case wrl_file:
			/* write extruded mesh to wrl format */
			ret = bim.export_wrl(conf.outfile);
			if(ret)
				return PROPEGATE_ERROR(-2, ret);
			break;
		
		/* unknown files */
		default:
			return -3;
	}

	/* success */
	toc(clk, "Exporting data");
	return 0;
}
