#include "transform.h"
#include "../structs/point.h"
#include "../util/parameters.h"

void affine_transform(point_t& y, double* R, point_t& x, double* T)
{
	double p[NUM_DIMS];

	/* apply 3x3 rotation matrix */
	p[0] = R[0]*x.get(0) + R[1]*x.get(1) + R[2]*x.get(2);
	p[1] = R[3]*x.get(0) + R[4]*x.get(1) + R[5]*x.get(2);
	p[2] = R[6]*x.get(0) + R[7]*x.get(1) + R[8]*x.get(2);

	/* apply translation */
	p[0] += T[0];
	p[1] += T[1];
	p[2] += T[2];

	/* store in y */
	y.set(p);
}
