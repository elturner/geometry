CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -std=c++0x
PFLAGS = #-pg
SOURCEDIR = ../../src/cpp/
EIGENDIR = /usr/include/eigen3/
OPENNIDIR = /usr/include/openni2/
IFLAGS = -I$(SOURCEDIR) -I$(SOURCEDIR)include -I$(EIGENDIR) -I$(OPENNIDIR)
LFLAGS = -lm -lOpenNI -lOpenNI2 
BUILDDIR = build/src/cpp
EXECUTABLE = ../../bin/oni2xyz

# defines for the program

SOURCES =	$(SOURCEDIR)util/tictoc.cpp \
		$(SOURCEDIR)util/cmd_args.cpp \
		$(SOURCEDIR)util/progress_bar.cpp \
		$(SOURCEDIR)util/rotLib.cpp \
		$(SOURCEDIR)config/backpackConfig.cpp \
		$(SOURCEDIR)config/cameraProp.cpp \
		$(SOURCEDIR)config/imuProp.cpp \
		$(SOURCEDIR)config/laserProp.cpp \
		$(SOURCEDIR)config/tofProp.cpp \
		$(SOURCEDIR)xmlreader/tinystr.cpp \
		$(SOURCEDIR)xmlreader/tinyxml.cpp \
		$(SOURCEDIR)xmlreader/tinyxmlerror.cpp \
		$(SOURCEDIR)xmlreader/tinyxmlparser.cpp \
		$(SOURCEDIR)xmlreader/tinyxmltools.cpp \
		$(SOURCEDIR)xmlreader/xmlsettings.cpp \
		$(SOURCEDIR)timestamp/sync_xml.cpp \
		src/log_reader.cpp \
		src/main.cpp

HEADERS =	$(SOURCEDIR)util/error_codes.h \
		$(SOURCEDIR)util/tictoc.h \
		$(SOURCEDIR)util/cmd_args.h \
		$(SOURCEDIR)util/progress_bar.h \
		$(SOURCEDIR)util/rotLib.h \
		$(SOURCEDIR)config/backpackConfig.h \
		$(SOURCEDIR)config/cameraProp.h \
		$(SOURCEDIR)config/imuProp.h \
		$(SOURCEDIR)config/laserProp.h \
		$(SOURCEDIR)config/sensorProp.h \
		$(SOURCEDIR)config/tofProp.h \
		$(SOURCEDIR)xmlreader/tinystr.h \
		$(SOURCEDIR)xmlreader/tinyxml.h \
		$(SOURCEDIR)xmlreader/tinyxmltools.h \
		$(SOURCEDIR)xmlreader/xmlsettings.h \
		$(SOURCEDIR)timestamp/sync_xml.h \
		src/log_reader.h

OBJECTS = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SOURCES))

# compile commands

all: $(SOURCES) $(EXECUTABLE)
	make --no-builtin-rules --no-builtin-variables $(EXECUTABLE)

simple:
	$(CC) $(IFLAGS) $(CFLAGS) $(LFLAGS) $(PFLAGS) $(SOURCES) -o $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LFLAGS) $(PFLAGS) $(IFLAGS) $(CFLAGS)

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

