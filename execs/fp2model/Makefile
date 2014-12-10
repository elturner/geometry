CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -ansi -pedantic
LFLAGS = -lm #-lpthread
PFLAGS = #-pg
SOURCEDIR = ../../src/cpp/
IFLAGS = -I$(SOURCEDIR)
BUILDDIR = build/src/cpp
EXECUTABLE = ../../bin/fp2model

# defines for the program

SOURCES =	$(SOURCEDIR)util/tictoc.cpp \
		$(SOURCEDIR)util/progress_bar.cpp \
		$(SOURCEDIR)util/cmd_args.cpp \
		$(SOURCEDIR)mesh/floorplan/floorplan.cpp \
		$(SOURCEDIR)mesh/floorplan/floorplan_input.cpp \
		$(SOURCEDIR)mesh/floorplan/floorplan_output.cpp \
		src/io/config.cpp \
		src/io/idf_io.cpp \
		src/io/csv_io.cpp \
		src/io/ply_io.cpp \
		src/io/shp_io.cpp \
		src/structs/building_model.cpp \
		src/structs/window.cpp \
		src/structs/lights.cpp \
		src/structs/people.cpp \
		src/structs/plugloads.cpp \
		src/process/parse_input.cpp \
		src/process/subdivide_room.cpp \
		src/process/export_data.cpp \
		src/main.cpp

HEADERS =	$(SOURCEDIR)util/error_codes.h \
		$(SOURCEDIR)util/tictoc.h \
		$(SOURCEDIR)util/progress_bar.h \
		$(SOURCEDIR)util/cmd_args.h \
		$(SOURCEDIR)util/endian.h \
		$(SOURCEDIR)geometry/poly_intersect/poly2d.h \
		$(SOURCEDIR)mesh/floorplan/floorplan.h \
		src/io/idf_io.h \
		src/io/csv_io.h \
		src/io/ply_io.h \
		src/io/shp_io.h \
		src/structs/building_model.h \
		src/structs/window.h \
		src/structs/lights.h \
		src/structs/people.h \
		src/structs/plugloads.h \
		src/process/parse_input.h \
		src/process/subdivide_room.h \
		src/process/export_data.h

OBJECTS = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SOURCES))

# compile commands

all: $(SOURCES) $(EXECUTABLE)
	make --no-builtin-rules --no-builtin-variables $(EXECUTABLE)

simple:
	$(CC) $(IFLAGS) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LFLAGS) $(PFLAGS) $(IFLAGS) $(CFLAGS)

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

