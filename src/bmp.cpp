/** bmp.cpp **/

#include "global.hpp"
#include <iostream>
#include <fstream>

//Returns data in RGB format starting from top left corner of the image
uchar *load_source_image() {
	cuchar width = 192;
	cuchar height = 128;
	FILE *file = fopen("malli.bmp", "rb");
	if(file == NULL) {
		std::cout << "Couldn't open malli.bmp!" << std::endl;
		return NULL;
	}
	for(uchar i=0;i<54;i++) getc(file);
	uchar *pixels = new uchar[width * height];
	for(uchar i=0;i<height;i++) {
		for(uchar j=0;j<width;j++) {
			pixels[/*(height -*/ i /*- 1)*/ * width + j] = getc(file);
			getc(file); getc(file);
		}
	}
	fclose(file);
	return pixels;
}

//Returns data in RGB format starting from top left corner of the image
//Supports 24 and 32 bit bmp:s without alpha
uchar *load_bmp(cchar *path, uint &width, uint &height) {
	width = 0;
	height = 0;
	FILE *file = fopen(path, "rb");
	if(file == NULL) {
		std::cout << "Couldn't open file " << path << "!" << std::endl;
		return NULL;
	}
	for(uchar i=0;i<18;i++) getc(file);
	for(uchar i=0;i<4;i++) width+= getc(file) << (i * 8);
	for(uchar i=0;i<4;i++) height+= getc(file) << (i * 8);
	getc(file); getc(file);
	uchar bpp = getc(file); //Bits per pixel
	if(bpp != 24 && bpp != 32) {
		std::cout << "Bitmap format " << (int)bpp << " bits per pixel not supported!" << std::endl;
		return NULL;
	}
	for(uchar i=0;i<25;i++) getc(file);
	uchar *pixels = new uchar[width * height * 3];
	for(uint i=0;i<height;i++) {
		for(uint j=0;j<width;j++) {
			pixels[((height - i - 1) * width + j) * 3 + 2] = getc(file);
			pixels[((height - i - 1) * width + j) * 3 + 1] = getc(file);
			pixels[((height - i - 1) * width + j) * 3] = getc(file);
			if(bpp == 32) getc(file);
		}
		if(bpp == 24 && width % 4 != 0) {
			 for(uchar i=0;i<4-width*3%4;i++) getc(file);
		}
	}
	fclose(file);
	return pixels;
}

inline void put_special(std::ofstream &file, cuint value) {
	for(uchar i=0;i<4;i++) file.put((value & (255 << (i * 8))) >> (i * 8));
}

void save_bmp(cuchar *data, cushort width, cushort height) {
	std::ifstream temp_file;
	std::ofstream file("teos.bmp", std::ios::binary);
	if(!file.good()) {
		std::cout << "Couldn't create teos.bmp!" << std::endl;
		return;
	}
	cuchar padding = width % 4;
	file.put(66); //B
	file.put(77); //M
	put_special(file, width * height * 3 + padding * height + 54); //size of the file
	for(uchar i=0;i<4;i++) file.put(0);
	file.put(54); //offset to image data
	for(uchar i=0;i<3;i++) file.put(0);
	file.put(40); //size of this header
	for(uchar i=0;i<3;i++) file.put(0);
	put_special(file, width); //width of the bitmap
	put_special(file, height); //height of the bitmap
	file.put(1);
	file.put(0);
	file.put(24); //bits per pixel
	for(uchar i=0;i<5;i++) file.put(0);
	put_special(file, width * height * 3); //size of the pixel data
	for(uchar i=0;i<16;i++) file.put(0);
	for(ushort i=0;i<height;i++) {
		for(ushort j=0;j<width;j++) {
			file.put(data[(i * width + j) * 3 + 2]);
			file.put(data[(i * width + j) * 3 + 1]);
			file.put(data[(i * width + j) * 3]);
		}
		for(uchar j=0;j<padding;j++) file.put(0);
	}
	file.close();
}
