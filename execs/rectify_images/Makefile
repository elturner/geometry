CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -std=c++0x
LFLAGS = -lm -lboost_thread -pthread -lboost_system -lboost_filesystem -lopencv_core -lopencv_imgproc -lopencv_highgui
PFLAGS = #-pg
SOURCEDIR = ../../src/cpp/
EIGENDIR = /usr/include/eigen3/
IFLAGS = -I$(SOURCEDIR) -I$(SOURCEDIR)/include -I$(EIGENDIR)
BUILDDIR = build/src/cpp
EXECUTABLE = ../../bin/rectify_images

# defines for the program

SOURCES =	$(SOURCEDIR)util/tictoc.cpp \
		$(SOURCEDIR)util/cmd_args.cpp \
		$(SOURCEDIR)util/progress_bar.cpp \
		$(SOURCEDIR)io/images/cam_pose_file.cpp \
		$(SOURCEDIR)io/data/color_image/color_image_metadata_reader.cpp \
		$(SOURCEDIR)image/fisheye/ocam_functions.cpp \
		src/rectify_images.cpp \
		src/main.cpp

HEADERS =	$(SOURCEDIR)util/tictoc.h \
		$(SOURCEDIR)util/cmd_args.h \
		$(SOURCEDIR)util/progress_bar.h \
		$(SOURCEDIR)io/images/cam_pose_file.h \
		$(SOURCEDIR)io/data/color_image/color_image_metadata_reader.h \
		$(SOURCEDIR)image/fisheye/ocam_functions.cpp \
		src/rectify_images.h

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

