CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -ansi -pedantic
LFLAGS = -lm 
PFLAGS = #-pg
EIGENDIR = /usr/include/eigen3/
IFLAGS = -I$(EIGENDIR)
BUILDDIR = build/src/cpp
EXECUTABLE = ../../bin/surface_carve

# defines for the program

SOURCES =	src/util/tictoc.cpp \
		src/util/progress_bar.cpp \
		src/math/mathlib.cpp \
		src/math/eigenwrapper.cpp \
		src/io/filetypes.cpp \
		src/io/config.cpp \
		src/io/scanner_config_io.cpp \
		src/io/point_io.cpp \
		src/io/pose_io.cpp \
		src/io/voxel_io.cpp \
		src/io/mesh_io.cpp \
		src/io/region_io.cpp \
		src/structs/point.cpp \
		src/structs/pose.cpp \
		src/structs/dgrid.cpp \
		src/structs/mesher.cpp \
		src/structs/triangulation.cpp \
		src/structs/normal.cpp \
		src/structs/quadtree.cpp \
		src/carve/carve_dgrid.cpp \
		src/triangulate/smoothing.cpp \
		src/triangulate/simplify.cpp \
		src/triangulate/union_find.cpp \
		src/triangulate/union_find_faces.cpp \
		src/triangulate/region_growing.cpp \
		src/process/makegrid.cpp \
		src/process/create_mesh.cpp \
		src/process/postprocessing.cpp \
		src/process/export_data.cpp \
		src/test/verify.cpp \
		src/test/stats.cpp \
		src/main.cpp

HEADERS =	src/util/error_codes.h \
		src/util/parameters.h \
		src/util/tictoc.h \
		src/util/progress_bar.h \
		src/math/mathlib.h \
		src/math/eigenwrapper.h \
		src/io/filetypes.h \
		src/io/config.h \
		src/io/scanner_config_io.h \
		src/io/point_io.h \
		src/io/pose_io.h \
		src/io/voxel_io.h \
		src/io/mesh_io.h \
		src/io/region_io.h \
		src/structs/point.h \
		src/structs/pose.h \
		src/structs/dgrid.h \
		src/structs/mesher.h \
		src/structs/triangulation.h \
		src/structs/normal.h \
		src/structs/quadtree.h \
		src/carve/carve_dgrid.h \
		src/triangulate/smoothing.h \
		src/triangulate/simplify.h \
		src/triangulate/union_find.h \
		src/triangulate/union_find_faces.h \
		src/triangulate/region_growing.h \
		src/process/makegrid.h \
		src/process/create_mesh.h \
		src/process/postprocessing.h \
		src/process/export_data.h \
		src/marching_cubes/LookUpTable.h \
		src/test/verify.h \
		src/test/stats.h

OBJECTS = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SOURCES))

# compile commands

all: $(SOURCES) $(EXECUTABLE)
	make --no-builtin-rules --no-builtin-variables $(EXECUTABLE)

simple:
	$(CC) $(IFLAGS) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

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

