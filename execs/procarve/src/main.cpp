#include <iostream>

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This is the main file for the probability-carving program (procarve),
 * which will generate a surface reconstruction of a building interior
 * environment from range scans from a mobile scanning system.
 */

using namespace std;

/* function implementations */

#include <Eigen/Dense>
#include <vector>
#include <geometry/octree/octree.h>
#include <stdlib.h>

using namespace Eigen;

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	vector<octnode_t*> leafs;
	octree_t tree(0.05);
	int ret;
	Vector3d a, b;

	/* carve a tree */
	a << -1,-1,-1;
	b << 1,2,3;
	ret = tree.raycarve(leafs, a, b);
	if(ret)
	{
		cerr << "Unable to carve tree: " << ret << endl;
		return 1;
	}

	/* export tree to file */
	ret = tree.serialize("tree_out_test.oct");
	if(ret)
	{
		cerr << "Unable to export: " << ret << endl;
		return 2;
	}

	/* success */
	return 0;
}
