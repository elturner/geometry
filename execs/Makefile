# Meta-Makefile
#
# Written by Eric Turner <elturner@eecs.berkeley.edu>
# July 2, 2014
#
# This make file will compile all programs in this repo
# by calling their respective makefiles
#

all: floorplan_progs procarve_progs image_progs util_progs
	@echo ""
	@echo "All programs compiled successfully"

# programs needed for generating floorplans
floorplan_progs:
	@mkdir -p ../bin # ensure output directory exists
	@echo ""
	@echo "Compiling floorplan programs..."
	@echo ""
	$(MAKE) -w -C pointcloud_generation $(CMD)
	$(MAKE) -w -C partition_pointcloud_levels $(CMD)
	$(MAKE) -w -C xyz2dq $(CMD)
	$(MAKE) -w -C dq_floorplan $(CMD)
	$(MAKE) -w -C fp2model $(CMD)

# programs needed to generate carvings
procarve_progs:
	@mkdir -p ../bin # ensure output directory exists
	@echo ""
	@echo "Compiling procarve programs..."
	@echo ""
	$(MAKE) -w -C filter_urg_scans $(CMD)
	$(MAKE) -w -C filter_tango_scans $(CMD)
	$(MAKE) -w -C generate_noisypath $(CMD)
	$(MAKE) -w -C wedge_generator $(CMD)
	$(MAKE) -w -C scan_chunker $(CMD)
	$(MAKE) -w -C procarve $(CMD)
	$(MAKE) -w -C oct2dq $(CMD)
	$(MAKE) -w -C generate_hia $(CMD)
	$(MAKE) -w -C fp_optimizer $(CMD)
	$(MAKE) -w -C merge_fp_oct $(CMD)
	$(MAKE) -w -C octsurf $(CMD)

# image manipulation programs
image_progs:
	@mkdir -p ../bin # ensure output directory exists
	@echo ""
	@echo "Compiling image processing programs..."
	@echo ""
	$(MAKE) -w -C rectify_images $(CMD)
	$(MAKE) -w -C mesh2image $(CMD)
	$(MAKE) -w -C generate_imap $(CMD)
	$(MAKE) -w -C generate_scanorama $(CMD)

# utility programs
util_progs:
	@mkdir -p ../bin # ensure output directory exists
	@echo ""
	@echo "Compiling utility programs..."
	@echo ""
	#$(MAKE) -w -C align_path  # deprecated, use localization repo
	$(MAKE) -w -C generate_tex $(CMD)
	#$(MAKE) -w -C simplify_fp
	$(MAKE) -w -C wedge2xyz $(CMD)
	$(MAKE) -w -C generate_msd $(CMD)
	$(MAKE) -w -C transform_model $(CMD)
	$(MAKE) -w -C filter_pointcloud $(CMD)
	$(MAKE) -w -C find_doors $(CMD)
	$(MAKE) -w -C screenshot_pointcloud $(CMD)
	$(MAKE) -w -C split_image_by_floorplan $(CMD)
	$(MAKE) -w -C surface_carve $(CMD)

# todo searcher
todo:
	@grep -n --color=auto -R --exclude=Makefile --exclude-dir=boost --exclude-dir=pint --exclude-dir=lodepng "TODO" ../src/ .

grep:
	@grep -n --color=auto -R --exclude=Makefile --exclude-dir=boost --exclude-dir=pint --exclude-dir=lodepng "${SEARCH}" ../src/ .

