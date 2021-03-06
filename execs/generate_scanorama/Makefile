CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -std=c++0x
LFLAGS = -lm -lopencv_core -lopencv_imgproc -lopencv_highgui -lboost_system -lboost_thread -lxerces-c
PFLAGS = #-pg
SOURCEDIR = ../../src/cpp/
EIGENDIR = /usr/include/eigen3/
INCLUDEDIR = $(SOURCEDIR)include/
IFLAGS = -I$(SOURCEDIR) -I$(EIGENDIR) -isystem $(INCLUDEDIR)
BUILDDIR = build/src/cpp
EXECUTABLE = ../../bin/generate_scanorama

# defines for the program

SOURCES =	$(INCLUDEDIR)lodepng/lodepng.cpp \
		$(INCLUDEDIR)libe57/E57Foundation.cpp \
		$(INCLUDEDIR)libe57/E57FoundationImpl.cpp \
		$(INCLUDEDIR)libe57/E57Simple.cpp \
		$(INCLUDEDIR)libe57/E57SimpleImpl.cpp \
		$(INCLUDEDIR)libe57/time_conversion.cpp \
		$(SOURCEDIR)util/tictoc.cpp \
		$(SOURCEDIR)util/cmd_args.cpp \
		$(SOURCEDIR)util/rotLib.cpp \
		$(SOURCEDIR)util/progress_bar.cpp \
		$(SOURCEDIR)util/range_list.cpp \
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
		$(SOURCEDIR)io/scanorama/scanolist_io.cpp \
		$(SOURCEDIR)io/mesh/mesh_io.cpp \
		$(SOURCEDIR)io/mesh/mesh_io_obj.cpp \
		$(SOURCEDIR)io/mesh/mesh_io_ply.cpp \
		$(SOURCEDIR)io/data/mcd/McdFile.cpp \
		$(SOURCEDIR)io/data/color_image/color_image_metadata_reader.cpp \
		$(SOURCEDIR)io/carve/noisypath_io.cpp \
		$(SOURCEDIR)image/image_cache.cpp \
		$(SOURCEDIR)image/camera.cpp \
		$(SOURCEDIR)image/rectilinear/rectilinear_functions.cpp \
		$(SOURCEDIR)image/rectilinear/rectilinear_camera.cpp \
		$(SOURCEDIR)image/fisheye/ocam_functions.cpp \
		$(SOURCEDIR)image/fisheye/fisheye_camera.cpp \
		$(SOURCEDIR)image/scanorama/scanorama_maker.cpp \
		$(SOURCEDIR)image/scanorama/scanorama.cpp \
		$(SOURCEDIR)geometry/system_path.cpp \
		$(SOURCEDIR)geometry/transform.cpp \
		src/generate_scanorama_run_settings.cpp \
		src/main.cpp

HEADERS =	$(INCLUDEDIR)lodepng/lodepng.h \
		$(INCLUDEDIR)libe57/E57Foundation.h \
		$(INCLUDEDIR)libe57/E57FoundationImpl.h \
		$(INCLUDEDIR)libe57/E57Simple.h \
		$(INCLUDEDIR)libe57/E57SimpleImpl.h \
		$(INCLUDEDIR)libe57/time_conversion.h \
		$(SOURCEDIR)util/error_codes.h \
		$(SOURCEDIR)util/tictoc.h \
		$(SOURCEDIR)util/cmd_args.h \
		$(SOURCEDIR)util/endian.h \
		$(SOURCEDIR)util/rotLib.h \
		$(SOURCEDIR)util/progress_bar.h \
		$(SOURCEDIR)util/range_list.h \
		$(SOURCEDIR)util/binary_search.h \
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
		$(SOURCEDIR)io/scanorama/scanolist_io.h \
		$(SOURCEDIR)io/mesh/mesh_io.h \
		$(SOURCEDIR)io/data/mcd/McdFile.h \
		$(SOURCEDIR)io/data/color_image/color_image_metadata_reader.h \
		$(SOURCEDIR)io/carve/noisypath_io.h \
		$(SOURCEDIR)timestamp/sync_xml.h \
		$(SOURCEDIR)image/image_cache.h \
		$(SOURCEDIR)image/image_cache.h \
		$(SOURCEDIR)image/camera.h \
		$(SOURCEDIR)image/rectilinear/rectilinear_functions.h \
		$(SOURCEDIR)image/rectilinear/rectilinear_camera.h \
		$(SOURCEDIR)image/fisheye/ocam_functions.h \
		$(SOURCEDIR)image/fisheye/fisheye_camera.h \
		$(SOURCEDIR)image/scanorama/scanorama_maker.h \
		$(SOURCEDIR)image/scanorama/scanorama.h \
		$(SOURCEDIR)image/scanorama/scanorama_point.h \
		$(SOURCEDIR)geometry/system_path.h \
		$(SOURCEDIR)geometry/transform.h \
		$(SOURCEDIR)geometry/raytrace/OctTree.h \
		$(SOURCEDIR)geometry/raytrace/OctTreeHelper.h \
		$(SOURCEDIR)geometry/raytrace/ray.h \
		$(SOURCEDIR)geometry/raytrace/Triangle3.h \
		$(SOURCEDIR)geometry/raytrace/tribox3.h \
		$(SOURCEDIR)geometry/raytrace/triray3.h \
		$(SOURCEDIR)geometry/raytrace/vector3.h \
		$(SOURCEDIR)geometry/raytrace/Vertex3.h \
		src/generate_scanorama_run_settings.h

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
	grep -n --color=auto "p, q" $(SOURCES) $(HEADERS)

size:
	wc $(SOURCES) $(HEADERS)

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) $(BUILDDIR) $(EXECUTABLE).dSYM

# include full recalculated dependencies
-include $(OBJECTS:.o=.d)

