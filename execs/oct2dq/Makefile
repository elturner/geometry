CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -std=c++0x
LFLAGS = -lm
PFLAGS = #-pg -fprofile-arcs
SOURCEDIR = ../../src/cpp/
EIGENDIR = /usr/include/eigen3/
IFLAGS = -I$(SOURCEDIR) -I$(EIGENDIR)
BUILDDIR = build/src/cpp
EXECUTABLE = ../../bin/oct2dq

# defines for the program

SOURCES =	$(SOURCEDIR)util/tictoc.cpp \
		$(SOURCEDIR)util/cmd_args.cpp \
		$(SOURCEDIR)util/progress_bar.cpp \
		$(SOURCEDIR)util/union_find.cpp \
		$(SOURCEDIR)util/range_list.cpp \
		$(SOURCEDIR)util/rotLib.cpp \
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
		$(SOURCEDIR)io/carve/noisypath_io.cpp \
		$(SOURCEDIR)io/data/fss/fss_io.cpp \
		$(SOURCEDIR)geometry/system_path.cpp \
		$(SOURCEDIR)geometry/transform.cpp \
		$(SOURCEDIR)geometry/octree/octtopo.cpp \
		$(SOURCEDIR)geometry/octree/octree.cpp \
		$(SOURCEDIR)geometry/octree/octnode.cpp \
		$(SOURCEDIR)geometry/octree/octdata.cpp \
		$(SOURCEDIR)geometry/quadtree/quadtree.cpp \
		$(SOURCEDIR)geometry/quadtree/quadnode.cpp \
		$(SOURCEDIR)geometry/quadtree/quaddata.cpp \
		$(SOURCEDIR)geometry/shapes/plane.cpp \
		$(SOURCEDIR)mesh/refine/octree_padder.cpp \
		$(SOURCEDIR)mesh/partition/node_partitioner.cpp \
		$(SOURCEDIR)mesh/partition/node_set.cpp \
		$(SOURCEDIR)mesh/surface/node_boundary.cpp \
		$(SOURCEDIR)mesh/surface/planar_region.cpp \
		$(SOURCEDIR)mesh/surface/planar_region_graph.cpp \
		$(SOURCEDIR)mesh/wall_sampling/wall_sampling.cpp \
		src/oct2dq_run_settings.cpp \
		src/wall_region_info.cpp \
		src/process.cpp \
		src/main.cpp

HEADERS =	$(SOURCEDIR)util/error_codes.h \
		$(SOURCEDIR)util/tictoc.h \
		$(SOURCEDIR)util/cmd_args.h \
		$(SOURCEDIR)util/endian.h \
		$(SOURCEDIR)util/progress_bar.h \
		$(SOURCEDIR)util/union_find.h \
		$(SOURCEDIR)util/range_list.h \
		$(SOURCEDIR)util/rotLib.h \
		$(SOURCEDIR)config/backpackConfig.h \
		$(SOURCEDIR)config/cameraProp.h \
		$(SOURCEDIR)config/imuProp.h \
		$(SOURCEDIR)config/laserProp.h \
		$(SOURCEDIR)config/tofProp.h \
		$(SOURCEDIR)config/sensorProp.h \
		$(SOURCEDIR)xmlreader/tinystr.h \
		$(SOURCEDIR)xmlreader/tinyxml.h \
		$(SOURCEDIR)xmlreader/tinyxmltools.h \
		$(SOURCEDIR)xmlreader/xmlsettings.h \
		$(SOURCEDIR)io/carve/noisypath_io.h \
		$(SOURCEDIR)io/data/fss/fss_io.h \
		$(SOURCEDIR)geometry/system_path.h \
		$(SOURCEDIR)geometry/transform.h \
		$(SOURCEDIR)geometry/octree/octtopo.h \
		$(SOURCEDIR)geometry/octree/octree.h \
		$(SOURCEDIR)geometry/octree/octnode.h \
		$(SOURCEDIR)geometry/octree/shape.h \
		$(SOURCEDIR)geometry/octree/octdata.h \
		$(SOURCEDIR)geometry/quadtree/quadtree.h \
		$(SOURCEDIR)geometry/quadtree/quadnode.h \
		$(SOURCEDIR)geometry/quadtree/quaddata.h \
		$(SOURCEDIR)geometry/poly_intersect/poly2d.h \
		$(SOURCEDIR)geometry/shapes/shape_wrapper.h \
		$(SOURCEDIR)geometry/shapes/linesegment.h \
		$(SOURCEDIR)geometry/shapes/linesegment_2d.h \
		$(SOURCEDIR)geometry/shapes/plane.h \
		$(SOURCEDIR)mesh/refine/octree_padder.h \
		$(SOURCEDIR)mesh/partition/node_partitioner.h \
		$(SOURCEDIR)mesh/partition/node_set.h \
		$(SOURCEDIR)mesh/surface/node_boundary.h \
		$(SOURCEDIR)mesh/surface/planar_region.h \
		$(SOURCEDIR)mesh/surface/planar_region_graph.h \
		$(SOURCEDIR)mesh/wall_sampling/wall_sampling.h \
		src/oct2dq_run_settings.h \
		src/wall_region_info.h \
		src/process.h

OBJECTS = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SOURCES))

# compile commands

all: $(SOURCES) $(EXECUTABLE)
	make --no-builtin-rules --no-builtin-variables $(EXECUTABLE)

simple:
	$(CC) $(IFLAGS) $(CFLAGS) $(LFLAGS) $(PFLAGS) $(SOURCES) -o $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LFLAGS) $(PFLAGS) $(IFLAGS)

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

