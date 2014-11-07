/** cube.cpp **/

#include "cube.hpp"
#include "math.hpp"

cube_c::cube_c(const std::vector<polygon_c> &polygons, cuint id1, cuint id2, cuint id3, cuint id4): hit(false) {
	minx = min(polygons.at(id1).minx, min(polygons.at(id2).minx, min(polygons.at(id3).minx, polygons.at(id4).minx)));
	maxx = max(polygons.at(id1).maxx, max(polygons.at(id2).maxx, max(polygons.at(id3).maxx, polygons.at(id4).maxx)));
	miny = min(polygons.at(id1).miny, min(polygons.at(id2).miny, min(polygons.at(id3).miny, polygons.at(id4).miny)));
	maxy = max(polygons.at(id1).maxy, max(polygons.at(id2).maxy, max(polygons.at(id3).maxy, polygons.at(id4).maxy)));
	minz = min(polygons.at(id1).minz, min(polygons.at(id2).minz, min(polygons.at(id3).minz, polygons.at(id4).minz)));
	maxz = max(polygons.at(id1).maxz, max(polygons.at(id2).maxz, max(polygons.at(id3).maxz, polygons.at(id4).maxz)));
}

cube_c::cube_c(const std::vector<cube_c> &cubes, cuint id1, cuint id2, cuint id3, cuint id4): hit(false) {
	minx = min(cubes.at(id1).minx, min(cubes.at(id2).minx, min(cubes.at(id3).minx, cubes.at(id4).minx)));
	maxx = max(cubes.at(id1).maxx, max(cubes.at(id2).maxx, max(cubes.at(id3).maxx, cubes.at(id4).maxx)));
	miny = min(cubes.at(id1).miny, min(cubes.at(id2).miny, min(cubes.at(id3).miny, cubes.at(id4).miny)));
	maxy = max(cubes.at(id1).maxy, max(cubes.at(id2).maxy, max(cubes.at(id3).maxy, cubes.at(id4).maxy)));
	minz = min(cubes.at(id1).minz, min(cubes.at(id2).minz, min(cubes.at(id3).minz, cubes.at(id4).minz)));
	maxz = max(cubes.at(id1).maxz, max(cubes.at(id2).maxz, max(cubes.at(id3).maxz, cubes.at(id4).maxz)));
}

//This is an empty cube and any ray will always hit it
cube_c::cube_c(): hit(true) {}

bool cube_c::test_hit(cfloat J, cfloat K, cfloat L, cfloat x, cfloat y, cfloat z) const {
	if(hit) return true;
	cfloat M = x - J;
	cfloat N = y - K;
	cfloat O = z - L;
	//Check the cube collision
	cfloat x1 = (minx - J) / M;
	cfloat x2 = (maxx - J) / M;
	cfloat y1 = (miny - K) / N;
	cfloat y2 = (maxy - K) / N;
	float minv = max(min(x1, x2), min(y1, y2));
	float maxv = min(max(x1, x2), max(y1, y2));
	if(minv > maxv) return false;
	cfloat z1 = (minz - L) / O;
	cfloat z2 = (maxz - L) / O;
	minv = max(minv, min(z1, z2));
	maxv = min(maxv, max(z1, z2));
	if(minv > maxv) return false;
	return true;
}
