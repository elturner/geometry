CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -ansi -pedantic
LFLAGS = -lm #-lpthread
PFLAGS = #-pg
BUILDDIR = build

# defines for the program

SOURCES =	src/util/tictoc.cpp \
		src/util/progress_bar.cpp \
		src/io/filetypes.cpp \
		src/io/config.cpp \
		src/io/pose_io.cpp \
		src/structs/pose.cpp \
		src/structs/quadtree.cpp \
		src/structs/point.cpp \
		src/main.cpp

HEADERS =	src/util/error_codes.h \
		src/util/parameters.h \
		src/util/tictoc.h \
		src/util/progress_bar.h \
		src/io/filetypes.h \
		src/io/config.h \
		src/io/pose_io.h \
		src/structs/pose.h \
		src/structs/quadtree.h \
		src/structs/point.h \
		src/structs/normal.h

OBJECTS = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SOURCES))
EXECUTABLE = ../../bin/xyz2dq

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
	grep -n --color=auto "tet_carve" $(SOURCES) $(HEADERS)

size:
	wc $(SOURCES) $(HEADERS)

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) $(BUILDDIR) $(EXECUTABLE).dSYM

# include full recalculated dependencies
-include $(OBJECTS:.o=.d)

