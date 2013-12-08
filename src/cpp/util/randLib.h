#ifndef _RANDLIB_H_
#define _RANDLIB_H_

#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <stdint.h>

/*
*	RandLib Class
*
*	This class serves as a method to have randn(), rand(), and randi() MATLAB style
*	functions in C++ programs.
*
*
*	Sources: http://people.sc.fsu.edu/~jburkardt/cpp_src/ziggurat/ziggurat.html
*		   : http://www.cplusplus.com/forum/beginner/7445/
*	
*/
class randLib {

private:

	/*
	*	shr3(unsigned long int& jsr)
	*
	*	This function evaluates the SHR3 generator for integers.  The seed jsr is 
	*	updated on each function call.
	*/
	static inline uint64_t shr3(uint64_t& jsr);

	/*
	*	float r4_uni(unsigned long int& jsr)
	*
	*	This function generates a psuedo-random number in the range [0,1]
	*/
	static inline double r4_uni(uint64_t& jsr);

	/*
	*	r4_nor_setup(std::vector<int>& kn, std::vector<double>& fn, std::vector<double>& wn)
	*
	*	Calculates the required parameters for n4_nor
	*/
	static inline void r4_nor_setup(std::vector<int>& kn, std::vector<double>& fn, std::vector<double>& wn);

	/*
	*	double r4_nor(std::vector<int>& kn, std::vector<double>& fn, std::vector<double>& wn, unsigned long int& jsr)
	*
	*	Calculates a normally distributed random number from the distribution X~N(0,1)
	*/
	static double r4_nor(std::vector<int>& kn, std::vector<double>& fn, std::vector<double>& wn, uint64_t& jsr);

public:

	/*
	*	randn(std::vector<double>& randomNumbers, const unsigned int numNumbersToGenerate) 
	*
	*	This function wraps an implementation of the zigguart algorithm for generating
	*	psuedo-random numbers from a normal distribution with mean 0 and variance 1.
	*/
	static void randn(std::vector<double>& randomNumbers, const unsigned int numNumbersToGenerate);

	/*
	*	rand(std::vector<double>& randomNumbers, const unsigned int numNumbersToGenerate)
	*
	*	This function will return a psuedo-random number between the range [0,1].
	*	This is uses the standard c++ rand() function as its underlying generator.
	*/
	static void unifRand(std::vector<double>& randomNumbers, const unsigned int numNumbersToGenerate);

	/*
	*	randi(std::vector<unsigned int>& randomNumbers, const unsigned int numNumbersToGenerate, const unsigned int maxVal)
	*
	*	This function will return a psuedo-random integer in the range [1,2,3,...,maxVal].
	*	This also uses the standard rand() as the underlying generator
	*/
	static void randi(std::vector<unsigned int>& randomNumbers, const unsigned int numNumbersToGenerate, const unsigned int maxVal);

	/*
	*	phi(double x);
	*
	*	This function computes the normal cdf for unit variance and zero mean
	*	at position x
	*/
	static double phi(double x);
};



#endif
