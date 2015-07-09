CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -std=c++0x
LFLAGS = -lm
PFLAGS = #-pg -fprofile-arcs
SOURCEDIR = ../../src/cpp/
EIGENDIR = /usr/include/eigen3/
IFLAGS = -I$(SOURCEDIR) -I$(EIGENDIR)
BUILDDIR = build/src/cpp
EXECUTABLE = ../../bin/find_doors

# defines for the program

SOURCES =	$(SOURCEDIR)util/tictoc.cpp \
		$(SOURCEDIR)util/cmd_args.cpp \
		$(SOURCEDIR)util/progress_bar.cpp \
		$(SOURCEDIR)util/rotLib.cpp \
		$(SOURCEDIR)util/range_list.cpp \
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
		$(SOURCEDIR)geometry/octree/octree.cpp \
		$(SOURCEDIR)geometry/octree/octnode.cpp \
		$(SOURCEDIR)geometry/octree/octdata.cpp \
		$(SOURCEDIR)geometry/octree/octtopo.cpp \
		src/door_finder.cpp \
		src/find_doors_settings.cpp \
		src/main.cpp

HEADERS =	$(SOURCEDIR)util/error_codes.h \
		$(SOURCEDIR)util/tictoc.h \
		$(SOURCEDIR)util/cmd_args.h \
		$(SOURCEDIR)util/endian.h \
		$(SOURCEDIR)util/progress_bar.h \
		$(SOURCEDIR)util/rotLib.h \
		$(SOURCEDIR)util/range_list.h \
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
		$(SOURCEDIR)geometry/octree/octree.h \
		$(SOURCEDIR)geometry/octree/octnode.h \
		$(SOURCEDIR)geometry/octree/octdata.h \
		$(SOURCEDIR)geometry/octree/shape.h \
		$(SOURCEDIR)geometry/octree/octtopo.h \
		$(SOURCEDIR)geometry/shapes/shape_wrapper.h \
		$(SOURCEDIR)geometry/shapes/linesegment.h \
		src/door_finder.h \
		src/find_doors_settings.h

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

