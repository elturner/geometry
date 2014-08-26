CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -std=c++0x
LFLAGS = -lm
PFLAGS = #-pg
SOURCEDIR = ../../src/cpp/
EIGENDIR = /usr/include/eigen3/
IFLAGS = -I$(SOURCEDIR) -I$(EIGENDIR)
BUILDDIR = build/src/cpp
EXECUTABLE = ../../bin/octsurf

# defines for the program

SOURCES =	$(SOURCEDIR)util/tictoc.cpp \
		$(SOURCEDIR)util/cmd_args.cpp \
		$(SOURCEDIR)util/progress_bar.cpp \
		$(SOURCEDIR)util/union_find.cpp \
		$(SOURCEDIR)xmlreader/tinystr.cpp \
		$(SOURCEDIR)xmlreader/tinyxml.cpp \
		$(SOURCEDIR)xmlreader/tinyxmlerror.cpp \
		$(SOURCEDIR)xmlreader/tinyxmlparser.cpp \
		$(SOURCEDIR)xmlreader/tinyxmltools.cpp \
		$(SOURCEDIR)xmlreader/xmlsettings.cpp \
		$(SOURCEDIR)io/octree/tree_exporter.cpp \
		$(SOURCEDIR)io/octree/vox_writer.cpp \
		$(SOURCEDIR)geometry/octree/octtopo.cpp \
		$(SOURCEDIR)geometry/octree/octree.cpp \
		$(SOURCEDIR)geometry/octree/octnode.cpp \
		$(SOURCEDIR)geometry/octree/octdata.cpp \
		$(SOURCEDIR)mesh/refine/octree_padder.cpp \
		$(SOURCEDIR)mesh/partition/node_partitioner.cpp \
		$(SOURCEDIR)mesh/partition/node_set.cpp \
		$(SOURCEDIR)mesh/surface/node_boundary.cpp \
		src/octsurf_run_settings.cpp \
		src/main.cpp

HEADERS =	$(SOURCEDIR)util/error_codes.h \
		$(SOURCEDIR)util/tictoc.h \
		$(SOURCEDIR)util/cmd_args.h \
		$(SOURCEDIR)util/endian.h \
		$(SOURCEDIR)util/progress_bar.h \
		$(SOURCEDIR)util/union_find.h \
		$(SOURCEDIR)xmlreader/tinystr.h \
		$(SOURCEDIR)xmlreader/tinyxml.h \
		$(SOURCEDIR)xmlreader/tinyxmltools.h \
		$(SOURCEDIR)xmlreader/xmlsettings.h \
		$(SOURCEDIR)io/octree/tree_exporter.h \
		$(SOURCEDIR)io/octree/vox_writer.h \
		$(SOURCEDIR)geometry/octree/octtopo.h \
		$(SOURCEDIR)geometry/octree/octree.h \
		$(SOURCEDIR)geometry/octree/octnode.h \
		$(SOURCEDIR)geometry/octree/shape.h \
		$(SOURCEDIR)geometry/octree/octdata.h \
		$(SOURCEDIR)geometry/poly_intersect/poly2d.h \
		$(SOURCEDIR)mesh/refine/octree_padder.h \
		$(SOURCEDIR)mesh/partition/node_partitioner.h \
		$(SOURCEDIR)mesh/partition/node_set.h \
		$(SOURCEDIR)mesh/surface/node_boundary.h \
		src/octsurf_run_settings.h

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

