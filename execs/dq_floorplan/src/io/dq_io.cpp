#include "dq_io.h"
#include <fstream>
#include "../util/error_codes.h"

using namespace std;

int read_dq(char* filename, quadtree_t& tree)
{
	ifstream infile;
	int ret;

	/* open file for reading */
	infile.open(filename);
	if(!(infile.is_open()))
		return -1;

	/* read the contents of the file */
	ret = tree.parse(infile);
	if(ret)
	{
		infile.close();
		return PROPEGATE_ERROR(-2, ret);
	}

	/* clean up */
	infile.close();
	return 0;
}

int write_dq(char* filename, quadtree_t& tree)
{
	ofstream outfile;

	/* open file for writing */
	outfile.open(filename);
	if(!(outfile.is_open()))
		return -1;
	
	/* write contents of tree */
	tree.print(outfile);

	/* clean up */
	outfile.close();
	return 0;
}
