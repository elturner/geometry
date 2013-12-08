/*
*	rotLib.cpp
*
*	This class of static function provides some basic roation
*	functions.
*
*/

#include "rotLib.h"
#include <iostream>

/*
	makeRFromGravHeading(Eigen::Matrix3d& Rout, double roll, double pitch,
						 double heading);

	This function forms the rotation matrix that corrisponds to the
	imu2world transform defined by the 2D heading and the roll and pitch
	of the system.  It is assumed in this function that the Z vector of the 
	imu corrisponds to the heading vector when projected into the XY plane
*/
bool rotLib::makeRFromGravHeading(Eigen::Matrix3d& Rout, double roll, 
									 double pitch, double heading) {
	
	// First we generate the rotation matrix from the pitch and roll
	Eigen::Matrix3d R1, RZ;
	rotLib::rpy2rot(roll, pitch, 0, R1);

	// Next we project the zvector of the imu into the world xy plane
	Eigen::Vector3d lv;
	lv << 0,0,1;
	lv = R1*lv;
	lv(2) = 0;
	lv = lv/lv.norm();
	double angle = atan2(lv(1),lv(0));

	// Make the correction matrix
	rotLib::rotZ(RZ,-heading+angle);
	Rout = RZ.transpose()*R1;
	return true;
}

/*
	rpy2rot(double roll, double pitch, double yaw, Eigen::Matrix3d &rotationMatrix);
	rpy2rot(const Eigen::Vector3d &orientation, Eigen::Matrix3d &rotationMatrix);

	This function converts a 3-2-1 euler orientation into the corrisponding
	rotation matrix.
*/
void rotLib::rpy2rot(double roll, double pitch, double yaw, Eigen::Matrix3d &rotationMatrix) {
	double cr = cos(roll);
	double sr = sin(roll);
	double cp = cos(pitch);
	double sp = sin(pitch);
	double cy = cos(yaw);
	double sy = sin(yaw);
		
	rotationMatrix(0,0) = cy*cp;
	rotationMatrix(0,1) = cy*sp*sr-sy*cr;
	rotationMatrix(0,2) = cy*cr*sp+sy*sr;
	rotationMatrix(1,0) = cp*sy;
	rotationMatrix(1,1) = sy*sp*sr+cy*cr;
	rotationMatrix(1,2) = sy*cr*sp-cy*sr;
	rotationMatrix(2,0) = -sp;
	rotationMatrix(2,1) = cp*sr;
	rotationMatrix(2,2) = cp*cr;	
}
void rotLib::rpy2rot(const Eigen::Vector3d &orientation, Eigen::Matrix3d &rotationMatrix) {
	double cr = cos(orientation(0));
	double sr = sin(orientation(0));
	double cp = cos(orientation(1));
	double sp = sin(orientation(1));
	double cy = cos(orientation(2));
	double sy = sin(orientation(2));

	rotationMatrix(0,0) = cy*cp;
	rotationMatrix(0,1) = cy*sp*sr-sy*cr;
	rotationMatrix(0,2) = cy*cr*sp+sy*sr;
	rotationMatrix(1,0) = cp*sy;
	rotationMatrix(1,1) = sy*sp*sr+cy*cr;
	rotationMatrix(1,2) = sy*cr*sp-cy*sr;
	rotationMatrix(2,0) = -sp;
	rotationMatrix(2,1) = cp*sr;
	rotationMatrix(2,2) = cp*cr;	
}

/*
	givenRotations(double a, double b, Eigen::VectorXd &returnedMat)

	This is a helper function for the rot2rpy family of functions.
*/
void rotLib::givenRotations(double a, double b, Eigen::VectorXd &returnedMat) {
	if(returnedMat.size() != 4) {
		returnedMat.resize(4);	
	}
	
	double c,s,r,t,u;
	if(b == 0) {
		c = a/std::abs(a);
		s = 0;
		r = std::abs(a);
	} else if(a == 0) {
		c = 0;
		s = b/std::abs(b);
		r = std::abs(b);
	} else if(std::abs(b) > std::abs(a)) {
		t = a/b;
		u = sqrt(1+pow(t,2))*(b/std::abs(b));
		s = 1/u;
		c = s*t;
		r = b*u;
	} else {
		t = b/a;
		u = sqrt(1+pow(t,2))*(a/std::abs(a));
		c = 1/u;
		s = c*t;
		r = a*u;
	}
	returnedMat(0) = r;
	returnedMat(1) = c;
	returnedMat(2) = s;
	returnedMat(3) = atan2(s,c);

	// Validate Answer
	if((-s*a+c*b) > (1e-10)) {
		std::cerr << "ERROR : Given Rotation's Failed Computation" << std::endl;
	}
}

/*
	rot2rpy(const Eigen::Matrix3d &rotationMatrix, Eigen::Vector3d &returnedOrientation);

	This function takes a rotation matrix and coverts it into the 3-2-1 euler
	angle representation.
*/
void rotLib::rot2rpy(const Eigen::Matrix3d &rotationMatrix, Eigen::Vector3d &returnedOrientation) {
	Eigen::VectorXd temp(4);

	givenRotations(rotationMatrix(0,0),rotationMatrix(1,0), temp);
	Eigen::Matrix3d Ryaw;
	Ryaw << temp(1), temp(2), 0, -temp(2), temp(1), 0, 0, 0, 1;
	returnedOrientation(2) = temp(3);

	givenRotations(temp(0),rotationMatrix(2,0),temp);
	Eigen::Matrix3d Rpitch;
	Rpitch << temp(1), 0, temp(2), 0, 1, 0, -temp(2), 0, temp(1);
	returnedOrientation(1) = -temp(3);

	Eigen::Matrix3d tempR = Rpitch*Ryaw*rotationMatrix;
	givenRotations(tempR(1,1),tempR(2,1), temp);
	Eigen::Matrix3d Rroll;
	Rroll << 1, 0, 0, 0, temp(1), temp(2), 0, -temp(2), temp(1);
	returnedOrientation(0) = temp(3);

	// Verification
	Eigen::Matrix3d Q = Rroll*tempR;
	for(int i = 0; i<3; i++) {
		for(int j = 0; j < 3; j++) {
			tempR(i,j) = abs(Q(i,j));
		}
	}
	for(int i = 0; i < 3; i++) {
		tempR(i,i) = 0;
	}
	if((Q(0,0) != 1 || Q(1,1) != 1 || Q(2,2) != 1) && tempR.sum() > 0.001) {
		std::cerr << "Matrix Decomposition into Euler Angles Failed!" << std::endl <<
					 "Is this really a rotation matrix\?" << std::endl;
	}
}

/* 
	rotX(Eigen::Matrix3d &Rout, double theta)
	rotY(Eigen::Matrix3d &Rout, double theta)
	rotZ(Eigen::Matrix3d &Rout, double theta)
	
	This group of three function creates a rotation matrix about the axis
	in question.  These are 3x3 rotation matricies.  Theta is assumed to be 
	in radians
*/
void rotLib::rotX(Eigen::Matrix3d &Rout, double theta) {
	double ct = cos(theta);
	double st = sin(theta);

	Rout(0,0) = 1;
	Rout(0,1) = 0;
	Rout(0,2) = 0;
	Rout(1,0) = 0;
	Rout(1,1) = ct;
	Rout(1,2) = -st;
	Rout(2,0) = 0;
	Rout(2,1) = st;
	Rout(2,2) = ct;
}
void rotLib::rotY(Eigen::Matrix3d &Rout, double theta) {
	double ct = cos(theta);
	double st = sin(theta);

	Rout(0,0) = ct;
	Rout(0,1) = 0;
	Rout(0,2) = st;
	Rout(1,0) = 0;
	Rout(1,1) = 1;
	Rout(1,2) = 0;
	Rout(2,0) = -st;
	Rout(2,1) = 0;
	Rout(2,2) = ct;
}
void rotLib::rotZ(Eigen::Matrix3d &Rout, double theta) {
	double ct = cos(theta);
	double st = sin(theta);

	Rout(0,0) = ct;
	Rout(0,1) = -st;
	Rout(0,2) = 0;
	Rout(1,0) = st;
	Rout(1,1) = ct;
	Rout(1,2) = 0;
	Rout(2,0) = 0;
	Rout(2,1) = 0;
	Rout(2,2) = 1;
}
