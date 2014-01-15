#include "test_trirep.h"
#include "../rooms/tri_rep.h"
#include "../structs/triple.h"
#include "../util/error_codes.h"
#include <vector>
#include <set>
#include <iostream>

using namespace std;

/* the individual tests */
int test1();
int test2();
int test3();

/* the testing suite */
int test_trirep()
{
	int ret;

	/* run tests */
	ret = test1();
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	ret = test2();
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	ret = test3();
	if(ret)
		return PROPEGATE_ERROR(-3, ret);

	/* success */
	return 0;
}

int test1()
{
	set<triple_t> input;
	vector<vector<int> > output;
	int ret, n;

	/* create basic triangulation */
	input.insert(triple_t(1, 2, 3));
	ret = tri_rep_t::compute_boundary_edges(output, input);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* check output */
	if(output.size() != 1)
		return PROPEGATE_ERROR(-2, -1 * output.size());
	n = output[0].size();
	if(n != 3)
		return PROPEGATE_ERROR(-3, -n);

	/* verify output numbers */
	if(output[0][0] != 1 || output[0][1] != 2 || output[0][2] != 3)
		return -4;

	/* success */
	return 0;
}

int test2()
{
	set<triple_t> input;
	vector<vector<int> > output;
	int ret, n;

	/* create basic triangulation */
	input.insert(triple_t(1, 2, 3));
	input.insert(triple_t(1, 3, 4));
	input.insert(triple_t(1, 4, 5));
	input.insert(triple_t(1, 5, 6));
	input.insert(triple_t(1, 6, 2));
	ret = tri_rep_t::compute_boundary_edges(output, input);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* check output */
	if(output.size() != 1)
		return PROPEGATE_ERROR(-2, -1 * output.size());
	n = output[0].size();
	if(n != 5)
		return PROPEGATE_ERROR(-3, -n);

	/* verify output numbers */
	if(output[0][0] != 2 
			|| output[0][1] != 3 || output[0][2] != 4 
			|| output[0][3] != 5 || output[0][4] != 6)
		return -4;

	/* success */
	return 0;
}

int test3()
{
	set<triple_t> input;
	vector<vector<int> > output;
	int ret, i, n;

	/* create basic triangulation */
	input.insert(triple_t(1, 2, 3));
	input.insert(triple_t(1, 3, 4));
	input.insert(triple_t(1, 4, 5));
	input.insert(triple_t(1, 5, 6));
	input.insert(triple_t(1, 6, 2));
	input.insert(triple_t(2, 7, 3));
	input.insert(triple_t(4, 7, 5));
	ret = tri_rep_t::compute_boundary_edges(output, input);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* check output */
	if(output.size() != 1)
		return PROPEGATE_ERROR(-2, -1 * output.size());
	n = output[0].size();
	if(n != 7)
		return PROPEGATE_ERROR(-3, -n);

	/* verify output numbers */
	for(i = 0; i < n; i++)
		cout << " " << output[0][i];
	cout << endl;

	/* success */
	return 0;
}
