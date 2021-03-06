CC = g++
CFLAGS = -g -O2 -W -Wall -Wextra -std=c++0x
LFLAGS = -lm -lopencv_core -lopencv_imgproc -lopencv_highgui
PFLAGS = #-pg
SOURCEDIR = ../../src/cpp/
EIGENDIR = /usr/include/eigen3/
IFLAGS = -I$(SOURCEDIR) -I$(EIGENDIR)
BUILDDIR = build/src/cpp
EXECUTABLE = ../../bin/pointcloud_gen

# defines for the program

SOURCES =	$(SOURCEDIR)util/tictoc.cpp \
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
		$(SOURCEDIR)io/data/urg/urg_data_reader.cpp \
		$(SOURCEDIR)io/data/d_imager/d_imager_data_reader.cpp \
		$(SOURCEDIR)io/data/fss/fss_io.cpp \
		$(SOURCEDIR)io/data/mcd/McdFile.cpp \
		$(SOURCEDIR)io/data/color_image/color_image_metadata_reader.cpp \
		$(SOURCEDIR)io/carve/noisypath_io.cpp \
		$(SOURCEDIR)io/pointcloud/pointcloud_writer.cpp \
		$(SOURCEDIR)io/pointcloud/writer/OBJWriter.cpp \
		$(SOURCEDIR)io/pointcloud/writer/XYZWriter.cpp \
		$(SOURCEDIR)io/pointcloud/writer/PTSWriter.cpp \
		$(SOURCEDIR)io/pointcloud/writer/PCDWriter.cpp \
		$(SOURCEDIR)io/pointcloud/writer/PointCloudWriter.cpp \
		$(SOURCEDIR)timestamp/sync_xml.cpp \
		$(SOURCEDIR)image/image_cache.cpp \
		$(SOURCEDIR)image/camera.cpp \
		$(SOURCEDIR)image/rectilinear/rectilinear_functions.cpp \
		$(SOURCEDIR)image/rectilinear/rectilinear_camera.cpp \
		$(SOURCEDIR)image/fisheye/ocam_functions.cpp \
		$(SOURCEDIR)image/fisheye/fisheye_camera.cpp \
		$(SOURCEDIR)geometry/system_path.cpp \
		$(SOURCEDIR)geometry/transform.cpp \
		main.cpp

HEADERS =	$(SOURCEDIR)util/error_codes.h \
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
		$(SOURCEDIR)io/data/urg/urg_data_reader.h \
		$(SOURCEDIR)io/data/d_imager/d_imager_data_reader.h \
		$(SOURCEDIR)io/data/fss/fss_io.h \
		$(SOURCEDIR)io/data/mcd/McdFile.h \
		$(SOURCEDIR)io/data/color_image/color_image_metadata_reader.h \
		$(SOURCEDIR)io/carve/noisypath_io.h \
		$(SOURCEDIR)io/pointcloud/pointcloud_writer.h \
		$(SOURCEDIR)io/pointcloud/writer/OBJWriter.h \
		$(SOURCEDIR)io/pointcloud/writer/XYZWriter.h \
		$(SOURCEDIR)io/pointcloud/writer/PTSWriter.h \
		$(SOURCEDIR)io/pointcloud/writer/PCDWriter.h \
		$(SOURCEDIR)io/pointcloud/writer/PointCloudWriter.h \
		$(SOURCEDIR)timestamp/sync_xml.h \
		$(SOURCEDIR)image/image_cache.h \
		$(SOURCEDIR)image/image_cache.h \
		$(SOURCEIDR)image/camera.h \
		$(SOURCEDIR)image/rectilinear/rectilinear_functions.h \
		$(SOURCEDIR)image/rectilinear/rectilinear_camera.h \
		$(SOURCEDIR)image/fisheye/ocam_functions.h \
		$(SOURCEDIR)image/fisheye/fisheye_camera.h \
		$(SOURCEDIR)geometry/system_path.h \
		$(SOURCEDIR)geometry/transform.h

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

