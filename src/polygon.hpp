/** polygon.hpp **/

#ifndef POLYGON_HPP
#define POLYGON_HPP

#include "global.hpp"

//This class represents any polygon that has 3 vertexes in a 3D space.
//Swap changes the order of the vertexes if ray casting is not possible with the current order. This, however, does not affect the way the polygon is drawn.
class polygon_c {
	public:
		float x1,x2,x3,y1,y2,y3,z1,z2,z3,tx1,tx2,tx3,ty1,ty2,ty3;
		float nx,ny,nz,tx,ty,tz,bx,by,bz;
		float minx,maxx,miny,maxy,minz,maxz;
		polygon_c(cfloat X1, cfloat Y1, cfloat Z1, cfloat X2, cfloat Y2, cfloat Z2, cfloat X3, cfloat Y3, cfloat Z3, cfloat TX1, cfloat TY1, cfloat TX2, cfloat TY2, cfloat TX3, cfloat TY3);
		void swap();
};

#endif
