#ifndef ERROR_CODES_H
#define ERROR_CODES_H

/**
 * @file error_codes.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains macros and functions that
 * are used to log errors and warnings.
 */

#include <stdio.h>
#include <iostream>

/* the following macro will propegate error codes up functions,
 * so that the source of an error is easy to trace. */
#define PROPEGATE_ERROR(curr, prev) ( (curr) + 100*(prev) )

/* Will suppress "unused parameter" warning */
#define MARK_USED(X)  ((void)(&(X)))

/*------------------------------*/
/* pretty-print error functions */
/*------------------------------*/

/* which command used depends on OS */
#ifdef _WIN32
	#define PRINTFUN __FUNCSIG__
#else
 	#define PRINTFUN __PRETTY_FUNCTION__
#endif

/* post errors to cerr and either do nothing, die, or return */
#define POST_FATAL_ERROR(e, exitCode) { \
 	std::cerr << "ERROR IN FILE : " << __FILE__ << '\n' \
 		   << "LINE : " << __LINE__ << '\n' \
 		   << "FUNCTION : " << PRINTFUN << '\n' \
 		   << "MESSAGE : " << e << std::endl << std::endl; \
 		   exit(exitCode);}
#define POST_RETURN_ERROR(e, returnCode) {\
 	std::cerr << "ERROR IN FILE : " << __FILE__ << '\n' \
 		   << "LINE : " << __LINE__ << '\n' \
 		   << "FUNCTION : " << PRINTFUN << '\n' \
 		   << "MESSAGE : " << e << std::endl << std::endl; \
 		   return(returnCode);}		
#define POST_ERROR(e) { \
 std::cerr << "ERROR IN FILE : " << __FILE__ << '\n' \
 		   << "LINE : " << __LINE__ << '\n' \
 		   << "FUNCTION : " << PRINTFUN << '\n' \
 		   << "MESSAGE : " << e << std::endl << std::endl;}

#endif
