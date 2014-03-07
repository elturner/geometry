#ifndef NOISY_TIMESTAMP_H
#define NOISY_TIMESTAMP_H

/**
 * @file noisy_timestamp.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This defines the noisy_timestamp_t class, which represents the
 * uncertainly in each timestamp sample.  These timestamps are modeled
 * with unbiased gaussian noise.
 */

class noisy_timestamp_t
{
	/* parameters */
	private:

		/* the reported timestamp observed from system clock,
		 * in units of seconds */
		double timestamp_mean;

		/* the uncertainty of this timestamp, in units of seconds */
		double stddev;
	
	/* functions */
	public:

		/**
		 * Initializes empty object (mean=0, stddev=1ms)
		 */
		noisy_timestamp_t();

		/**
		 * Initializes clock based on given mean and std. dev.
		 *
		 * All given values must be in seconds.
		 *
		 * @param ts  The timestamp that represents the mean
		 * @param n   The noise (std. dev.) of this random variable
		 */
		noisy_timestamp_t(double ts, double n);

		/* init */

		/**
		 * Changes the observed timestamp to sample around.
		 *
		 * This value is used as the mean of a gaussian random
		 * variable, and should be a timestamp that was observed
		 * from the system clock of the system.
		 *
		 * @param ts   The new timestamp to sample around
		 */
		inline void set_timestamp(double ts)
		{
			this->timestamp_mean = ts;
		};

		/**
		 * Changes the modeled noise of the system clock.
		 *
		 * This value is used to change how spread out the
		 * sampling around this timestamp will be.  It will
		 * be used as the std. dev. of the modeled random
		 * variable.
		 *
		 * @param n   The system clock's modeled std. dev.
		 */
		inline void set_noise(double n)
		{
			this->stddev = n;
		};

		/* sample */

		/**
		 * Generates a random sample around current timestamp
		 *
		 * Given the current settings of the clock's mean time
		 * and the clock's noise, will generate a sample from
		 * the modeled gaussian distribution around the current
		 * timestamp.
		 *
		 * @return   Returns a randomly generated timestamp
		 */
		double generate_sample() const;
};

#endif
