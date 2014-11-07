/** math.cpp **/

#include <cmath>

float clampi(const int value, const int min, const int max) {
	if(value < min) return min;
	else if(value > max) return max;
	else return value;
}

float clampf(const float value, const float min, const float max) {
	if(value < min) return min;
	else if(value > max) return max;
	else return value;
}

float calc_distance(const float x1, const float y1, const float x2, const float y2) {
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

float calc_distance_fast(const float x1, const float y1, const float x2, const float y2) {
	return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

float mix(const float x1, const float x2, const float y1, const float y2, const float x) {
	return (y1 - y2) / (x1 - x2) * (x - x1) + y1;
}

float calc_angle(const float x1, const float y1, const float x2, const float y2) {
	return atan2(y2 - y1, x2 - x1);
}

float min(const float a, const float b) {
	return a < b ? a : b;
}

float max(const float a, const float b) {
	return a > b ? a : b;
}
