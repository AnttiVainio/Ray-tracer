/** bmp.hpp **/

#ifndef BMP_HPP
#define BMP_HPP

#include "global.hpp"

uchar *load_source_image();
uchar *load_bmp(cchar *path, uint &width, uint &height);
void save_bmp(cuchar *data, cushort width = 192, cushort height = 128);

#endif
