#ifndef H_TRIBOX3_H
#define H_TRIBOX3_H

/********************************************************/

/* AABB-triangle overlap test code                      */

/* by Tomas Akenine-Möller                              */

/* Function: int triBoxOverlap(float boxcenter[3],      */

/*          float boxhalfsize[3],float triverts[3][3]); */

/* History:                                             */

/*   2001-03-05: released the code in its first version */

/*   2001-06-18: changed the order of the tests, faster */

/*                                                      */

/* Acknowledgement: Many thanks to Pierre Terdiman for  */

/* suggestions and discussions on how to optimize code. */

/* Thanks to David Hunt for finding a ">="-bug!         */

/********************************************************/

#include <math.h>
#include <cmath>

#include <stdio.h>

#define XXXXX 0
#define YYYYY 1
#define ZZZZZ 2
#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0]; 
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2]; 
#define FINDMINMAX(x0,x1,x2,min,max) \
  min = max = x0;   \
  if(x1<min) min=x1;\
  if(x1>max) max=x1;\
  if(x2<min) min=x2;\
  if(x2>max) max=x2;

template<typename T>
int planeBoxOverlap(const T normal[3], const T vert[3], const T maxbox[3])	// -NJMP-

{
  int q;
  T vmin[3],vmax[3],v;
  for(q=XXXXX;q<=ZZZZZ;q++)
  {
    v=vert[q];					// -NJMP-
    if(normal[q]>T(0))
    {
      vmin[q]=-maxbox[q] - v;	// -NJMP-
      vmax[q]= maxbox[q] - v;	// -NJMP-
    }
    else
    {
      vmin[q]= maxbox[q] - v;	// -NJMP-
      vmax[q]=-maxbox[q] - v;	// -NJMP-
    }
  }
  if(DOT(normal,vmin)>T(0)) return 0;	// -NJMP-
  if(DOT(normal,vmax)>=T(0)) return 1;	// -NJMP-
  return 0;

}

/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)			   \
	p0 = a*v0[YYYYY] - b*v0[ZZZZZ];			       	   \
	p2 = a*v2[YYYYY] - b*v2[ZZZZZ];			       	   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[YYYYY] + fb * boxhalfsize[ZZZZZ];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)			   \
	p0 = a*v0[YYYYY] - b*v0[ZZZZZ];			           \
	p1 = a*v1[YYYYY] - b*v1[ZZZZZ];			       	   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[YYYYY] + fb * boxhalfsize[ZZZZZ];   \
	if(min>rad || max<-rad) return 0;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)			   \
	p0 = -a*v0[XXXXX] + b*v0[ZZZZZ];		      	   \
	p2 = -a*v2[XXXXX] + b*v2[ZZZZZ];	       	       	   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[XXXXX] + fb * boxhalfsize[ZZZZZ];   \
	if(min>rad || max<-rad) return 0;
#define AXISTEST_Y1(a, b, fa, fb)			   \
	p0 = -a*v0[XXXXX] + b*v0[ZZZZZ];		      	   \
	p1 = -a*v1[XXXXX] + b*v1[ZZZZZ];	     	       	   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[XXXXX] + fb * boxhalfsize[ZZZZZ];   \
	if(min>rad || max<-rad) return 0;

/*======================== Z-tests ========================*/
#define AXISTEST_Z12(a, b, fa, fb)			   \
	p1 = a*v1[XXXXX] - b*v1[YYYYY];			           \
	p2 = a*v2[XXXXX] - b*v2[YYYYY];			       	   \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
	rad = fa * boxhalfsize[XXXXX] + fb * boxhalfsize[YYYYY];   \
	if(min>rad || max<-rad) return 0;
#define AXISTEST_Z0(a, b, fa, fb)			   \
	p0 = a*v0[XXXXX] - b*v0[YYYYY];				   \
	p1 = a*v1[XXXXX] - b*v1[YYYYY];			           \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[XXXXX] + fb * boxhalfsize[YYYYY];   \
	if(min>rad || max<-rad) return 0;

template<typename T>
int triBoxOverlap(const T boxcenter[3],
  const T boxhalfsize[3], 
  const T triverts[3][3],
  const T normal[3])
{
  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
  /*       this gives 3x3=9 more tests */
   T v0[3],v1[3],v2[3];
   T min,max,p0,p1,p2,rad,fex,fey,fez;		// -NJMP- "d" local variable removed
   T e0[3],e1[3],e2[3];

   /* This is the fastest branch on Sun */
   /* move everything so that the boxcenter is in (0,0,0) */
   SUB(v0,triverts[0],boxcenter);
   SUB(v1,triverts[1],boxcenter);
   SUB(v2,triverts[2],boxcenter);

   /* compute triangle edges */
   SUB(e0,v1,v0);      /* tri edge 0 */
   SUB(e1,v2,v1);      /* tri edge 1 */
   SUB(e2,v0,v2);      /* tri edge 2 */

   /* Bullet 3:  */
   /*  test the 9 tests first (this was faster) */
   fex = std::abs(e0[XXXXX]);
   fey = std::abs(e0[YYYYY]);
   fez = std::abs(e0[ZZZZZ]);
   AXISTEST_X01(e0[ZZZZZ], e0[YYYYY], fez, fey);
   AXISTEST_Y02(e0[ZZZZZ], e0[XXXXX], fez, fex);
   AXISTEST_Z12(e0[YYYYY], e0[XXXXX], fey, fex);

   fex = std::abs(e1[XXXXX]);
   fey = std::abs(e1[YYYYY]);
   fez = std::abs(e1[ZZZZZ]);
   AXISTEST_X01(e1[ZZZZZ], e1[YYYYY], fez, fey);
   AXISTEST_Y02(e1[ZZZZZ], e1[XXXXX], fez, fex);
   AXISTEST_Z0(e1[YYYYY], e1[XXXXX], fey, fex);

   fex = std::abs(e2[XXXXX]);
   fey = std::abs(e2[YYYYY]);
   fez = std::abs(e2[ZZZZZ]);
   AXISTEST_X2(e2[ZZZZZ], e2[YYYYY], fez, fey);
   AXISTEST_Y1(e2[ZZZZZ], e2[XXXXX], fez, fex);
   AXISTEST_Z12(e2[YYYYY], e2[XXXXX], fey, fex);

   /* Bullet 1: */
   /*  first test overlap in the {x,y,z}-directions */
   /*  find min, max of the triangle each direction, and test for overlap in */
   /*  that direction -- this is equivalent to testing a minimal AABB around */
   /*  the triangle against the AABB */

   /* test in X-direction */
   FINDMINMAX(v0[XXXXX],v1[XXXXX],v2[XXXXX],min,max);
   if(min>boxhalfsize[XXXXX] || max<-boxhalfsize[XXXXX]) return 0;

   /* test in Y-direction */
   FINDMINMAX(v0[YYYYY],v1[YYYYY],v2[YYYYY],min,max);
   if(min>boxhalfsize[YYYYY] || max<-boxhalfsize[YYYYY]) return 0;

   /* test in Z-direction */
   FINDMINMAX(v0[ZZZZZ],v1[ZZZZZ],v2[ZZZZZ],min,max);
   if(min>boxhalfsize[ZZZZZ] || max<-boxhalfsize[ZZZZZ]) return 0;

   /* Bullet 2: */
   /*  test if the box intersects the plane of the triangle */
   if(!planeBoxOverlap<T>(normal,v0,boxhalfsize)) return 0;	// -NJMP-

   return 1;   /* box and triangle overlaps */

}

#endif
