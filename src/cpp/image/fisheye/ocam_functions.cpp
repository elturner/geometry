/*------------------------------------------------------------------------------
   Example code that shows the use of the 'cam2world" and 'world2cam" functions
   Shows also how to undistort images into perspective or panoramic images
   Copyright (C) 2008 DAVIDE SCARAMUZZA, ETH Zurich  
   Author: Davide Scaramuzza - email: davide.scaramuzza@ieee.org
------------------------------------------------------------------------------*/

#include "ocam_functions.h"
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

using namespace std;

#ifndef M_PI
#define M_PI 3.1415926535
#endif

//------------------------------------------------------------------------------
int get_ocam_model(struct ocam_model *myocam_model, const char *filename)
{
 double *pol        = myocam_model->pol;
 double *invpol     = myocam_model->invpol; 
 double *xc         = &(myocam_model->xc);
 double *yc         = &(myocam_model->yc); 
 double *c          = &(myocam_model->c);
 double *d          = &(myocam_model->d);
 double *e          = &(myocam_model->e);
 int    *width      = &(myocam_model->width);
 int    *height     = &(myocam_model->height);
 int *length_pol    = &(myocam_model->length_pol);
 int *length_invpol = &(myocam_model->length_invpol);
 FILE *f;
 char buf[CMV_MAX_BUF];
 char* ret;
 int i, n;
 
 //Open file
 if(!(f=fopen(filename,"r")))
 {
   printf("File %s cannot be opened\n", filename);				  
   return -1;
 }
 
 //Read polynomial coefficients
 ret = fgets(buf,CMV_MAX_BUF,f);
 n = fscanf(f,"\n");
 n = fscanf(f,"%d", length_pol);
 for (i = 0; i < *length_pol; i++)
 {
     n = fscanf(f," %lf",&pol[i]);
 }

 //Read inverse polynomial coefficients
 n = fscanf(f,"\n");
 ret = fgets(buf,CMV_MAX_BUF,f);
 n = fscanf(f,"\n");
 n = fscanf(f,"%d", length_invpol);
 for (i = 0; i < *length_invpol; i++)
 {
     n = fscanf(f," %lf",&invpol[i]);
 }
 
 //Read center coordinates
 n = fscanf(f,"\n");
 ret = fgets(buf,CMV_MAX_BUF,f);
 n = fscanf(f,"\n");
 n = fscanf(f,"%lf %lf\n", xc, yc);

 //Read affine coefficients
 ret = fgets(buf,CMV_MAX_BUF,f);
 n = fscanf(f,"\n");
 n = fscanf(f,"%lf %lf %lf\n", c,d,e);

 //Read image size
 ret = fgets(buf,CMV_MAX_BUF,f);
 n = fscanf(f,"\n");
 n = fscanf(f,"%d %d", height, width);

 ret = ret;
 n = n;
 fclose(f);
 return 0;
}

//------------------------------------------------------------------------------

/* the following defines are used to parse input binary files */
#define MAGIC_NUMBER "CALIB"
#define MAGIC_NUMBER_LEN 6

/* This function will parse the input binary file */
int get_ocam_model_bin(struct ocam_model* myocam_model,
                       std::string& camera_name,
                       const std::string& filename)
{
	char magic[MAGIC_NUMBER_LEN];
	ifstream infile;
	stringstream ss;
	int i;
	unsigned int xc, yc, len, h, w;

	/* attempt to open file */
	infile.open(filename.c_str(), ios::in | ios::binary);
	if(!(infile.is_open()))
		return -1; /* unable to open file */

	/* read magic number from file */
	infile.read(magic, MAGIC_NUMBER_LEN);
	if(!(infile.good()))
		return -2; /* could not read from file */
	
	/* check that filetype is correct */
	if(strcmp(magic, MAGIC_NUMBER))
		return -3; /* magic number is different */

	/* get the camera name */
	getline(infile, camera_name, '\0');
	
	/* read polynomial size and coefficient values */
	infile.read((char*) (&(len)), sizeof(len));
	myocam_model->length_pol = len;
	for(i = 0; i < myocam_model->length_pol; i++)
		infile.read((char*) (&(myocam_model->pol[i])),
		            sizeof(myocam_model->pol[i]));
	if(!(infile.good()))
		return -4; /* could not read from file */

	/* read image center coordinates */
	infile.read((char*)(&(xc)), sizeof(xc));
	myocam_model->xc = xc;
	infile.read((char*)(&(yc)), sizeof(yc));
	myocam_model->yc = yc;

	/* skew parameters */
	infile.read((char*) (&(myocam_model->c)), sizeof(myocam_model->c));
	infile.read((char*) (&(myocam_model->d)), sizeof(myocam_model->d));
	infile.read((char*) (&(myocam_model->e)), sizeof(myocam_model->e));

	/* image size */
	infile.read((char*) (&(w)), sizeof(w));
	myocam_model->width = w;
	infile.read((char*) (&(h)), sizeof(h));
	myocam_model->height = h;

	/* check file status */
	if(!(infile.good()))
		return -5; /* could not read from file */

	/* read in inverse polynomial approximation values */
	infile.read((char*) (&(len)), sizeof(len));
	myocam_model->length_invpol = len;
	for(i = 0; i < myocam_model->length_invpol; i++)
		infile.read((char*) (&(myocam_model->invpol[i])),
		            sizeof(myocam_model->invpol[i]));
	if(infile.fail())
		return -6; /* could not read from file */

	/* close file and return success */
	infile.close();
	return 0;
}

//------------------------------------------------------------------------------
void cam2world(double point3D[3], double point2D[2], struct ocam_model *myocam_model)
{
 double *pol    = myocam_model->pol;
 double xc      = (myocam_model->xc);
 double yc      = (myocam_model->yc); 
 double c       = (myocam_model->c);
 double d       = (myocam_model->d);
 double e       = (myocam_model->e);
 int length_pol = (myocam_model->length_pol); 
 double invdet  = 1/(c-d*e); // 1/det(A), where A = [c,d;e,1] as in the Matlab file

 double xp = invdet*(    (point2D[0] - xc) - d*(point2D[1] - yc) );
 double yp = invdet*( -e*(point2D[0] - xc) + c*(point2D[1] - yc) );
  
 double r   = sqrt(  xp*xp + yp*yp ); //distance [pixels] of  the point from the image center
 double zp  = pol[0];
 double r_i = 1;
 int i;
 
 for (i = 1; i < length_pol; i++)
 {
   r_i *= r;
   zp  += r_i*pol[i];
 }
 
 //normalize to unit norm
 double invnorm = 1/sqrt( xp*xp + yp*yp + zp*zp );
 
 point3D[0] = invnorm*xp;
 point3D[1] = invnorm*yp; 
 point3D[2] = invnorm*zp;
}

//------------------------------------------------------------------------------
void world2cam(double point2D[2], double point3D[3], struct ocam_model *myocam_model)
{
 double *invpol     = myocam_model->invpol; 
 double xc          = (myocam_model->xc);
 double yc          = (myocam_model->yc); 
 double c           = (myocam_model->c);
 double d           = (myocam_model->d);
 double e           = (myocam_model->e);
 //int    width       = (myocam_model->width);
 //int    height      = (myocam_model->height);
 int length_invpol  = (myocam_model->length_invpol);
 double norm        = sqrt(point3D[0]*point3D[0] + point3D[1]*point3D[1]);
 double theta       = atan(point3D[2]/norm);
 double t, t_i;
 double rho, x, y;
 double invnorm;
 int i;
  
  if (norm != 0) 
  {
    invnorm = 1/norm;
    t  = theta;
    rho = invpol[0];
    t_i = 1;

    for (i = 1; i < length_invpol; i++)
    {
      t_i *= t;
      rho += t_i*invpol[i];
    }

    x = point3D[0]*invnorm*rho;
    y = point3D[1]*invnorm*rho;
  
    point2D[0] = x*c + y*d + xc;
    point2D[1] = x*e + y   + yc;
  }
  else
  {
    point2D[0] = xc;
    point2D[1] = yc;
  }
}
//------------------------------------------------------------------------------
void create_perspecive_undistortion_LUT( CvMat *mapx, CvMat *mapy, struct ocam_model *ocam_model, float sf)
{
     int i, j;
     int width = mapx->cols; //New width
     int height = mapx->rows;//New height     
     float *data_mapx = mapx->data.fl;
     float *data_mapy = mapy->data.fl;
     float Nxc = height/2.0;
     float Nyc = width/2.0;
     float Nz  = -width/sf;
     double M[3];
     double m[2];
     
     for (i=0; i<height; i++)
         for (j=0; j<width; j++)
         {   
             M[0] = (i - Nxc);
             M[1] = (j - Nyc);
             M[2] = Nz;
             world2cam(m, M, ocam_model);
             *( data_mapx + i*width+j ) = (float) m[1];
             *( data_mapy + i*width+j ) = (float) m[0];
         }
}

//------------------------------------------------------------------------------
void create_panoramic_undistortion_LUT ( CvMat *mapx, CvMat *mapy, float Rmin, float Rmax, float xc, float yc )
{
     int i, j;
     float theta;
     int width = mapx->width;
     int height = mapx->height;     
     float *data_mapx = mapx->data.fl;
     float *data_mapy = mapy->data.fl;
     float rho;
     
     for (i=0; i<height; i++)
         for (j=0; j<width; j++)
         {
             theta = -((float)j)/width*2*M_PI; // Note, if you would like to flip the image, just inverte the sign of theta
             rho   = Rmax - (Rmax-Rmin)/height*i;
             *( data_mapx + i*width+j ) = yc + rho*sin(theta); //in OpenCV "x" is the
             *( data_mapy + i*width+j ) = xc + rho*cos(theta);             
         }
}
