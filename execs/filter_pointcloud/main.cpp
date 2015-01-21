/*
	filter_pointcloud

	This executable handles basic operations on point clouds that can be 
	accomplished in a streaming fashion
*/

/* includes */
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>

#include <util/cmd_args.h>
#include <io/conf/conf_reader.h>
#include <io/pointcloud/reader/PointCloudReader.h>

#include "Point.h"
#include "Filter.h"

/* name spaces */
using namespace std;

/* defines */
#define FLAG_INPUT "-i"
#define FLAG_SCRIPT "-x"
#define FLAG_OUTPUT "-o"
#define FLAG_LISTCOMMANDS "--list_commands"

#define CMD_DECIMATE "Decimate"
#define CMD_FLIPVALID "FlipValid"
#define CMD_KILL "Kill"
#define CMD_OUTPUT "Output"
#define CMD_PARTITIONAABB "PartitionAABB"
#define CMD_PARTITIONCYLINDER "PartitionCylinder"
#define CMD_PARTITIONPLANE "PartitionPlane"
#define CMD_PARTITIONRADIUS "PartitionRadius"
#define CMD_PRINTSTATS "PrintStats"
#define CMD_RECOLOR "Recolor"
#define CMD_ROTATE "Rotate"
#define CMD_SCALE "Scale"
#define CMD_TRANSLATE "Translate"

#define CMD_FILTERALL "ALL"
#define CMD_FILTERVALID "VALID"
#define CMD_FILTERINVALID "INVALID"

/* function definitions */
void build_conf_reader(conf::reader_t& conf_reader);
int convert_to_filters(conf::reader_t& conf_reader, 
	vector<shared_ptr<PointCloudFilter> >& filters);

/* the main function */
int main(int argc, char * argv[])
{
	int ret;

	/* create the script parsing commands */
	conf::reader_t conf_reader;
	build_conf_reader(conf_reader);

	/* create the argument parser */
	cmd_args_t parser;
	parser.set_program_description(
		"This program allows for simple operations and scripts on point "
		"cloud files. Please use the " FLAG_LISTCOMMANDS " flag to see "
		"a full list of commands.");
	parser.add(FLAG_INPUT,
		"The list of input files that will be operated on. This can any number "
		"of files.",
		true,
		cmd_args_t::FLEX_ARGS);
	parser.add(FLAG_SCRIPT,
		"Sets the input source for commands. Either a file containing commands "
		"or a single string of commands can be given. If this command is not " 
		"given then commands will be read from standard in.",
		true,
		1);
	parser.add(FLAG_OUTPUT,
		"This flag will force the points at the end of the filtering pipeline "
		"to be written in a file.\n"
		"\n"
		"WARNING:\n"
		"\tSince points are written in a streaming fashion do NOT write to\n"
		"\ta file that is already being written to in the filter chain.",
		true,
		1);
	parser.add(FLAG_LISTCOMMANDS,
		"Dumps the accepted commands to standard out.",
		true,
		0);

	/* parse the inputs */
	ret = parser.parse(argc, argv);
	if(ret)
		return -1;

	/* check if we should dump the command list */
	if(parser.tag_seen(FLAG_LISTCOMMANDS))
	{
		conf_reader.helptext(cout);
		return -2;
	}

	/* check if there are any input files if there are not then we dont */
	/* actually need to do anything */
	vector<string> inputFiles;
	if(!parser.tag_seen(FLAG_INPUT, inputFiles))
		return 0;

	/* parse the given commands */
	if(!parser.tag_seen(FLAG_SCRIPT))
	{	
		ret = conf_reader.parse(std::cin);
	}
	else
	{
		ifstream f(parser.get_val(FLAG_SCRIPT));
		if(f.is_open())
		{
			ret = conf_reader.parse(f);
		}
		else
		{
			stringstream ss(parser.get_val(FLAG_SCRIPT));
			ret = conf_reader.parse(ss);
		}
	}
	if(ret)
		return -3;

	/* convert the conf reader into a list of filters */
	vector<shared_ptr<PointCloudFilter> > filters;
	ret = convert_to_filters(conf_reader, filters);
	if(ret)
		return -4;

	/* if an output file is given then create an output filter and tack it on */
	/* the end */
	if(parser.tag_seen(FLAG_OUTPUT))
	{
		filters.push_back(make_shared<FilterOutputToFile>(
			parser.get_val(FLAG_OUTPUT)));
	}


	/* now we need to run the program logic */
	for(auto pcFile : inputFiles)
	{
		/* create the point cloud reader */
		PointCloudReader reader = PointCloudReader::create(pcFile);
		if(!reader.open(pcFile))
		{
			cerr << "[main] Unable to open pointcloud file : " 
				 << pcFile << endl;
			return -5;
		}

		/* loop over the readers points */
		Point p;
		while(reader.read_point(p.x, p.y, p.z, 
			p.r, p.g, p.b, 
			p.index, p.timestamp))
		{

			/* push the point through the filters */
			for(auto&& filter : filters)
				if(!filter->apply(p))
					break;
		}

	}

	/* return success */
	return 0;
}

/* define the commands for the filtering */
void build_conf_reader(conf::reader_t& conf_reader)
{
	conf_reader.set_line_width(79);
	conf_reader.set_general_description(
		"This describes the total list of commands that are understood by "
		"the filter_pointcloud scripting engine.\n"
		"\n"
		"Each command has a has an optional <OPERATE_ON> argument that "
		"can be inserted between the command and the commands arguments. By "
		"setting this the command can be flagged to only operate on valid or "
		"invalid points. The accepted list of options for this are:\n"
		"\t" CMD_FILTERALL ", " CMD_FILTERVALID ", or " CMD_FILTERINVALID "\n"
		"\n"
		"Points are flagged as invalid if they have been effected by a "
		"partitioning or decimation operation.  They are not actually "
		"removed from the filtering pipeline until a " CMD_KILL " command "
		"is issued.");
	conf_reader.add_keyword(CMD_FLIPVALID,
		"Usage: " CMD_FLIPVALID " [<OPERATE_ON>]\n"
		"\n"
		"This command operates as a logical NOT on the set of currently valid"
		"points in the filter pipeline.",
		conf::VARARGS);
	conf_reader.add_keyword(CMD_ROTATE,
		"Usage: " CMD_ROTATE " [<OPERATE_ON>] <ROLL> <PITCH> <YAW>\n"
		"              [<OPERATE_ON>] <W> <X> <Y> <Z>\n"
		"              [<OPERATE_ON>] <R00> <R01> <R02> <R10> ... <R22>\n"
		"\n"
		"This command rotates the point cloud using a rigid body rotation. "
		"The rotation can be specified as either:\n" 
		"\t3 element Euler angle set in degrees : roll pitch yaw\n" 
		"\t4 element quaternion in order : w x y z\n"
		"\t9 element rotation matrix in row major order\n"
		"\n"
		"The Euler angles are specified in 3-2-1 axis ordering. This means "
		"that a rotation matrix is specified as R = Rz*Ry*Rx where Rx is a "
		"rotation about the x axis, Ry is a rotation about the y axis, and "
		"Rz is a rotation about the z axis.",
		conf::VARARGS);
	conf_reader.add_keyword(CMD_SCALE,
		"Usage: " CMD_SCALE " [<OPERATE_ON>] <SCALE>\n"
		"             [<OPERATE_ON>] <SCALE_X> <SCALE_Y> <SCALE_Z>\n"
		"\n"
		"This command will scale the points in a point cloud. The scale "
		"factor can be defined either as a single global scale factor or "
		"on a per axis basis.",
		conf::VARARGS);
	conf_reader.add_keyword(CMD_TRANSLATE,
		"Usage: " CMD_TRANSLATE " [<OPERATE_ON>] <OFFSET>\n"
		"                 [<OPERATE_ON>] <OFFSET_X> <OFFSET_Y> <OFFSET_Z>\n"
		"\n"
		"This command will translate a point cloud. The translation can "
		"either be specified as a single offset for all axis or on a per "
		"axis basis",
		conf::VARARGS);
	conf_reader.add_keyword(CMD_RECOLOR,
		"Usage: " CMD_RECOLOR " [<OPERATE_ON>] <GRAY_LEVEL>\n"
		"               [<OPERATE_ON>] <R> <G> <B>\n"
		"\n"
		"This command recolors the points in a point cloud to a set color. "
		"The color can be specified as either a single gray-scale value or "
		"as an RGB triplet. The values are expected in the range [0 255]",
		conf::VARARGS);
	conf_reader.add_keyword("Decimate",
		"Usage: Decimate [<OPERATE_ON>] <DECIMATION_FACTOR>\n"
		"\n"
		"This command decimates a point cloud using a set decimation rate.",
		conf::VARARGS);
	conf_reader.add_keyword(CMD_KILL,
		"Usage: " CMD_KILL " [<OPERATE_ON>]\n"
		"\n"
		"This command will eliminate points from the filtering pipeline at "
		"the point this command is specified.",
		conf::VARARGS);
	conf_reader.add_keyword(CMD_PARTITIONPLANE,
		"Usage: " CMD_PARTITIONPLANE " [<OPERATE_ON>] <NX> <NY> <NZ>\n"
		"                      [<OPERATE_ON>] <NX> <NY> <NZ> <PX> <PY> <PZ>\n"
		"\n"
		"This command will partition the points using a partitioning plane. "
		"The plane is specified by a 6 element set containing first the "
		"normal vector of the plane followed by a point on the plane. "
		"If the point on plane is not specified it will be assumed to be "
		"the origin.",
		conf::VARARGS);
	conf_reader.add_keyword(CMD_PARTITIONRADIUS,
		"Usage: " CMD_PARTITIONRADIUS " [<OPERATE_ON>] <PX> <PY> <PZ> <RADIUS>\n"
		"\n"
		"This command will partition the points using a sphere centered on "
		"a given point. The sphere is specified using a 4 element set "
		"containing first point that is the center of the sphere followed by "
		"the radius of the sphere.",
		conf::VARARGS);
	conf_reader.add_keyword(CMD_PARTITIONCYLINDER,
		"Usage: " CMD_PARTITIONCYLINDER " [<OPERATE_ON>] <PX> <PY> <PZ> ... <RADIUS>\n"
		"\n"
		"This command will partition the points using a cylinder. The cylinder "
		"is specified by a line and a width around that line. The parameters "
		"are passed in a 7 element set. The set first contains a point on "
		"the line that defines the cylinders axis, then a direction for that "
		"line, then the radius of the cylinder.",
		conf::VARARGS);
	conf_reader.add_keyword(CMD_PARTITIONAABB,
		"Usage: " CMD_PARTITIONAABB " [<OPERATE_ON>] <MINX> <MAXX> ... <MAXZ>\n"
		"\n"
		"This command will partition the points using an axis-aligned "
		"bounding box. The bounding box is defined in terms of its "
		"min and max values in each of the three coordinate axis. The "
		"parameters are passed in a six element set first by defining "
		"the x axis limits, then the y axis limits, then the z axis "
		"limits");
	conf_reader.add_keyword(CMD_PRINTSTATS,
		"Usage: " CMD_PRINTSTATS " [<OPERATE_ON>] [<DESCRIPTION>]\n"
		"\n"
		"This command instructs the program to compute the number of points "
		"and bounding box of the point cloud at the stage of the filtering "
		"where this command is defined. The stats are displayed at the "
		"end of the program execution. An optional description may be given.",
		conf::VARARGS);
	conf_reader.add_keyword(CMD_OUTPUT,
		"Usage: " CMD_OUTPUT " [<OPERATE_ON>] <OUTPUT_FILENAME>\n"
		"\n"
		"This command will write the points to the specified output file. "
		"If the output file type is different than the input file type "
		"then file format conversion will occur.");
}

int convert_to_filters(conf::reader_t& conf_reader,
	vector<shared_ptr<PointCloudFilter> >& filters)
{

	/* force the mark valid filter on the front of the list */
	filters.clear();
	filters.push_back(
		make_shared<FilterFlipValidity>(FilterFlipValidity::FILTER_INVALID));

	/* figure out which filter this is */
	for(size_t i = 0; i < conf_reader.size(); i++)
	{
		/* make a nice copy of the command for ease */
		const string keyword = conf_reader.get(i).get_keyword();
		vector<string> args = conf_reader.get(i).get_args();

		/* get the operational flag */
		PointCloudFilter::FILTER_OPERATES_ON operatesOn;
		bool hasOFlag;
		if(args.size() > 0)
			hasOFlag = PointCloudFilter::get_operates_on(args[0], operatesOn);
		else
			hasOFlag = false;

		/* rend the first element if needed */
		if(hasOFlag)
			args.erase(args.begin());



		/*
		* compare against known keywords
		*/

		/*
		* 	Handle the FlipValidity Filter
		*/
		if(keyword.compare(CMD_FLIPVALID) == 0)
		{
			if(args.size() > 0)
			{
				cerr << "Error at Command #" << i << " : " << keyword 
					 << " has too many arguments" << endl;
				return -1;
			}
			filters.push_back(make_shared<FilterFlipValidity>(operatesOn));
		}

		/*
		*	Handle the Rotate filter
		*/
		else if(keyword.compare(CMD_ROTATE) == 0)
		{
			if(args.size() != 3 && args.size() != 4 && args.size() != 9)
			{
				cerr << "Error at Command #" << i << " : " << keyword 
					 << " has incorrect number of arguments." << endl;
				return -2;
			}

			/* doublize the values */
			vector<double> vals(args.size());
			for(size_t j = 0; j < args.size(); j++)
			{
				stringstream ss(args[j]);
				if(!(ss >> vals[j]))
				{
					cerr << "Error at Command #" << i << " : Parameter #"
						 << j << " : Can not convert " << args[j] << " to "
						 << "numeric." << endl;
					return -3;
				}
			}

			if(vals.size() == 3)
			{
				filters.push_back(make_shared<FilterRotate>(
					vals[0]*M_PI/180.0,
					vals[1]*M_PI/180.0,
					vals[2]*M_PI/180.0,
					operatesOn));
			}
			else if(vals.size() == 4)
			{
				filters.push_back(make_shared<FilterRotate>(
					vals[0], vals[1], vals[2], vals[3], operatesOn));
			}
			else if(vals.size() == 9)
			{
				filters.push_back(make_shared<FilterRotate>(
					vals[0], vals[1], vals[2],
					vals[3], vals[4], vals[5],
					vals[6], vals[7], vals[8],
					operatesOn));	
			}
		}

		/*
		*	Handle the Scale filter
		*/
		else if(keyword.compare(CMD_SCALE) == 0)
		{
			if(args.size() != 3 && args.size() != 1)
			{
				cerr << "Error at Command #" << i << " : " << keyword 
					 << " has incorrect number of arguments." << endl;
				return -2;
			}

			/* doublize the values */
			vector<double> vals(args.size());
			for(size_t j = 0; j < args.size(); j++)
			{
				stringstream ss(args[j]);
				if(!(ss >> vals[j]))
				{
					cerr << "Error at Command #" << i << " : Parameter #"
						 << j << " : Can not convert " << args[j] << " to "
						 << "numeric." << endl;
					return -3;
				}
			}

			/* create the correct filter */
			if(vals.size() == 1)
			{
				filters.push_back(make_shared<FilterScale>(vals[0], 
					operatesOn));
			}
			else if(vals.size() == 3)
			{
				filters.push_back(make_shared<FilterScale>(
					vals[0], vals[1], vals[2], operatesOn));
			}
		}

		/*
		*	Handle the Translate filter
		*/
		else if(keyword.compare(CMD_TRANSLATE) == 0)
		{
			if(args.size() != 3 && args.size() != 1)
			{
				cerr << "Error at Command #" << i << " : " << keyword 
					 << " has incorrect number of arguments." << endl;
				return -2;
			}

			/* doublize the values */
			vector<double> vals(args.size());
			for(size_t j = 0; j < args.size(); j++)
			{
				stringstream ss(args[j]);
				if(!(ss >> vals[j]))
				{
					cerr << "Error at Command #" << i << " : Parameter #"
						 << j << " : Can not convert " << args[j] << " to "
						 << "numeric." << endl;
					return -3;
				}
			}

			/* create the correct filter */
			if(vals.size() == 1)
			{
				filters.push_back(make_shared<FilterTranslate>(vals[0],
					operatesOn));
			}
			else if(vals.size() == 3)
			{
				filters.push_back(make_shared<FilterTranslate>(
					vals[0], vals[1], vals[2], operatesOn));
			}
		}

		/*
		*	Handle the Recolor filter
		*/
		else if(keyword.compare(CMD_RECOLOR) == 0)
		{
			if(args.size() != 3 && args.size() != 1)
			{
				cerr << "Error at Command #" << i << " : " << keyword 
					 << " has incorrect number of arguments." << endl;
				return -2;
			}

			/* doublize the values */
			vector<unsigned char> vals(args.size());
			for(size_t j = 0; j < args.size(); j++)
			{
				unsigned int v;
				stringstream ss(args[j]);
				if(!(ss >> v))
				{
					cerr << "Error at Command #" << i << " : Parameter #"
						 << j << " : Can not convert " << args[j] << " to "
						 << "numeric." << endl;
					return -3;
				} 
				vals[j] = v;
			}

			/* create the correct filter */
			if(vals.size() == 1)
			{
				filters.push_back(make_shared<FilterRecolor>(vals[0],
					operatesOn));
			}
			else if(vals.size() == 3)
			{
				filters.push_back(make_shared<FilterRecolor>(
					vals[0], vals[1], vals[2], operatesOn));
			}
		}

		/*
		*	Handle the Decimation filter
		*/
		else if(keyword.compare(CMD_DECIMATE) == 0)
		{
			if(args.size() != 1)
			{
				cerr << "Error at Command #" << i << " : " << keyword 
					 << " has incorrect number of arguments." << endl;
				return -2;
			}

			/* doublize the values */
			vector<size_t> vals(args.size());
			for(size_t j = 0; j < args.size(); j++)
			{
				stringstream ss(args[j]);
				if(!(ss >> vals[j]))
				{
					cerr << "Error at Command #" << i << " : Parameter #"
						 << j << " : Can not convert " << args[j] << " to "
						 << "numeric." << endl;
					return -3;
				}
			}

			/* create the correct filter */
			filters.push_back(make_shared<FilterDecimate>(vals[0],
				operatesOn));
		}

		/*
		*	Handle the kill filter
		*/
		else if(keyword.compare(CMD_KILL) == 0)
		{
			if(args.size() > 0)
			{
				cerr << "Error at Command #" << i << " : " << keyword 
					 << " has too many arguments" << endl;
				return -1;
			}
			filters.push_back(make_shared<FilterKill>(operatesOn));
		}

		/*
		*	Handle partition by plane
		*/
		else if(keyword.compare(CMD_PARTITIONPLANE) == 0)
		{
			if(args.size() != 3 && args.size() != 6)
			{
				cerr << "Error at Command #" << i << " : " << keyword 
					 << " has incorrect number of arguments." << endl;
				return -2;
			}

			/* doublize the values */
			vector<double> vals(args.size());
			for(size_t j = 0; j < args.size(); j++)
			{
				stringstream ss(args[j]);
				if(!(ss >> vals[j]))
				{
					cerr << "Error at Command #" << i << " : Parameter #"
						 << j << " : Can not convert " << args[j] << " to "
						 << "numeric." << endl;
					return -3;
				}
			}

			/* create the correct filter */
			if(vals.size() == 3)
			{
				filters.push_back(make_shared<FilterPartitionPlane>(vals[0],
					vals[1], vals[2],
					operatesOn));
			}
			else if(vals.size() == 6)
			{
				filters.push_back(make_shared<FilterPartitionPlane>(
					vals[0], vals[1], vals[2], 
					vals[3], vals[4], vals[5], operatesOn));
			}
		}

		/*
		*	Handle partition by radius
		*/
		else if(keyword.compare(CMD_PARTITIONRADIUS) == 0)
		{
			if(args.size() != 4)
			{
				cerr << "Error at Command #" << i << " : " << keyword 
					 << " has incorrect number of arguments." << endl;
				return -2;
			}

			/* doublize the values */
			vector<double> vals(args.size());
			for(size_t j = 0; j < args.size(); j++)
			{
				stringstream ss(args[j]);
				if(!(ss >> vals[j]))
				{
					cerr << "Error at Command #" << i << " : Parameter #"
						 << j << " : Can not convert " << args[j] << " to "
						 << "numeric." << endl;
					return -3;
				}
			}

			/* create the correct filter */
			filters.push_back(make_shared<FilterPartitionRadius>(vals[0],
				vals[1], vals[2], vals[3],
				operatesOn));
		}

		/*
		*	Handle the parition by cylinder
		*/
		else if(keyword.compare(CMD_PARTITIONCYLINDER) == 0)
		{
			if(args.size() != 7)
			{
				cerr << "Error at Command #" << i << " : " << keyword 
					 << " has incorrect number of arguments." << endl;
				return -2;
			}

			/* doublize the values */
			vector<double> vals(args.size());
			for(size_t j = 0; j < args.size(); j++)
			{
				stringstream ss(args[j]);
				if(!(ss >> vals[j]))
				{
					cerr << "Error at Command #" << i << " : Parameter #"
						 << j << " : Can not convert " << args[j] << " to "
						 << "numeric." << endl;
					return -3;
				}
			}

			/* create the correct filter */
			filters.push_back(make_shared<FilterPartitionCylinder>(vals[0],
				vals[1], vals[2], vals[3], vals[4],
				vals[5], vals[6],
				operatesOn));
		}

		/*
		*	Handle the parition by aabb
		*/
		else if(keyword.compare(CMD_PARTITIONAABB) == 0)
		{
			if(args.size() != 6)
			{
				cerr << "Error at Command #" << i << " : " << keyword 
					 << " has incorrect number of arguments." << endl;
				return -2;
			}

			/* doublize the values */
			vector<double> vals(args.size());
			for(size_t j = 0; j < args.size(); j++)
			{
				stringstream ss(args[j]);
				if(!(ss >> vals[j]))
				{
					cerr << "Error at Command #" << i << " : Parameter #"
						 << j << " : Can not convert " << args[j] << " to "
						 << "numeric." << endl;
					return -3;
				}
			}

			/* create the correct filter */
			filters.push_back(make_shared<FilterPartitionAABB>(vals[0],
				vals[1], vals[2], vals[3], vals[4],
				vals[5], 
				operatesOn));
		}

		/*
		*	Handle the printing stats filter
		*/
		else if(keyword.compare(CMD_PRINTSTATS) == 0)
		{
			stringstream ss;
			for(size_t j = 0; j < args.size(); j++)
				ss << args[j] << " ";
			filters.push_back(make_shared<FilterPrintStats>(cout, ss.str(),
				operatesOn));
		}

		/*
		*	Handle output filters
		*/
		else if(keyword.compare(CMD_OUTPUT) == 0)
		{
			if(args.size() != 1)
			{
				cerr << "Error at Command #" << i << " : " << keyword 
					 << " has incorrect number of arguments." << endl;
				return -1;
			}
			filters.push_back(make_shared<FilterOutputToFile>(args[0], 
				operatesOn));
		}

		/*
		*	This should never happen
		*/
		else
		{
			throw std::runtime_error("A register command is not listed "
				"in the create filter function. This should never happen."
				"Command : " + keyword);
		}

	}


	/* return success */
	return 0;
}