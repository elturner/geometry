CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -std=c++11
LFLAGS = -lm
PFLAGS = #-pg -fprofile-arcs
SOURCEDIR = ../../src/cpp/
IFLAGS = -I$(SOURCEDIR)
BUILDDIR = build/src/cpp
EXECUTABLE = ../../bin/filter_pointcloud

# defines for the program

SOURCES =	$(SOURCEDIR)util/tictoc.cpp \
		$(SOURCEDIR)util/cmd_args.cpp \
		$(SOURCEDIR)io/conf/conf_reader.cpp \
		$(SOURCEDIR)io/pointcloud/writer/PointCloudWriter.cpp \
		$(SOURCEDIR)io/pointcloud/writer/OBJWriter.cpp \
		$(SOURCEDIR)io/pointcloud/writer/PTSWriter.cpp \
		$(SOURCEDIR)io/pointcloud/writer/PCDWriter.cpp \
		$(SOURCEDIR)io/pointcloud/writer/XYZWriter.cpp \
		$(SOURCEDIR)io/pointcloud/reader/PointCloudReader.cpp \
		$(SOURCEDIR)io/pointcloud/reader/OBJReader.cpp \
		$(SOURCEDIR)io/pointcloud/reader/PTSReader.cpp \
		$(SOURCEDIR)io/pointcloud/reader/XYZReader.cpp \
		Filter.cpp \
		main.cpp

HEADERS =	$(SOURCEDIR)util/error_codes.h \
		$(SOURCEDIR)util/tictoc.h \
		$(SOURCEDIR)util/cmd_args.h \
		$(SOURCEDIR)io/conf/conf_reader.h \
		$(SOURCEDIR)io/pointcloud/writer/PointCloudWriter.h \
		$(SOURCEDIR)io/pointcloud/writer/OBJWriter.h \
		$(SOURCEDIR)io/pointcloud/writer/PTSWriter.h \
		$(SOURCEDIR)io/pointcloud/writer/PCDWriter.h \
		$(SOURCEDIR)io/pointcloud/writer/XYZWriter.h \
		$(SOURCEDIR)io/pointcloud/reader/PointCloudReader.h \
		$(SOURCEDIR)io/pointcloud/reader/OBJReader.h \
		$(SOURCEDIR)io/pointcloud/reader/PTSReader.h \
		$(SOURCEDIR)io/pointcloud/reader/XYZReader.h \
		Filter.h \
		Point.h

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
	@g++ -std=c++11 -MM -MF $(patsubst %.o,%.d,$@) -MT $@ $< # recalc depends
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

