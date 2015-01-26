# Meta-Makefile
#
# Written by Eric Turner <elturner@eecs.berkeley.edu>
# July 2, 2014
#
# This make file will compile all programs in this repo
# by calling their respective makefiles
#

all: floorplan_progs procarve_progs util_progs
	@echo ""
	@echo "All programs compiled successfully"

# programs needed for generating floorplans
floorplan_progs:
	@mkdir -p ../bin # ensure output directory exists
	@echo ""
	@echo "Compiling floorplan programs..."
	@echo ""
	$(MAKE) -w -C pointcloud_generation
	$(MAKE) -w -C partition_pointcloud_levels
	$(MAKE) -w -C xyz2dq
	$(MAKE) -w -C dq_floorplan
	$(MAKE) -w -C fp2model

# programs needed to generate carvings
procarve_progs:
	@mkdir -p ../bin # ensure output directory exists
	@echo ""
	@echo "Compiling procarve programs..."
	@echo ""
	$(MAKE) -w -C filter_urg_scans
	$(MAKE) -w -C generate_noisypath
	$(MAKE) -w -C wedge_generator
	$(MAKE) -w -C scan_chunker
	$(MAKE) -w -C procarve
	$(MAKE) -w -C oct2dq
	$(MAKE) -w -C fp_optimizer
	$(MAKE) -w -C merge_fp_oct
	$(MAKE) -w -C octsurf

# utility programs
util_progs:
	@mkdir -p ../bin # ensure output directory exists
	@echo ""
	@echo "Compiling utility programs..."
	@echo ""
	#$(MAKE) -w -C align_path  # deprecated, use localization repo
	$(MAKE) -w -C generate_tex
	#$(MAKE) -w -C simplify_fp
	$(MAKE) -w -C wedge2xyz
	$(MAKE) -w -C generate_msd
	$(MAKE) -w -C transform_model
	$(MAKE) -w -C filter_pointcloud
	$(MAKE) -w -C surface_carve
