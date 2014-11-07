/** polygon.cpp **/

#include "polygon.hpp"
#include "math.hpp"
#include <cmath>

//In this program texture coordinates are not used to determine the position in the texture to be drawn
//	and therefore there is no function for the polygon to return the correct texture position for a certain position in the polygon
//Texture positions are used in this program only to determine the tangent and binormal vectors
polygon_c::polygon_c(cfloat X1, cfloat Y1, cfloat Z1,
		cfloat X2, cfloat Y2, cfloat Z2,
		cfloat X3, cfloat Y3, cfloat Z3,
		cfloat TX1, cfloat TY1,
		cfloat TX2, cfloat TY2,
		cfloat TX3, cfloat TY3) {
	x1 = X1; x2 = X2; x3 = X3;
	y1 = Y1; y2 = Y2; y3 = Y3;
	z1 = Z1; z2 = Z2; z3 = Z3;
	tx1 = TX1; tx2 = TX2; tx3 = TX3;
	ty1 = TY1; ty2 = TY2; ty3 = TY3;
	//Calculate normal vector
	nx = (y2 - y1) * (z3 - z1) - (z2 - z1) * (y3 - y1);
	ny = (z2 - z1) * (x3 - x1) - (x2 - x1) * (z3 - z1);
	nz = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
	cfloat l = sqrt(nx * nx + ny * ny + nz * nz);
	nx/= l;
	ny/= l;
	nz/= l;
	//Calculate tangent vector
	cfloat div1 = (ty2 - ty1) * (tx3 - tx1) - (ty3 - ty1) * (tx2 - tx1);
	tx = ((ty2 - ty1) * (x3 - x1) - (ty3 - ty1) * (x2 - x1)) / div1;
	ty = ((ty2 - ty1) * (y3 - y1) - (ty3 - ty1) * (y2 - y1)) / div1;
	tz = ((ty2 - ty1) * (z3 - z1) - (ty3 - ty1) * (z2 - z1)) / div1;
	cfloat l2 = -sqrt(tx * tx + ty * ty + tz * tz);
	tx/= l2;
	ty/= l2;
	tz/= l2;
	//Calculate binormal vector
	cfloat div2 = (tx2 - tx1) * (ty3 - ty1) - (tx3 - tx1) * (ty2 - ty1);
	bx = ((tx2 - tx1) * (x3 - x1) - (tx3 - tx1) * (x2 - x1)) / div2;
	by = ((tx2 - tx1) * (y3 - y1) - (tx3 - tx1) * (y2 - y1)) / div2;
	bz = ((tx2 - tx1) * (z3 - z1) - (tx3 - tx1) * (z2 - z1)) / div2;
	cfloat l3 = -sqrt(bx * bx + by * by + bz * bz);
	bx/= l3;
	by/= l3;
	bz/= l3;
	//Set mins and maxs for the cube collision
	//These values represent a cube (actually a cuboid) around the polygon
	//The values are first used to determine if the ray can even hit the polygon as it is faster to test whether a ray hits a cube than a polygon
	minx = min(x1, min(x2, x3)) - 0.001;
	maxx = max(x1, max(x2, x3)) + 0.001;
	miny = min(y1, min(y2, y3)) - 0.001;
	maxy = max(y1, max(y2, y3)) + 0.001;
	minz = min(z1, min(z2, z3)) - 0.001;
	maxz = max(z1, max(z2, z3)) + 0.001;
}

//Change the order of the vertexes
//Doesn't affect the way the polygon is drawn in any way
void polygon_c::swap() {
	cfloat xtemp = x1, ytemp = y1, ztemp = z1, txtemp = tx1, tytemp = ty1;
	x1 = x2; y1 = y2; z1 = z2; tx1 = tx2; ty1 = ty2;
	x2 = x3; y2 = y3; z2 = z3; tx2 = tx3; ty2 = ty3;
	x3 = xtemp; y3 = ytemp; z3 = ztemp; tx3 = txtemp; ty3 = tytemp;
}
