#include "randLib.h"

/*
*	RandLib Class
*
*	This class serves as a method to have randn(), rand(), and randi() MATLAB style
*	functions in C++ programs.
*
*/

/*
*	shr3(unsigned long int& jsr)
*
*	This function evaluates the SHR3 generator for integers.  The seed jsr is 
*	updated on each function call.
*/
uint64_t randLib::shr3(uint64_t &jsr) {
	uint64_t value = jsr;
	jsr = (jsr^(jsr << 13));
	jsr = (jsr^(jsr >> 17));
	jsr = (jsr^(jsr << 5));
	return value+jsr;
}

/*
*	float r4_uni(unsigned long int& jsr)
*
*	This function generates a psuedo-random number in the range [0,1]
*/
double randLib::r4_uni(uint64_t& jsr) {
	
	uint64_t jsr_input = jsr;
	jsr = (jsr^(jsr << 13));
	jsr = (jsr^(jsr >> 17));
	jsr = (jsr^(jsr << 5));
	return fmod(0.5 + (double)(jsr_input+jsr)/ 65536.0 / 65536.0, 1.0);
}

/*
*	r4_nor_setup
*
*	Calculates the required parameters for n4_nor
*
*/
void randLib::r4_nor_setup(std::vector<int>& kn, std::vector<double>& fn, std::vector<double>& wn) {

	// Make sure the vectors are the correct size
	if(kn.size() != 128) {
		kn.resize(128);
	}
	if(fn.size() != 128) {
		fn.resize(128);
	}
	if(wn.size() != 128) {
		wn.resize(128);
	}

	// Declare some constants used in the algorithm
	double dn = 3.442619855899;
	double m1 = 2147483648.0;
	double tn =  3.442619855899;
	const double vn = 9.91256303526217E-03;
	double q;
	
	// Magically calculate the values
	q = vn / exp( -0.5*dn*dn );
	kn[0] = (int)((dn/q)*m1);
	kn[1] = 0;

	wn[0] = (double)(q/m1);
	wn[127] = (double)(dn/m1);

	fn[0] = 1.0;
	fn[127] = (double)(exp(-0.5*dn*dn));

	for(int i = 126; 1 <= i; i--) {
		dn = sqrt(-2.0*log(vn/dn+exp(-0.5*dn*dn)));
		kn[i+1] = (int)((dn/tn)*m1);
		tn = dn;
		fn[i] = (double)(exp(-0.5*dn*dn));
		wn[i] = (double)(dn/m1);
	}
	return;
}

/*
*	double r4_nor(std::vector<int>& kn, std::vector<double>& fn, std::vector<double>& wn, unsigned long int& jsr)
*
*	Calculates a normally distributed random number from the distribution X~N(0,1)
*/
double randLib::r4_nor(std::vector<int>& kn, std::vector<double>& fn, std::vector<double>& wn, uint64_t& jsr) {

	// Declare some stuff
	int hz, iz;
	const double r = 3.442620;
	float value, x, y;

	hz = shr3(jsr);
	iz = (hz&127);
	if( abs(hz) < kn[iz] ) {
		value = (double)(hz)*wn[iz];
	}
	else {
		for( ; ; ) {
			if(iz == 0) {
				for( ; ; ) {
					x = - 0.2904764 * log ( r4_uni ( jsr ) );
					y = - log ( r4_uni ( jsr ) );
					if ( x * x <= y + y ); {
						break;
					}
				}

				if ( hz <= 0 ) {
					value = - r - x;
				}
				else {
					value = + r + x;
				}
				break;
			}

			x = ( double ) ( hz ) * wn[iz];

			if ( fn[iz] + r4_uni ( jsr ) * ( fn[iz-1] - fn[iz] ) < exp ( - 0.5 * x * x ) ) {
				value = x;
				break;
			}

			hz = shr3 ( jsr );
			iz = ( hz & 127 );

			if ( abs ( hz ) < kn[iz] ) {
				value = ( float ) ( hz ) * wn[iz];
				break;
			}
		}
	}
	return value;
}

/*
*	randn(std::vector<double>& randomNumbers, unsigned int numNumbersToGenerate) 
*
*	This function wraps an implementation of teh zigguart algorithm for generating
*	psuedo-random numbers from a normal distribution with mean 0 and variance 1.
*
*/
void randLib::randn(std::vector<double>& randomNumbers, const unsigned int numNumbersToGenerate) {

	// Seed the random number generator with this
	uint64_t seed = (uint64_t)rand();
	
	std::vector<int> kn(128);
	std::vector<double> wn(128);
	std::vector<double> fn(128);

	// Setup
	randLib::r4_nor_setup(kn, fn, wn);

	// Generate the random numbers
	randomNumbers.resize(numNumbersToGenerate);
	for(unsigned int i = 0; i < numNumbersToGenerate; i++) {
		randomNumbers[i] = r4_nor(kn, fn, wn, seed);
	}
	return;
}

/*
*	rand(std::vector<double>& randomNumbers, unsigned int numNumbersToGenerate)
*
*	This function will return a psuedo-random number between the range [0,1].
*	This is uses the standard c++ rand() function as its underlying generator.
*/
void randLib::unifRand(std::vector<double>& randomNumbers, const unsigned int numNumbersToGenerate) {

	// Generate the random numbers
	randomNumbers.resize(numNumbersToGenerate);
	for(unsigned int i = 0; i < numNumbersToGenerate; i++) {
		randomNumbers[i] = rand() / double(RAND_MAX);
	}
	return;
}

/*
*	randi(std::vector<unsigned int>& randomNumbers, const unsigned int numNumbersToGenerate, const unsigned int maxVal)
*
*	This function will return a psuedo-random integer in the range [1,2,3,...,maxVal].
*	This also uses the standard rand() as the underlying generator
*/
void randLib::randi(std::vector<unsigned int>& randomNumbers, const unsigned int numNumbersToGenerate, const unsigned int maxVal) {

	unsigned int guard;

	if(maxVal == 0) {
		randomNumbers.resize(numNumbersToGenerate,0);
		return;
	}

	// Generate the random numbers
	randomNumbers.resize(numNumbersToGenerate);
	for(unsigned int i = 0; i < numNumbersToGenerate; i++) {
		guard = (long)((rand() / double(RAND_MAX))*maxVal)+1;
		randomNumbers[i] = (guard > maxVal)? maxVal : guard;
	}
	return;
}

/*
*	phi(double x);
*
*	This function computes the normal cdf for unit variance and zero mean
*	at position x
*/
double randLib::phi(double x) {
	
	// constants
    double a1 =  0.254829592;
    double a2 = -0.284496736;
    double a3 =  1.421413741;
    double a4 = -1.453152027;
    double a5 =  1.061405429;
    double p  =  0.3275911;

	// Save the sign of x
    int sign = 1;
    if (x < 0)
        sign = -1;
    x = fabs(x)/sqrt(2.0);

	// Do the computation
	double t = 1.0/(1.0 + p*x);
    double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

    return 0.5*(1.0 + sign*y);
}

