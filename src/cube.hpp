/** cube.hpp **/

#ifndef CUBE_HPP
#define CUBE_HPP

#include "global.hpp"
#include "polygon.hpp"
#include <vector>

//This class represents a cuboid (not a cube a matter of fact)
//Cubes are used to quickly test wether a ray can hit groups of polygons or cubes by placing cubes around them
//Testing wether a ray hits a cube is way faster than testing wether it hits a polygon
class cube_c {
	private:
		float minx, maxx, miny, maxy, minz, maxz;
		bool hit;

	public:
		cube_c(const std::vector<polygon_c> &polygons, cuint id1, cuint id2, cuint id3, cuint id4);
		cube_c(const std::vector<cube_c> &cubes, cuint id1, cuint id2, cuint id3, cuint id4);
		cube_c();
		bool test_hit(cfloat J, cfloat K, cfloat L, cfloat x, cfloat y, cfloat z) const;
};

#endif
