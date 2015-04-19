#ifndef _RAY_H_
#define _RAY_H_

#include "vector3.h"

/*
 * Ray class, for use with the optimized ray-box intersection test
 * described in:
 *
 *      Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
 *      "An Efficient and Robust Ray-Box Intersection Algorithm"
 *      Journal of graphics tools, 10(1):49-54, 2005
 * 
 */

template<typename T>
class Ray {
  public:
    Ray() { }
    Ray(Vector3<T> o, Vector3<T> d) {
      origin = o;
      direction = d;
      inv_direction = Vector3<T>(1/d.x(), 1/d.y(), 1/d.z());
      sign[0] = (inv_direction.x() < 0);
      sign[1] = (inv_direction.y() < 0);
      sign[2] = (inv_direction.z() < 0);
    }
    Ray(const Ray &r) {
      origin = r.origin;
      direction = r.direction;
      inv_direction = r.inv_direction;
      sign[0] = r.sign[0]; sign[1] = r.sign[1]; sign[2] = r.sign[2];
    }

    Vector3<T> origin;
    Vector3<T> direction;
    Vector3<T> inv_direction;
    int sign[3];
};

#endif // _RAY_H_
