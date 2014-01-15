CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -ansi -pedantic
LFLAGS = -lm #-lpthread
PFLAGS = #-pg
BUILDDIR = build
EXECUTABLE = ../../bin/floorplan_gen

# defines for the program

SOURCES =	src/util/tictoc.cpp \
		src/util/progress_bar.cpp \
		src/io/filetypes.cpp \
		src/io/config.cpp \
		src/io/dq_io.cpp \
		src/io/mesh_io.cpp \
		src/io/bim_io.cpp \
		src/io/fp_io.cpp \
		src/structs/quadtree.cpp \
		src/structs/point.cpp \
		src/structs/path.cpp \
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

HEADERS =	src/util/error_codes.h \
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
		src/structs/path.h \
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
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LFLAGS) $(PFLAGS)

$(BUILDDIR)/%.o : %.cpp
	@mkdir -p $(shell dirname $@)		# ensure folder exists
	@g++ -MM -MF $(patsubst %.o,%.d,$@) -MT $@ $< # recalc depends
	$(CC) -c $(CFLAGS) $< -o $@

# helper commands

todo:
	grep -n --color=auto "TODO" $(SOURCES) $(HEADERS)

grep:
	grep -n --color=auto "p, q" $(SOURCES) $(HEADERS)

size:
	wc $(SOURCES) $(HEADERS)

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) $(BUILDDIR) $(EXECUTABLE).dSYM

# include full recalculated dependencies
-include $(OBJECTS:.o=.d)

