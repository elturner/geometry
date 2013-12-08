/*
*	rotLib.h
*
*	This class of static function provides some basic roation
*	functions.
*
*/

#ifndef _H_ROTLIB_
#define _H_ROTLIB_

#include <eigen3/Eigen/Core>
#include <cmath>
#include <iostream>

class rotLib {

private:

	/*
		givenRotations(double a, double b, Eigen::VectorXd &returnedMat)

		This is a helper function for the rot2rpy family of functions.
	*/
	static void givenRotations(double a, double b, Eigen::VectorXd &returnedMat);


public:

	/*
		makeRFromGravHeading(Eigen::Matrix3d& Rout, double roll, double pitch,
							 double heading);

		This function forms the rotation matrix that corrisponds to the
		imu2world transform defined by the 2D heading and the roll and pitch
		of the system.  It is assumed in this function that the Z vector of the 
		imu corrisponds to the heading vector when projected into the XY plane
	*/
	static bool makeRFromGravHeading(Eigen::Matrix3d& Rout, double roll, 
									 double pitch, double heading);

	/*
		rpy2rot(double roll, double pitch, double yaw, Eigen::Matrix3d &rotationMatrix);
		rpy2rot(const Eigen::Vector3d &orientation, Eigen::Matrix3d &rotationMatrix);

		This function converts a 3-2-1 euler orientation into the corrisponding
		rotation matrix.
	*/
	static void rpy2rot(double roll, double pitch, double yaw, Eigen::Matrix3d &rotationMatrix);
	static void rpy2rot(const Eigen::Vector3d &orientation, Eigen::Matrix3d &rotationMatrix);

	/*
		rot2rpy(const Eigen::Matrix3d &rotationMatrix, Eigen::Vector3d &returnedOrientation);

		This function takes a rotation matrix and coverts it into the 3-2-1 euler
		angle representation.
	*/
	static void rot2rpy(const Eigen::Matrix3d &rotationMatrix, Eigen::Vector3d &returnedOrientation);

	/* 
		rotX(Eigen::Matrix3d &Rout, double theta)
		rotY(Eigen::Matrix3d &Rout, double theta)
		rotZ(Eigen::Matrix3d &Rout, double theta)
	
		This group of three function creates a rotation matrix about the axis
		in question.  These are 3x3 rotation matricies.  Theta is assumed to be 
		in radians
	*/
	static void rotX(Eigen::Matrix3d &Rout, double theta);
	static void rotY(Eigen::Matrix3d &Rout, double theta);
	static void rotZ(Eigen::Matrix3d &Rout, double theta);

};

#endif
