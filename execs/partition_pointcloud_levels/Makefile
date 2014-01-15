CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -ansi -pedantic
LFLAGS = -lm #-lpthread
PFLAGS = #-pg
BUILDDIR = build
EXECUTABLE = ../../bin/partition_pointcloud_levels

# defines for the program

SOURCES =	src/util/tictoc.cpp \
		src/util/progress_bar.cpp \
		src/io/filetypes.cpp \
		src/io/config.cpp \
		src/math/geometry.cpp \
		src/math/transform.cpp \
		src/structs/path.cpp \
		src/structs/point.cpp \
		src/structs/color.cpp \
		src/structs/pointcloud.cpp \
		src/structs/scanner.cpp \
		src/structs/histogram.cpp \
		src/process/populate_histogram.cpp \
		src/process/compute_level_ranges.cpp \
		src/process/export_data.cpp \
		src/main.cpp

HEADERS =	src/util/error_codes.h \
		src/util/tictoc.h \
		src/util/progress_bar.h \
		src/util/parameters.h \
		src/math/geometry.h \
		src/math/transform.h \
		src/structs/path.h \
		src/structs/point.h \
		src/structs/color.h \
		src/structs/pointcloud.h \
		src/structs/scanner.h \
		src/structs/histogram.h \
		src/process/populate_histogram.h \
		src/process/compute_level_ranges.h \
		src/process/export_data.h

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
	grep -n --color=auto "coryf2.edge" $(SOURCES) $(HEADERS)

size:
	wc $(SOURCES) $(HEADERS)

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) $(BUILDDIR) $(EXECUTABLE).dSYM

# include full recalculated dependencies
-include $(OBJECTS:.o=.d)

