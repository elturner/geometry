CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -ansi
LFLAGS = -lm #-lpthread
PFLAGS = #-pg
SOURCEDIR = ../../src/cpp/
EIGENDIR = /usr/include/eigen3/
IFLAGS = -I$(SOURCEDIR) -I$(EIGENDIR)
BUILDDIR = build/src/cpp
EXECUTABLE = ../../bin/floorplan_gen

# defines for the program

SOURCES =	$(SOURCEDIR)util/range_list.cpp \
		$(SOURCEDIR)util/rotLib.cpp \
		$(SOURCEDIR)io/carve/noisypath_io.cpp \
		$(SOURCEDIR)config/backpackConfig.cpp \
		$(SOURCEDIR)config/cameraProp.cpp \
		$(SOURCEDIR)config/imuProp.cpp \
		$(SOURCEDIR)config/laserProp.cpp \
		$(SOURCEDIR)config/tofProp.cpp \
		$(SOURCEDIR)config/flirProp.cpp \
		$(SOURCEDIR)xmlreader/tinystr.cpp \
		$(SOURCEDIR)xmlreader/tinyxml.cpp \
		$(SOURCEDIR)xmlreader/tinyxmlerror.cpp \
		$(SOURCEDIR)xmlreader/tinyxmlparser.cpp \
		$(SOURCEDIR)xmlreader/tinyxmltools.cpp \
		$(SOURCEDIR)xmlreader/xmlsettings.cpp \
		$(SOURCEDIR)geometry/system_path.cpp \
		$(SOURCEDIR)geometry/transform.cpp \
		src/util/tictoc.cpp \
		src/util/progress_bar.cpp \
		src/io/filetypes.cpp \
		src/io/config.cpp \
		src/io/dq_io.cpp \
		src/io/mesh_io.cpp \
		src/io/bim_io.cpp \
		src/io/fp_io.cpp \
		src/structs/quadtree.cpp \
		src/structs/point.cpp \
		src/structs/cell_graph.cpp \
		src/structs/triple.cpp \
		src/delaunay/insertion.cpp \
		src/delaunay/reordering.cpp \
		src/delaunay/triangulation/geometry.cpp \
		src/delaunay/triangulation/linkring.cpp \
		src/delaunay/triangulation/triangulation.cpp \
		src/delaunay/triangulation/vertex.cpp \
		src/rooms/tri_rep.cpp \
		src/process/generate_boundary.cpp \
		src/process/simplify_graph.cpp \
		src/process/export_data.cpp \
		src/test/test_trirep.cpp \
		src/main.cpp

HEADERS =	$(SOURCEDIR)util/range_list.h \
		$(SOURCEDIR)util/error_codes.h \
		$(SOURCEDIR)util/endian.h \
		$(SOURCEDIR)util/rotLib.h \
		$(SOURCEDIR)util/binary_search.h \
		$(SOURCEDIR)io/carve/noisypath_io.h \
		$(SOURCEDIR)config/backpackConfig.h \
		$(SOURCEDIR)config/cameraProp.h \
		$(SOURCEDIR)config/imuProp.h \
		$(SOURCEDIR)config/laserProp.h \
		$(SOURCEDIR)config/sensorProp.h \
		$(SOURCEDIR)config/tofProp.h \
		$(SOURCEDIR)config/flirProp.h \
		$(SOURCEDIR)xmlreader/tinystr.h \
		$(SOURCEDIR)xmlreader/tinyxml.h \
		$(SOURCEDIR)xmlreader/tinyxmltools.h \
		$(SOURCEDIR)xmlreader/xmlsettings.h \
		$(SOURCEDIR)geometry/system_path.h \
		$(SOURCEDIR)geometry/transform.h \
		src/util/error_codes.h \
		src/util/tictoc.h \
		src/util/progress_bar.h \
		src/util/constants.h \
		src/util/room_parameters.h \
		src/io/filetypes.h \
		src/io/config.h \
		src/io/dq_io.h \
		src/io/mesh_io.h \
		src/io/bim_io.h \
		src/io/fp_io.h \
		src/structs/quadtree.h \
		src/structs/point.h \
		src/structs/normal.h \
		src/structs/parameters.h \
		src/structs/cell_graph.h \
		src/structs/triple.h \
		src/delaunay/insertion.h \
		src/delaunay/reordering.h \
		src/delaunay/triangulation/geometry.h \
		src/delaunay/triangulation/linkring.h \
		src/delaunay/triangulation/triangulation.h \
		src/delaunay/triangulation/vertex.h \
		src/rooms/tri_rep.h \
		src/process/generate_boundary.h \
		src/process/simplify_graph.h \
		src/process/export_data.h \
		src/test/test_trirep.h

OBJECTS = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SOURCES))

# compile commands

all: $(SOURCES) $(EXECUTABLE)
	make --no-builtin-rules --no-builtin-variables $(EXECUTABLE)

simple:
	$(CC) $(CFLAGS) $(IFLAGS) $(SOURCES) -o $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LFLAGS) $(PFLAGS) $(IFLAGS)

$(BUILDDIR)/%.o : %.cpp
	@mkdir -p $(shell dirname $@)		# ensure folder exists
	@g++ -MM -MF $(patsubst %.o,%.d,$@) -MT $@ $< # recalc depends
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

