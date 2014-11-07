/** cast_ray.cpp **/

#include "global.hpp"
#include "polygon.hpp"
#include "math.hpp"

//Makes the ray collision calculation
//Returns false for cast_ray function to try to calculate with different values if something is about to be divided by zero
bool cast_ray2(float &a, float &b, float &c, float &rx, float &ry, float &rz,
	cfloat A, cfloat B, cfloat C,
	cfloat D, cfloat E, cfloat F,
	cfloat G, cfloat H, cfloat I,
	cfloat J, cfloat K, cfloat L,
	cfloat M, cfloat N, cfloat O) {

	//This formula was not actually copied from anywhere, it's all made by me :)
	if(D == 0) return false;
	cfloat four = H * D - G * E;
	if(four == 0) return false;

	cfloat one = N * D - M * E;
	cfloat two = I * D - G * F;
	cfloat three = O * D - M * F;

	cfloat temp = one * two - three * four;
	if(temp == 0) return false;

	cfloat JA = J - A;
	cfloat five = D * (K - B) - E * JA;
	cfloat six = D * (L - C) - F * JA;

	c = (four * six - two * five) / temp;

	rx = J + c * M;
	ry = K + c * N;
	rz = L + c * O;
	b = (one * c + five) / four;
	a = (rx - A - b * G) / D;
	return true;
}

//#define DRAW_CUBE

//This function calls a function that calculates if a ray hits a polygon
//a, b, c, rx, ry and rz are returned
//a and b represent the values in where the ray hits the plane where the polygon lies
	//a is the multiplier for vector vertex1 -> vertex2 and b is multiplier for vector vertex1 -> vertex3 of the polygon
	//So, if a < 0 || b < 0 || a + b > 1 the ray does not hit the polygon
//rx, ry, rz is the location of the collision in the 3D space
//J, K and L are the position of the viewer (x, y, z)
//x, y and z are the position of the target (x, y, z) the viewer is looking at
//c is the multiplier of the viewer vector for the collision (kind of like the distance from the viewer)
	//c is used for depth testing and testing if something is between a light and a position in 3D space
//returns false if the ray doesn't hit the cube around the polygon
bool cast_ray(polygon_c &polygon, float &a, float &b, float &c, float &rx, float &ry, float &rz,
		cfloat J, cfloat K, cfloat L, cfloat x, cfloat y, cfloat z) {
	cfloat M = x - J;
	cfloat N = y - K;
	cfloat O = z - L;
	//Check the cube collision
	cfloat x1 = (polygon.minx - J) / M;
	cfloat x2 = (polygon.maxx - J) / M;
	cfloat y1 = (polygon.miny - K) / N;
	cfloat y2 = (polygon.maxy - K) / N;
	float minv = max(min(x1, x2), min(y1, y2));
	float maxv = min(max(x1, x2), max(y1, y2));
	if(minv > maxv) return false;
	cfloat z1 = (polygon.minz - L) / O;
	cfloat z2 = (polygon.maxz - L) / O;
	minv = max(minv, min(z1, z2));
	maxv = min(maxv, max(z1, z2));
	if(minv > maxv) return false;
	#ifdef DRAW_CUBE
		a = 0.25;
		b = 0.25;
		c = (minv + maxv) / 2.0;
		rx = J + c * M;
		ry = K + c * N;
		rz = L + c * O;
		return true;
	#endif
	//Try to make the calculation three times and swap the polygon if it failed
	//Only swap 3 times at max because after 3 swaps the order of the vertexes in the polygon is the same as it originally was
	for(uchar t=0;t<3;t++) {
		cfloat A = polygon.x1;
		cfloat B = polygon.y1;
		cfloat C = polygon.z1;
		cfloat D = polygon.x2 - A;
		cfloat E = polygon.y2 - B;
		cfloat F = polygon.z2 - C;
		cfloat G = polygon.x3 - A;
		cfloat H = polygon.y3 - B;
		cfloat I = polygon.z3 - C;
		//Try with different orders of the values
		if(cast_ray2(a, b, c, rx, ry, rz, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O)) return true;
		if(cast_ray2(a, b, c, rz, rx, ry, C, A, B, F, D, E, I, G, H, L, J, K, O, M, N)) return true;
		if(cast_ray2(a, b, c, ry, rz, rx, B, C, A, E, F, D, H, I, G, K, L, J, N, O, M)) return true;
		polygon.swap();
	}
	return false; //It should be impossible to ever get this far
}
