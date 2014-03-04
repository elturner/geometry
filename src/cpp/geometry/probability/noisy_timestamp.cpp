#include "noisy_timestamp.h"
#include <util/randLib.h>
#include <vector>

using namespace std;

/* function implementations */

noisy_timestamp_t::noisy_timestamp_t()
{
	/* default values for this timestamp */
	this->timestamp_mean = 0; /* units: seconds */
	this->stddev = 0.001; /* units: seconds */
}

noisy_timestamp_t::noisy_timestamp_t(double ts, double n)
{
	this->timestamp_mean = ts;
	this->stddev = n;
}

double noisy_timestamp_t::generate_sample() const
{
	vector<double> samples;

	/* get a random sample */
	randLib::randn(samples, 1);

	/* convert to given mean and std. dev. */
	return ((this->stddev * samples[0]) + this->timestamp_mean);
}
