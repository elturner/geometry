CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -std=c++0x
LFLAGS = -lm
PFLAGS = #-pg
SOURCEDIR = ../../src/cpp/
EIGENDIR = /usr/include/eigen3/
IFLAGS = -I$(SOURCEDIR) -I$(EIGENDIR)
BUILDDIR = build/src/cpp
EXECUTABLE = ../../bin/wedge_gen

# defines for the program

SOURCES =	$(SOURCEDIR)util/tictoc.cpp \
		$(SOURCEDIR)util/cmd_args.cpp \
		$(SOURCEDIR)util/rotLib.cpp \
		$(SOURCEDIR)util/randLib.cpp \
		$(SOURCEDIR)util/range_list.cpp \
		$(SOURCEDIR)util/progress_bar.cpp \
		$(SOURCEDIR)config/backpackConfig.cpp \
		$(SOURCEDIR)config/cameraProp.cpp \
		$(SOURCEDIR)config/imuProp.cpp \
		$(SOURCEDIR)config/laserProp.cpp \
		$(SOURCEDIR)config/tofProp.cpp \
		$(SOURCEDIR)xmlreader/tinystr.cpp \
		$(SOURCEDIR)xmlreader/tinyxml.cpp \
		$(SOURCEDIR)xmlreader/tinyxmlerror.cpp \
		$(SOURCEDIR)xmlreader/tinyxmlparser.cpp \
		$(SOURCEDIR)xmlreader/tinyxmltools.cpp \
		$(SOURCEDIR)xmlreader/xmlsettings.cpp \
		$(SOURCEDIR)timestamp/sync_xml.cpp \
		$(SOURCEDIR)io/data/fss/fss_io.cpp \
		$(SOURCEDIR)io/carve/wedge_io.cpp \
		$(SOURCEDIR)io/carve/chunk_io.cpp \
		$(SOURCEDIR)io/carve/carve_map_io.cpp \
		$(SOURCEDIR)io/carve/noisypath_io.cpp \
		$(SOURCEDIR)geometry/system_path.cpp \
		$(SOURCEDIR)geometry/transform.cpp \
		$(SOURCEDIR)geometry/octree/octree.cpp \
		$(SOURCEDIR)geometry/octree/octnode.cpp \
		$(SOURCEDIR)geometry/octree/octdata.cpp \
		$(SOURCEDIR)geometry/shapes/carve_wedge.cpp \
		$(SOURCEDIR)geometry/shapes/chunk_exporter.cpp \
		$(SOURCEDIR)geometry/carve/wedge_generator.cpp \
		$(SOURCEDIR)geometry/carve/frame_model.cpp \
		$(SOURCEDIR)geometry/carve/gaussian/noisy_scanpoint.cpp \
		$(SOURCEDIR)geometry/carve/gaussian/scan_model.cpp \
		$(SOURCEDIR)geometry/carve/gaussian/carve_map.cpp \
		$(SOURCEDIR)geometry/pca/line_fit.cpp \
		$(SOURCEDIR)geometry/poly_intersect/fpcube.cpp \
		$(SOURCEDIR)geometry/poly_intersect/pcube.cpp \
		$(SOURCEDIR)geometry/poly_intersect/get_polygon_normal.cpp \
		src/wedge_run_settings.cpp \
		src/main.cpp

HEADERS =	$(SOURCEDIR)util/error_codes.h \
		$(SOURCEDIR)util/tictoc.h \
		$(SOURCEDIR)util/cmd_args.h \
		$(SOURCEDIR)util/endian.h \
		$(SOURCEDIR)util/rotLib.h \
		$(SOURCEDIR)util/randLib.h \
		$(SOURCEDIR)util/range_list.h \
		$(SOURCEDIR)util/progress_bar.h \
		$(SOURCEDIR)util/binary_search.h \
		$(SOURCEDIR)config/backpackConfig.h \
		$(SOURCEDIR)config/cameraProp.h \
		$(SOURCEDIR)config/imuProp.h \
		$(SOURCEDIR)config/laserProp.h \
		$(SOURCEDIR)config/sensorProp.h \
		$(SOURCEDIR)config/tofProp.h \
		$(SOURCEDIR)xmlreader/tinystr.h \
		$(SOURCEDIR)xmlreader/tinyxml.h \
		$(SOURCEDIR)xmlreader/tinyxmltools.h \
		$(SOURCEDIR)xmlreader/xmlsettings.h \
		$(SOURCEDIR)timestamp/sync_xml.h \
		$(SOURCEDIR)io/data/fss/fss_io.h \
		$(SOURCEDIR)io/carve/wedge_io.h \
		$(SOURCEDIR)io/carve/chunk_io.h \
		$(SOURCEDIR)io/carve/carve_map_io.h \
		$(SOURCEDIR)io/carve/noisypath_io.h \
		$(SOURCEDIR)geometry/system_path.h \
		$(SOURCEDIR)geometry/transform.h \
		$(SOURCEDIR)geometry/octree/shape.h \
		$(SOURCEDIR)geometry/octree/octree.h \
		$(SOURCEDIR)geometry/octree/octnode.h \
		$(SOURCEDIR)geometry/octree/octdata.h \
		$(SOURCEDIR)geometry/shapes/carve_wedge.h \
		$(SOURCEDIR)geometry/shapes/chunk_exporter.h \
		$(SOURCEDIR)geometry/carve/wedge_generator.h \
		$(SOURCEDIR)geometry/carve/frame_model.h \
		$(SOURCEDIR)geometry/carve/gaussian/noisy_scanpoint.h \
		$(SOURCEDIR)geometry/carve/gaussian/scan_model.h \
		$(SOURCEDIR)geometry/carve/gaussian/carve_map.h \
		$(SOURCEDIR)geometry/pca/line_fit.h \
		$(SOURCEDIR)geometry/poly_intersect/poly2d.h \
		$(SOURCEDIR)geometry/poly_intersect/pcube.h \
		$(SOURCEDIR)geometry/poly_intersect/get_polygon_normal.h \
		$(SOURCEDIR)geometry/poly_intersect/vec.h \
		src/wedge_run_settings.h

OBJECTS = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SOURCES))

# compile commands

all: $(SOURCES) $(EXECUTABLE)
	make --no-builtin-rules --no-builtin-variables $(EXECUTABLE)

simple:
	$(CC) $(IFLAGS) $(CFLAGS) $(LFLAGS) $(PFLAGS) $(SOURCES) -o $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LFLAGS) $(PFLAGS) $(IFLAGS) $(CFLAGS)

$(BUILDDIR)/%.o : %.cpp
	@mkdir -p $(shell dirname $@)		# ensure folder exists
	@g++ -std=c++0x -MM -MF $(patsubst %.o,%.d,$@) -MT $@ $< # recalc depends
	$(CC) -c $(CFLAGS) $(IFLAGS) $< -o $@

# helper commands

todo:
	grep -n --color=auto "TODO" $(SOURCES) $(HEADERS)

grep:
	grep -n --color=auto "$(SEARCH)" $(SOURCES) $(HEADERS)

size:
	wc $(SOURCES) $(HEADERS)

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) $(BUILDDIR) $(EXECUTABLE).dSYM

# include full recalculated dependencies
-include $(OBJECTS:.o=.d)

