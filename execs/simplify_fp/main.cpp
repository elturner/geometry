/*
	simplify_fp

	banana banana
*/

/* includes */
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <util/cmd_args.h>

#include "Polygon.h"

/* namespaces */
using namespace std;

/* defines */
#define INPUT_FILE_TAG "-i"
#define SIMPLIFICATION_TAG "-s"
#define OUTPUT_FILE_TAG "-o"

/* function definitions */

/*
	int read_polygons(const std::string& inFile,
		vector<Polygon>& polys)

	Reads the polygons in from the file
*/
int read_polygons(const std::string& inFile,
	vector<Polygon>& polys);

/*
	int write_polys(const std::string& outFile,
		vector<Polygon>& polys)

	Writes the polys to file 
*/
int write_polys(const std::string& outFile,
	vector<Polygon>& polys);

/* main function */
int main(int argc, char * argv[])
{
	int ret;

	/* create an argparser */
	cmd_args_t parser;
	parser.add(INPUT_FILE_TAG, "Specifies the input file.", false, 1);
	parser.add(OUTPUT_FILE_TAG, "Specifies the desired name of the output "
		"file.", false, 1);
	parser.add(SIMPLIFICATION_TAG, "Specifies the simplification error "
		"thresholds in meters.  Two arguments are required.  The first "
		"is the threshold used on the first pass that simplifies the "
		"number of vertices and joins lines.  The second is the threshold "
		"used for simplifying the number of edges on a macro scale in the "
		"polygon.", false, 2);

	/* parse the input arguements */
	ret = parser.parse(argc, argv);
	if(ret)
		return 1;

	/* copy out the arguements */
	const string inFile = parser.get_val(INPUT_FILE_TAG, 0);
	const string outFile = parser.get_val(OUTPUT_FILE_TAG, 0);
	const double simp1 = parser.get_val_as<double>(SIMPLIFICATION_TAG, 0);
	const double simp2 = parser.get_val_as<double>(SIMPLIFICATION_TAG, 1);

	/* read the polys */
	vector<Polygon> polys;
	ret = read_polygons(inFile, polys);
	if(ret)
	{
		cerr << "Unable to read polygon file : " << inFile << endl;
		return 2;
	}

	/* simplyify the polys */
	for(size_t i = 0; i < polys.size(); i++)
	{
		polys[i].simplify(simp1);
		polys[i].simplify(simp2);
	}

	/* write the polys */
	ret = write_polys(outFile, polys);
	if(ret)
	{
		cerr << "Unable to write polygon file : " << outFile << endl;
		return 3;
	}

	/* return success */
	return 0;
}

/*
	int read_polygons(const std::string& inFile,
		vector<Polygon>& polys)

	Reads the polygons in from the file
*/
int read_polygons(const std::string& inFile,
	vector<Polygon>& polys)
{
	polys.clear();

	/* open the file */
	ifstream f(inFile.c_str());
	if(!f.is_open())
		return 1;

	/* read in the polygons */
	while(f.good())
	{
		/* read in a line */
		string line;
		getline(f, line);
		if(line.empty())
			continue;

		/* create a stream from that line */
		stringstream ss(line);

		/* get the number of verts */
		size_t numVerts = 0;
		ss >> numVerts;
		if(numVerts == 0)
			continue;

		/* read the verts */
		vector<Point2D> verts;
		for(size_t i = 0; i < numVerts; i++)
		{
			double vx, vy;
			ss >> vx;
			ss >> vy;
			verts.push_back(Point2D(vx,vy));
		}
		verts.push_back(verts.front());

		/* reverse ordering so they are clockwise */
		reverse(verts.begin(), verts.end());

		/* polygonalize it */
		polys.push_back(Polygon(verts));
	}

	/* return success */
	return 0;
}

/*
	int write_polys(const std::string& outFile,
		vector<Polygon>& polys)

	Writes the polys to file 
*/
int write_polys(const std::string& outFile,
	vector<Polygon>& polys)
{
	/* open output stream */
	ofstream f(outFile.c_str());
	if(!f.is_open())
		return 1;

	/* loop over the polygons reading them back to a vector */
	for(size_t i = 0; i < polys.size(); i++)
	{
		vector<Point2D> verts;
		for(size_t j = 0; j < polys[i].num_verts()-1; j++)
		{
			verts.push_back(Point2D(
				polys[i].vert_x(j),
				polys[i].vert_y(j)));
		}
		reverse(verts.begin(), verts.end());

		f << verts.size() << " ";
		for(size_t j = 0; j < verts.size(); j++)
		{
			f << verts[j].x() << " " << verts[j].y() << " ";
		}
		f << "\n";
	}

	f.close();

	/* return success */
	return 0;
}