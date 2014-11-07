/** main.cpp **/

/*

	This program participated in a programming contest in which you were supposed to create a 'drawing program'.
	The program was supposed to draw an image based on a black and white silhouette image stored in 'malli.bmp' bitmap.
	The resulting image would be saved into 'teos.bmp'.
	This version of this program differs from the one that participated in the contest by:
		-not saving the resulting image into a bitmap of the same size as the silhuette image as was required.
		-outputting text to the user and opening the resulting image after it is saved.
		-loading the images used by this program from separate bitmap files instead of a single data file.

	This program is a ray tracer that uses given image as a heightmap to render a mountain scenery.
	The scene has a single directional light for which accurate shadows are determined.
	The scene is lit using diffuse and specular reflections of which the specular reflection uses phong shading.
	The texturing of the polygons uses parallax maps and normal maps.
	The final image is post processed using simple antialiasing, depth of field and bloom.
	The final image is rendered in higher resolution before it is scaled down to the required resolution (this also works as antialiasing).

	Calculations for a ray colliding with a polygon is located in cast_ray.cpp
	There is a nice bmp saving function in bmp.cpp
	The program is quite optimized as tracing rays is slow altough it could be even more optimized

*/

#include "global.hpp"
#include "bmp.hpp"
#include "polygon.hpp"
#include "cube.hpp"
#include "cast_ray.hpp"
#include "math.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

#define OUTPUT //Defines wether program output is allowed
#define INPUT //Defines wether user input is allowed

//Note that the bloom and depth of field blurriness are affected if the size of the rendered image is changed
//FINAL_X and FINAL_Y are not the size of the image that is saved but the size of the rendered image before it is scaled down
#define FINAL_X 600
#define FINAL_Y 400
#define FINAL_SCALE_DOWN 1 //This is the factor used to scale down the image after it has been fully rendered

//This will tell the OS to open the image after it is saved
#define OPEN_IMAGE

#ifdef OPEN_IMAGE
	#ifdef _WIN32
		#include <windows.h>
	#endif
#endif

void open_image() {
	#ifdef OPEN_IMAGE
		#ifdef _WIN32
			ShellExecute(NULL, "open", "teos.bmp", NULL, NULL, SW_SHOWNORMAL);
		#else
			system("xdg-open teos.bmp"); //Attempts to open the image on Linux but may not work on all distros
		#endif
	#endif
}

//The program will only process the source image and show that before it is used to create the actual work
//#define SHOW_SOURCE

int main() {
	#ifdef OUTPUT
		std::cout << "Loading the source image and bitmaps" << std::endl;
	#endif
	uchar *final = new uchar[FINAL_X * FINAL_Y * 3 / FINAL_SCALE_DOWN / FINAL_SCALE_DOWN];
	float *depth_buffer = new float[FINAL_X * FINAL_Y];
	uchar *source = load_source_image();
	uint temp_width, temp_height;
	uchar *texture1 = load_bmp("img/rock.bmp", temp_width, temp_height); //Rock texture
	uchar *texture2 = load_bmp("img/snow.bmp", temp_width, temp_height); //Snow texture
	uchar *texture3 = new uchar[256 * 256]; //Texture mixing mask
	uchar *texture4 = new uchar[256 * 256]; //Rock parallax map
	uchar *texture5 = new uchar[256 * 256]; //Snow parallax map
	uchar *texture6 = load_bmp("img/rock_normal.bmp", temp_width, temp_height); //Rock normalmap
	uchar *texture7 = load_bmp("img/snow_normal.bmp", temp_width, temp_height); //Snow normalmap
	uchar *temp = load_bmp("img/mix_mask.bmp", temp_width, temp_height);
	for(int i=0;i<256*256;i++) texture3[i] = temp[i * 3];
	delete [] temp;
	temp = load_bmp("img/rock_parallax.bmp", temp_width, temp_height);
	for(int i=0;i<256*256;i++) texture4[i] = temp[i * 3];
	delete [] temp;
	temp = load_bmp("img/snow_parallax.bmp", temp_width, temp_height);
	for(int i=0;i<256*256;i++) texture5[i] = temp[i * 3];
	delete [] temp;

		/** Blur the source image **/
	#ifdef OUTPUT
		std::cout << "Blurring the source image" << std::endl;
	#endif
	uchar *temp_source = new uchar[192 * 128];
	for(uchar i=0;i<192;i++) {
		for(uchar j=0;j<128;j++) {
			float sum = 0;
			float div = 0;
			for(char k=-10;k<=10;k++) {
				cuchar x = clampi(i + k, 0, 191);
				cfloat mult1 = abs(k) + 4;
				cfloat mult = 1.0 / mult1 / mult1;
				sum+= (float)source[j * 192 + x] * mult;
				div+= mult;
			}
			temp_source[j * 192 + i] = uchar(sum / div);
		}
	}
	for(uchar i=0;i<192;i++) {
		for(uchar j=0;j<128;j++) {
			float sum = 0;
			float div = 0;
			for(char k=-10;k<=10;k++) {
				cuchar y = clampi(j + k, 0, 127);
				cfloat mult1 = abs(k) + 4;
				cfloat mult = 1.0 / mult1 / mult1;
				sum+= (float)temp_source[y * 192 + i] * mult;
				div+= mult;
			}
			source[j * 192 + i] = uchar(sum / div);
		}
	}
	delete [] temp_source;

	#ifndef SHOW_SOURCE
	#define ACC 1 //This is the accuracy of the scene; bigger values are less accurate; valid values are 1, 2, 4, 8, 16, 32 and 64
	#define CUBE_AMOUNT 5 //This can't be 0; comment out if 0 is wanted
		/** Scale down the source image **/
	//This could have been done in a single for loop instead of separate loops for x and y axes without speed loss
	#ifdef OUTPUT
		std::cout << "Scaling down the source image by " << ACC << std::endl;
	#endif
	temp_source = new uchar[192 * 128 / ACC];
	for(uchar i=0;i<192;i+=ACC) {
		for(uchar j=0;j<128;j++) {
			float sum = 0;
			for(uchar k=0;k<ACC;k++) sum+= source[j * 192 + i + k];
			temp_source[j * 192 / ACC + i / ACC] = uchar(sum / (float)ACC);
		}
	}
	delete [] source;
	source = new uchar[192 * 128 / ACC / ACC];
	for(uchar i=0;i<192/ACC;i++) {
		for(uchar j=0;j<128;j+=ACC) {
			float sum = 0;
			for(uchar k=0;k<ACC;k++) sum+= temp_source[(j + k) * 192 / ACC + i];
			source[j * 192 / ACC / ACC + i] = uchar(sum / (float)ACC);
		}
	}
	delete [] temp_source;

		/** Create polygons **/
	#ifdef OUTPUT
		std::cout << "Creating polygons" << std::endl;
	#endif
	std::vector<polygon_c> polygons;
	for(uchar i=0;i<192/ACC-1;i++) {
		for(uchar j=0;j<128/ACC-1;j++) {
			cuchar h1 = (255 - source[(128 / ACC - 1 - j) * 192 / ACC + i]) / 8;
			cuchar h2 = (255 - source[(128 / ACC - 1 - j) * 192 / ACC + i + 1]) / 8;
			cuchar h3 = (255 - source[(128 / ACC - 1 - j - 1) * 192 / ACC + i]) / 8;
			cuchar h4 = (255 - source[(128 / ACC - 1 - j - 1) * 192 / ACC + i + 1]) / 8;
			polygons.push_back(polygon_c(i * ACC, h1, j * ACC, (i + 1) * ACC, h2, j * ACC, i * ACC, h3, (j + 1) * ACC, 0, 0, 1, 0, 0, 1));
			polygons.push_back(polygon_c((i + 1) * ACC, h2, j * ACC, (i + 1) * ACC, h4, (j + 1) * ACC, i * ACC, h3, (j + 1) * ACC, 1, 0, 1, 1, 0, 1));
		}
	}
	#define BORDER_LENGTH 50
	#define BORDER_HEIGHT 8
	//Creating edge polygons
	for(uchar i=0;i<192/ACC-1;i++) {
		cuchar h1 = (255 - source[(128 / ACC - 1) * 192 / ACC + i]) / 8;
		cuchar h2 = (255 - source[(128 / ACC - 1) * 192 / ACC + i + 1]) / 8;
		cuchar h3 = (255 - source[i]) / 8;
		cuchar h4 = (255 - source[i + 1]) / 8;
		polygons.push_back(polygon_c(i * ACC, h1, 0, i * ACC, BORDER_HEIGHT, -BORDER_LENGTH, (i + 1) * ACC, h2, 0, 0, 1, 0, 0, 1, 1));
		polygons.push_back(polygon_c((i + 1) * ACC, h2, 0, i * ACC, BORDER_HEIGHT, -BORDER_LENGTH, (i + 1) * ACC, BORDER_HEIGHT, -BORDER_LENGTH, 1, 1, 0, 0, 1, 0));
		polygons.push_back(polygon_c(i * ACC, BORDER_HEIGHT, 128 - ACC + BORDER_LENGTH, i * ACC, h3, 128 - ACC, (i + 1) * ACC, BORDER_HEIGHT, 128 - ACC + BORDER_LENGTH, 0, 1, 0, 0, 1, 1));
		polygons.push_back(polygon_c((i + 1) * ACC, BORDER_HEIGHT, 128 - ACC + BORDER_LENGTH, i * ACC, h3, 128 - ACC, (i + 1) * ACC, h4, 128 - ACC, 1, 1, 0, 0, 1, 0));
	}
	for(uchar j=0;j<128/ACC-1;j++) {
		cuchar h1 = (255 - source[(128 / ACC - 1 - j) * 192 / ACC]) / 8;
		cuchar h2 = (255 - source[(128 / ACC - 1 - j) * 192 / ACC + 192 / ACC - 1]) / 8;
		cuchar h3 = (255 - source[(128 / ACC - 1 - j - 1) * 192 / ACC]) / 8;
		cuchar h4 = (255 - source[(128 / ACC - 1 - j - 1) * 192 / ACC + 192 / ACC - 1]) / 8;
		polygons.push_back(polygon_c(0, h1, j * ACC, 0, h3, (j + 1) * ACC, -BORDER_LENGTH, BORDER_HEIGHT, j * ACC, 1, 0, 1, 1, 0, 0));
		polygons.push_back(polygon_c(-BORDER_LENGTH, BORDER_HEIGHT, j * ACC, 0, h3, (j + 1) * ACC, -BORDER_LENGTH, BORDER_HEIGHT, (j + 1) * ACC, 0, 0, 1, 1, 0, 1));
		polygons.push_back(polygon_c(192 - ACC + BORDER_LENGTH, BORDER_HEIGHT, j * ACC, 192 - ACC + BORDER_LENGTH, BORDER_HEIGHT, (j + 1) * ACC, 192 - ACC, h2, j * ACC, 1, 0, 1, 1, 0, 0));
		polygons.push_back(polygon_c(192 - ACC, h2, j * ACC, 192 - ACC + BORDER_LENGTH, BORDER_HEIGHT, (j + 1) * ACC, 192 - ACC, h4, (j + 1) * ACC, 0, 0, 1, 1, 0, 1));
	}
	//Creating corner polygons
	cuchar h1 = (255 - source[(128 / ACC - 1) * 192 / ACC]) / 8;
	cuchar h2 = (255 - source[(128 / ACC - 1) * 192 / ACC + 192 / ACC - 1]) / 8;
	cuchar h3 = (255 - source[0]) / 8;
	cuchar h4 = (255 - source[192 / ACC - 1]) / 8;
	polygons.push_back(polygon_c(0, h1, 0, -BORDER_LENGTH, BORDER_HEIGHT, 0, 0, BORDER_HEIGHT, -BORDER_LENGTH, 1, 1, 0, 1, 1, 0));
	polygons.push_back(polygon_c(-BORDER_LENGTH, BORDER_HEIGHT, 0, -BORDER_LENGTH, BORDER_HEIGHT, -BORDER_LENGTH, 0, BORDER_HEIGHT, -BORDER_LENGTH, 0, 1, 0, 0, 1, 0));
	polygons.push_back(polygon_c(192 - ACC + BORDER_LENGTH, BORDER_HEIGHT, 0, 192 - ACC, h2, 0, 192 - ACC + BORDER_LENGTH, BORDER_HEIGHT, -BORDER_LENGTH, 1, 1, 0, 1, 1, 0));
	polygons.push_back(polygon_c(192 - ACC, h2, 0, 192 - ACC, BORDER_HEIGHT, -BORDER_LENGTH, 192 - ACC + BORDER_LENGTH, BORDER_HEIGHT, -BORDER_LENGTH, 0, 1, 0, 0, 1, 0));
	polygons.push_back(polygon_c(0, BORDER_HEIGHT, 128 - ACC + BORDER_LENGTH, -BORDER_LENGTH, BORDER_HEIGHT, 128 - ACC + BORDER_LENGTH, 0, h3, 128 - ACC, 1, 1, 0, 1, 1, 0));
	polygons.push_back(polygon_c(-BORDER_LENGTH, BORDER_HEIGHT, 128 - ACC + BORDER_LENGTH, -BORDER_LENGTH, BORDER_HEIGHT, 128 - ACC, 0, h3, 128 - ACC, 0, 1, 0, 0, 1, 0));
	polygons.push_back(polygon_c(192 - ACC + BORDER_LENGTH, BORDER_HEIGHT, 128 - ACC + BORDER_LENGTH, 192 - ACC, BORDER_HEIGHT, 128 - ACC + BORDER_LENGTH, 192 - ACC + BORDER_LENGTH, BORDER_HEIGHT, 128 - ACC, 1, 1, 0, 1, 1, 0));
	polygons.push_back(polygon_c(192 - ACC, BORDER_HEIGHT, 128 - ACC + BORDER_LENGTH, 192 - ACC, h4, 128 - ACC, 192 - ACC + BORDER_LENGTH, BORDER_HEIGHT, 128 - ACC, 0, 1, 0, 0, 1, 0));
	#ifdef OUTPUT
		std::cout << "     Created " << polygons.size() << " polygons" << std::endl;
	#endif
		//all polygons created; creating acceleration structure with cubes (actually cuboids)
	#ifdef CUBE_AMOUNT
		std::vector<std::vector<cube_c> > cubes;
		for(uchar i=0;i<CUBE_AMOUNT;i++) cubes.push_back(std::vector<cube_c>());
		if(polygons.size() >= 4) {
			for(uint i=0;i<polygons.size()-3;i+=4) cubes.at(0).push_back(cube_c(polygons, i, i + 1, i + 2, i + 3));
		}
		cubes.at(0).push_back(cube_c());
		#ifdef OUTPUT
			std::cout << "     Created " << cubes.at(0).size() << " level 1 cubes" << std::endl;
		#endif
		for(uchar i=1;i<CUBE_AMOUNT;i++) {
			if(cubes.at(i - 1).size() >= 5) {
				//Using size()-4 here instead of size()-3 always to ignore the last cube which is empty
				for(uint j=0;j<cubes.at(i-1).size()-4;j+=4) cubes.at(i).push_back(cube_c(cubes.at(i - 1), j, j + 1, j + 2, j + 3));
			}
			cubes.at(i).push_back(cube_c());
			#ifdef OUTPUT
				std::cout << "     Created " << cubes.at(i).size() << " level " << int(i + 1) << " cubes" << std::endl;
			#endif
		}
	#endif

	//Data for more accurate color calculations and high dynamic range colors
	float *image = new float[FINAL_X * FINAL_Y * 3];

	//Direction for sun lighting
	float sunx = 15;
	float suny = -7;
	float sunz = -5;
	cfloat sunl = sqrt(sunx * sunx + suny * suny + sunz * sunz);
	sunx/= sunl;
	suny/= sunl;
	sunz/= sunl;

	#define CAMERA_X 128.0
	#define CAMERA_Y 128.0
	#define CAMERA_Z 192.0

		/** Trace rays and do all the rendering stuff **/
	//This thing shoots a ray from the viewer for every pixel in the image
	//After the position in which the ray hits a polygon has been determined:
	//	A ray is shot towards the direction of the sun lighting to check if other polygon occludes the sun
	//	Another ray is reflected by the surface normal, altered by normalmap, for phong shading
	//The texture coordinate is determined by the hit position and parallax mapping
	#ifdef OUTPUT
		std::cout << "Tracing rays" << std::endl;
	#endif
	float a, b, c, x, y, z;
	char progress = 0;
	for(ushort i=0;i<FINAL_X;i++) {
		cfloat target_x = 0.85 * (float)i / (float)FINAL_X * 192.0 + 16.0;
		for(ushort j=0;j<FINAL_Y;j++) {
			float best = 1000;
			float hitx = 0, hity = 0, hitz = 0;
			ushort hitpolygon = 0;
			for(uint k=0;k<polygons.size();k++) { //polygons
				bool cast = true;
				//Check cube collisions to skip polygons
				#ifdef CUBE_AMOUNT
					for(char l=CUBE_AMOUNT-1;l>=0;l--) {
						cuint div = uint(4) << (l * 2);
						if(k % div == 0) {
							if(!cubes.at(l).at(k / div).test_hit(CAMERA_X, CAMERA_Y, CAMERA_Z, target_x, (float)j / (float)FINAL_Y * 128.0 - 64.0, 80)) {
								k+= div - 1;
								cast = false;
								l = -1;
							}
						}
					}
				#endif
				//Now check the polygon collision
				if(cast) {
					if(cast_ray(polygons.at(k), a, b, c, x, y, z, CAMERA_X, CAMERA_Y, CAMERA_Z, target_x, (float)j / (float)FINAL_Y * 128.0 - 64.0, 80)) {
						if(a >= 0 && b >= 0 && a + b <= 1 && c < best && c > 0) { //A polygon, that is closer to the camera than any other polygon so far, is hitting the ray
							best = c;
							hitx = x; hity = y; hitz = z; hitpolygon = k;
						}
					}
				}
			}
			cuint id = (j * FINAL_X + i) * 3;
			if(best < 999) { //The ray actually hits a polygon
				//Calculate shadow
				bool shadow = false;
				for(uint k=0;k<polygons.size();k++) { //polygons
					bool cast = true;
					#ifdef CUBE_AMOUNT
						for(char l=CUBE_AMOUNT-1;l>=0;l--) {
							cuint div = uint(4) << (l * 2);
							if(k % div == 0) {
								if(!cubes.at(l).at(k / div).test_hit(hitx, hity + 0.01, hitz, hitx - sunx, hity - suny + 0.01, hitz - sunz)) {
									k+= div - 1;
									cast = false;
									l = -1;
								}
							}
						}
					#endif
					if(cast) {
						if(cast_ray(polygons.at(k), a, b, c, x, y, z, hitx, hity + 0.01, hitz, hitx - sunx, hity - suny + 0.01, hitz - sunz)) {
							if(a >= 0 && b >= 0 && a + b <= 1 && c > 0 && k != hitpolygon) {
								shadow = true;
								k = polygons.size();
							}
						}
					}
				}
					//Lighting defines
				#define AMBIENT
				#define DIFFUSE
				#define PHONG
					//Map defines
				#define PARALLAX
				#define NORMAL
					//normal vector
				cfloat nx = polygons.at(hitpolygon).nx;
				cfloat ny = polygons.at(hitpolygon).ny;
				cfloat nz = polygons.at(hitpolygon).nz;
					//tangent vector
				cfloat tx = polygons.at(hitpolygon).tx;
				cfloat ty = polygons.at(hitpolygon).ty;
				cfloat tz = polygons.at(hitpolygon).tz;
					//binormal vector
				cfloat bx = polygons.at(hitpolygon).bx;
				cfloat by = polygons.at(hitpolygon).by;
				cfloat bz = polygons.at(hitpolygon).bz;
					//camera vector
				float cx = hitx - CAMERA_X;
				float cy = hity - CAMERA_Y;
				float cz = hitz - CAMERA_Z;
				cfloat cl = sqrt(cx * cx + cy * cy + cz * cz);
				cx/= cl;
				cy/= cl;
				cz/= cl;
				depth_buffer[id / 3] = cl;
					//camera vector in tangent space
				cfloat tscx = tx * cx + bx * cy + nx * cz;
				cfloat tscy = ty * cx + by * cy + ny * cz;
				cfloat tscz = tz * cx + bz * cy + nz * cz;
					//texture position
				short tex_idx = (short)mix(0, 191, 0, 255, hitx);
				short tex_idy = (short)mix(0, 127, 0, 255, hitz);
				while(tex_idx > 255) tex_idx-= 256;
				while(tex_idx < 0) tex_idx+= 256;
				while(tex_idy > 255) tex_idy-= 256;
				while(tex_idy < 0) tex_idy+= 256;
				cuint tex_id = tex_idy * 256 + tex_idx;
				const bool tex = hity < mix(0, 255, 4, 19, texture3[tex_id]);
				cuchar *texture = tex ? texture1 : texture2;
				#ifdef PARALLAX
					//Parallax offset
					cfloat mult = 0.2 * (float(tex ? texture4[tex_id] : texture5[tex_id]) - 128.0);
					tex_idx+= mult * tscx;
					tex_idy+= mult * tscy;
					while(tex_idx > 255) tex_idx-= 256;
					while(tex_idx < 0) tex_idx+= 256;
					while(tex_idy > 255) tex_idy-= 256;
					while(tex_idy < 0) tex_idy+= 256;
				#endif
				cuint tex_id2 = (tex_idy * 256 + tex_idx) * 3;
				cfloat texr = texture[tex_id2];
				cfloat texg = texture[tex_id2 + 1];
				cfloat texb = texture[tex_id2 + 2];
					//normalmap
				#ifdef NORMAL
					float nmx = (float(tex ? texture6[tex_id2] : texture7[tex_id2]) / 255.0 - 0.5) * 2.0;
					float nmy = (float(tex ? texture6[tex_id2 + 1] : texture7[tex_id2 + 1]) / 255.0 - 0.5) * 2.0;
					float nmz = (float(tex ? texture6[tex_id2 + 2] : texture7[tex_id2 + 2]) / 255.0 - 0.5) * 2.0 * 0.3;
					cfloat nml = sqrt(nmx * nmx + nmy * nmy + nmz * nmz);
					nmx/= nml;
					nmy/= nml;
					nmz/= nml;
				#else
					cfloat nmx = 0;
					cfloat nmy = 0;
					cfloat nmz = 1;
				#endif
				//Add ambient lighting
				#ifdef AMBIENT
					cfloat brightness = nmx * -bx + nmy * -by + nmz * -bz; //equals vector {0, -1, 0}
					image[id] = texr * brightness * 0.3;
					image[id + 1] = texg * brightness * 0.4;
					image[id + 2] = texb * brightness * 0.5;
				#else
					image[id] = 0;
					image[id + 1] = 0;
					image[id + 2] = 0;
				#endif
				if(!shadow) {
						//sun vector in tangent space
					cfloat tssunx = tx * sunx + bx * suny + nx * sunz;
					cfloat tssuny = ty * sunx + by * suny + ny * sunz;
					cfloat tssunz = tz * sunx + bz * suny + nz * sunz;
					//Calculate sun lighting
					#ifdef DIFFUSE
						cfloat dbrightness = max(nmx * tssunx + nmy * tssuny + nmz * tssunz, 0.0) * 1.5;
						image[id]+= texr * dbrightness * 1.0;
						image[id + 1]+= texg * dbrightness * 0.6;
						image[id + 2]+= texb * dbrightness * 0.5;
					#endif
					//Calculate phong lighting
					#ifdef PHONG
						cfloat dot = nmx * tscx + nmy * tscy + nmz * tscz * 2.0;
							//phong vector
						cfloat px = tscx - dot * nmx;
						cfloat py = tscy - dot * nmy;
						cfloat pz = tscz - dot * nmz;
						cfloat pbrightness = pow(max(px * -tssunx + py * -tssuny + pz * -tssunz, 0.0), 10.0) * 5.0;
						image[id]+= texr * pbrightness * 1.0;
						image[id + 1]+= texg * pbrightness * 0.6;
						image[id + 2]+= texb * pbrightness * 0.5;
					#endif
				}
			}
			else {
				image[id] = 255;
				image[id + 1] = uchar(mix(0, FINAL_Y, 0, 255, j));
				image[id + 2] = uchar(mix(0, FINAL_Y, 0, 128, j));
				depth_buffer[id / 3] = 1000000;
			}
		}
		if(i * 10 / (FINAL_X - 1) > progress) {
			progress++;
			#ifdef OUTPUT
				std::cout.width(2);
				std::cout << std::right << (int)progress << " / 10" << std::endl;
			#endif
		}
	}

	//Fix depth buffer
	//Sometimes there are seams in between the polygons where the ray doesn't hit any polygons which causes single deep spots in the depth buffer
	//This removes those spots in the depth buffer for better result in depth of field calculation
	for(ushort i=0;i<FINAL_X;i++) {
		for(ushort j=0;j<FINAL_Y;j++) {
			if(depth_buffer[j * FINAL_X + i] > 999999) {
				uchar sum1 = 0;
				float sum = 0;
				float div = 0;
				for(char k=-1;k<=1;k++) {
					for(char l=-1;l<=1;l++) {
						cushort x = clampi(i + k, 0, FINAL_X - 1);
						cushort y = clampi(j + l, 0, FINAL_Y - 1);
						if(depth_buffer[y * FINAL_X + x] < 999999) {
							sum1++;
							sum+= depth_buffer[y * FINAL_X + x];
							div++;
						}
					}
				}
				if(sum1 >= 7) depth_buffer[j * FINAL_X + i] = sum / div;
			}
		}
	}

		//Post processing defines
	#define ANTIALIASING
	#define DOF
	#define BLOOM

	float *temp_image = new float[FINAL_X * FINAL_Y * 3];

		/** Antialiasing **/
	#ifdef ANTIALIASING
	//This is a pretty cheap way of antialiasing that basically blurs the image a bit
	#ifdef OUTPUT
		std::cout << "Applying antialiasing" << std::endl;
	#endif
	cfloat mult1 = sqrt(32.0);
	cfloat mult2 = sqrt(1.6);
	cfloat mult3 = 2.0 / sqrt(18.0);
	for(ushort i=0;i<FINAL_X;i++) {
		for(ushort j=0;j<FINAL_Y;j++) {
			float sum_r = 0, sum_g = 0, sum_b = 0;
			float div = 0;
			for(char k=-1;k<=1;k++) {
				for(char l=-1;l<=1;l++) {
					cushort x = clampi(i + k, 0, FINAL_X - 1);
					cushort y = clampi(j + l, 0, FINAL_Y - 1);
					cchar dist = abs(k) + abs(l);
					cfloat mult = dist ? (dist == 1 ? mult2 : mult3) : mult1;
					sum_r+= image[(y * FINAL_X + x) * 3] * mult;
					sum_g+= image[(y * FINAL_X + x) * 3 + 1] * mult;
					sum_b+= image[(y * FINAL_X + x) * 3 + 2] * mult;
					div+= mult;
				}
			}
			temp_image[(j * FINAL_X + i) * 3] = sum_r / div;
			temp_image[(j * FINAL_X + i) * 3 + 1] = sum_g / div;
			temp_image[(j * FINAL_X + i) * 3 + 2] = sum_b / div;
		}
	}
	for(uint i=0;i<FINAL_X*FINAL_Y*3;i++) image[i] = temp_image[i];
	#endif

		/** Depth of field **/
	#ifdef DOF
	#ifdef OUTPUT
		std::cout << "Applying depth of field" << std::endl;
	#endif
	#define DOF_START 160.0
	#define DOF_END 500.0
	#define DOF_AMOUNT 2.5
	#define DOF_MAX 4.0
	#define DOF_ACC 15
	float *sum_r1 = new float[FINAL_X * FINAL_Y];
	float *sum_g1 = new float[FINAL_X * FINAL_Y];
	float *sum_b1 = new float[FINAL_X * FINAL_Y];
	float *div1 = new float[FINAL_X * FINAL_Y];
	for(ushort i=0;i<FINAL_X;i++) {
		for(ushort j=0;j<FINAL_Y;j++) {
			cfloat depth = depth_buffer[j * FINAL_X + i];
			cuint id1 = j * FINAL_X + i;
			sum_r1[id1] = 0;
			sum_g1[id1] = 0;
			sum_b1[id1] = 0;
			div1[id1] = 0;
			if(depth > DOF_START) {
				for(char k=-DOF_ACC;k<=DOF_ACC;k++) {
					cushort x = clampi(i + k, 0, FINAL_X - 1);
					cfloat depth2 = depth_buffer[j * FINAL_X + x];
					if(depth2 > DOF_START) {
						cfloat dof_amount = clampf(mix(DOF_START, DOF_END, 0, DOF_AMOUNT, depth2), 0, DOF_MAX);
						cfloat mult = 1.0 / (fabs((float)k / dof_amount) + 1.0);
						cuint id = (j * FINAL_X + x) * 3;
						sum_r1[id1]+= image[id] * mult;
						sum_g1[id1]+= image[id + 1] * mult;
						sum_b1[id1]+= image[id + 2] * mult;
						div1[id1]+= mult;
					}
				}
			}
		}
	}
	for(ushort i=0;i<FINAL_X;i++) {
		for(ushort j=0;j<FINAL_Y;j++) {
			cfloat depth = depth_buffer[j * FINAL_X + i];
			if(depth > DOF_START) {
				float sum_r = 0;
				float sum_g = 0;
				float sum_b = 0;
				float div = 0;
				for(char k=-DOF_ACC;k<=DOF_ACC;k++) {
					cushort y = clampi(j + k, 0, FINAL_Y - 1);
					cfloat depth2 = depth_buffer[y * FINAL_X + i];
					if(depth2 > DOF_START) {
						cfloat dof_amount = clampf(mix(DOF_START, DOF_END, 0, DOF_AMOUNT, depth2), 0, DOF_MAX);
						cfloat mult = 1.0 / (fabs((float)k / dof_amount) + 1.0);
						cuint id = y * FINAL_X + i;
						sum_r+= sum_r1[id] * mult;
						sum_g+= sum_g1[id] * mult;
						sum_b+= sum_b1[id] * mult;
						div+= div1[id] * mult;
					}
				}
				cuint id = (j * FINAL_X + i) * 3;
				temp_image[id] = sum_r / div;
				temp_image[id + 1] = sum_g / div;
				temp_image[id + 2] = sum_b / div;
			}
			else {
				cuint id = (j * FINAL_X + i) * 3;
				temp_image[id] = image[id];
				temp_image[id + 1] = image[id + 1];
				temp_image[id + 2] = image[id + 2];
			}
		}
	}
	delete [] sum_r1;
	delete [] sum_g1;
	delete [] sum_b1;
	delete [] div1;
	//Apply dof
	for(uint i=0;i<FINAL_X*FINAL_Y*3;i++) image[i] = temp_image[i];
	#endif

		/** Bloom **/
	#ifdef BLOOM
	//Bloom makes everything look better, always.
	#ifdef OUTPUT
		std::cout << "Applying bloom" << std::endl;
	#endif
	#define BLOOM_SIZE 12.0
	#define CONTRAST_AMOUNT 1.4
	#define BLOOM_AMOUNT 0.5
	#define DARKNESS 70.0
	//Copy the image into temp_image for bloom operations; also add contrast
	for(uint i=0;i<FINAL_X*FINAL_Y*3;i++) temp_image[i] = (image[i] - 0.5) * CONTRAST_AMOUNT + 0.5;
	float *temp_image2 = new float[FINAL_X * FINAL_Y * 3];
	//Blur
	for(ushort i=0;i<FINAL_X;i++) {
		for(ushort j=0;j<FINAL_Y;j++) {
			float sum_r = 0;
			float sum_g = 0;
			float sum_b = 0;
			float div = 0;
			for(char k=-(char)ceil(BLOOM_SIZE);k<(char)ceil(BLOOM_SIZE);k++) {
				cushort x = clampi(i + k, 0, FINAL_X - 1);
				cfloat mult = 1.0 / (fabs((float)k / BLOOM_SIZE) + 1.0);
				cuint id = (j * FINAL_X + x) * 3;
				sum_r+= temp_image[id] * mult;
				sum_g+= temp_image[id + 1] * mult;
				sum_b+= temp_image[id + 2] * mult;
				div+= mult;
			}
			cuint id = (j * FINAL_X + i) * 3;
			temp_image2[id] = sum_r / div;
			temp_image2[id + 1] = sum_g / div;
			temp_image2[id + 2] = sum_b / div;
		}
	}
	for(ushort i=0;i<FINAL_X;i++) {
		for(ushort j=0;j<FINAL_Y;j++) {
			float sum_r = 0;
			float sum_g = 0;
			float sum_b = 0;
			float div = 0;
			for(char k=-(char)ceil(BLOOM_SIZE);k<(char)ceil(BLOOM_SIZE);k++) {
				cushort y = clampi(j + k, 0, FINAL_Y - 1);
				cfloat mult = 1.0 / (fabs((float)k / BLOOM_SIZE) + 1.0);
				cuint id = (y * FINAL_X + i) * 3;
				sum_r+= temp_image2[id] * mult;
				sum_g+= temp_image2[id + 1] * mult;
				sum_b+= temp_image2[id + 2] * mult;
				div+= mult;
			}
			cuint id = (j * FINAL_X + i) * 3;
			temp_image[id] = sum_r / div;
			temp_image[id + 1] = sum_g / div;
			temp_image[id + 2] = sum_b / div;
		}
	}
	//Apply bloom
	for(uint i=0;i<FINAL_X*FINAL_Y*3;i++) image[i] = image[i] + temp_image[i] * BLOOM_AMOUNT - DARKNESS;
	#endif

		/** Scale down the high precision image into low precision image **/
	//This also works as a proper way of antialiasing
	#ifdef OUTPUT
		std::cout << "Saving the final image" << std::endl;
	#endif
	for(ushort i=0;i<FINAL_X/FINAL_SCALE_DOWN;i++) {
		for(ushort j=0;j<FINAL_Y/FINAL_SCALE_DOWN;j++) {
			float sum_r = 0;
			float sum_g = 0;
			float sum_b = 0;
			for(uchar k=0;k<FINAL_SCALE_DOWN;k++) {
				for(uchar l=0;l<FINAL_SCALE_DOWN;l++) {
					cuint pos = ((j * FINAL_SCALE_DOWN + l) * FINAL_X + i * FINAL_SCALE_DOWN + k) * 3;
					sum_r+= image[pos];
					sum_g+= image[pos + 1];
					sum_b+= image[pos + 2];
				}
			}
			cuint pos = (j * FINAL_X / FINAL_SCALE_DOWN + i) * 3;
			final[pos] = uchar(clampf(sum_r / float(FINAL_SCALE_DOWN * FINAL_SCALE_DOWN), 0, 255));
			final[pos + 1] = uchar(clampf(sum_g / float(FINAL_SCALE_DOWN * FINAL_SCALE_DOWN), 0, 255));
			final[pos + 2] = uchar(clampf(sum_b / float(FINAL_SCALE_DOWN * FINAL_SCALE_DOWN), 0, 255));
		}
	}
	delete [] temp_image;
	#ifdef BLOOM
	delete [] temp_image2;
	#endif
	delete [] image;
	save_bmp(final, FINAL_X / FINAL_SCALE_DOWN, FINAL_Y / FINAL_SCALE_DOWN);

	#else
	for(int i=0;i<192*128;i++) {
		final[i * 3] = source[i];
		final[i * 3 + 1] = source[i];
		final[i * 3 + 2] = source[i];
	}
	#ifdef OUTPUT
		std::cout << "Saving the source image" << std::endl;
	#endif
	save_bmp(final);
	#endif

	delete [] final;
	delete [] depth_buffer;
	delete [] source;
	delete [] texture1;
	delete [] texture2;
	delete [] texture3;
	delete [] texture4;
	delete [] texture5;
	delete [] texture6;
	delete [] texture7;
	open_image();
	#ifdef OUTPUT
		std::cout << "It's done. Result saved in teos.bmp" << std::endl;
		std::cout << "     Press enter to close this program" << std::endl;
	#endif
	#ifdef INPUT
		std::cin.ignore();
	#endif
	return 0;
}
