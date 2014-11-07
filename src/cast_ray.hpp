/** cast_ray.hpp **/

#ifndef CAST_RAY_HPP
#define CAST_RAY_HPP

#include "global.hpp"
#include "polygon.hpp"

bool cast_ray(polygon_c &polygon, float &a, float &b, float &c, float &rx, float &ry, float &rz, cfloat J, cfloat K, cfloat L, cfloat x, cfloat y, cfloat z);

#endif
