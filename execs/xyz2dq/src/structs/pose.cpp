#include "pose.h"

#include <vector>
#include "../util/error_codes.h"

using namespace std;

int poselist_closest_index(vector<pose_t>& pl, double t)
{
	unsigned int low, high, mid, last;
	double comp;

	/* check arguments */
	if(pl.empty())
		return -1;

	/* check outer bounds */
	if(t < pl[0].timestamp)
		return 0;
	last = pl.size() - 1;
	if(t > pl[last].timestamp)
		return last;

	/* assume poses are in order, and perform binary search */
	low = 0;
	high = pl.size();
	while(low <= high)
	{
		/* check middle of range */
		mid = (low + high)/2;
		comp = pl[mid].timestamp - t;

		/* subdivide */
		if(comp < 0)
			low = mid + 1;
		else if(comp > 0)
		{
			high = mid - 1;
			
			/* check if high is now less than t */
			if(mid != 0 && pl[high].timestamp < t)
			{
				low = high;
				break;
			}
		}
		else
			return mid; /* exactly the right time */
	}

	/* we know t falls between indices low and low+1 */
	if(t - pl[low].timestamp > pl[low+1].timestamp - t)
		return low+1;
	return low;
}
