/** math.hpp **/

#ifndef MATH_HPP
#define MATH_HPP

#define PI 3.14159265f

float clampi(const int value, const int min, const int max);
float clampf(const float value, const float min, const float max);
float calc_distance(const float x1, const float y1, const float x2, const float y2);
float calc_distance_fast(const float x1, const float y1, const float x2, const float y2);
float mix(const float x1, const float x2, const float y1, const float y2, const float x);
float calc_angle(const float x1, const float y1, const float x2, const float y2);
float min(const float a, const float b);
float max(const float a, const float b);

#endif
