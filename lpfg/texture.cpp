/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#include <cstring>

#ifdef WIN32
#include <GL/glew.h>
#include <windows.h>
#include <GL/gl.h>
#else
#include <qgl.h>
#include "lodepng.h"
#include "lodepng_util.h"
#endif

#include <image/image.h>
#include <string>

#include "texture.h"
#include "exception.h"
#include "asrt.h"
#include "utils.h"

template <typename T> class auto_vector {
public:
  auto_vector(size_t n) {
    _arr = new T[n];
#ifndef NDEBUG
    _size = n;
#endif
  }
  auto_vector(T *arr) { _arr = arr; }
  ~auto_vector() { delete[] _arr; }
  T *get() { return _arr; }
  T get(size_t i) const {
    ASSERT(i < _size);
    return _arr[i];
  }
  void set(size_t i, T t) {
    ASSERT(i < _size);
    _arr[i] = t;
  }
  T *release() {
    T *a = _arr;
    _arr = 0;
    return a;
  }

private:
  T *_arr;
#ifndef NDEBUG
  size_t _size;
#endif
};

Texture::Texture(const char *fname) {
  strcpy(_fname, fname);
  _Xsize = _Ysize = _Zsize = -1;
  _id = tUninitialized;
}

bool Texture::Reread() {
  return Bind();
}


bool Texture::Bind() {
  const char *type = strrchr(_fname, '.');
  if (0 == type) {
    return false;
    throw Exception("Cannot determine format for the texture %s\n", _fname);
  }

  ++type;
  GLubyte *pData = 0;
  int count = 0;
  while ((pData == 0) && (count < 10000)) {
    count++;
    if (!strcmp(type, "rgb"))
      pData = _LoadRGB(_fname);
    else if (!strcmp(type, "png"))
      pData = _LoadPNG(_fname);
  }

  /*
  if (count >= 10000)
    std::cerr << "we can't reload the PNG file : " << count << std::endl;
  */
  auto_vector<GLubyte> data(pData);

  // Possible issue: deleting the texture and generating the id again
  // causes problems on some systems. It may be better to generate the
  // texture once if (_id == tUninitialized) and remove the delete.
  if (tUninitialized != _id)
    glDeleteTextures(1, &_id);

  glGenTextures(1, &_id);
  glBindTexture(GL_TEXTURE_2D, _id);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  GLenum fmt = GL_RGB;
  if (4 == _Zsize)
    fmt = GL_RGBA;
  glTexImage2D(GL_TEXTURE_2D, 0, _Zsize, _Xsize, _Ysize, 0, fmt,
               GL_UNSIGNED_BYTE, data.get());
  glEnable(GL_TEXTURE_2D);

 // why on earth were the following two lines here?!
 // glPopMatrix(); /* see above */
 // glMatrixMode(GL_MODELVIEW);

  glBindTexture(GL_TEXTURE_2D, 0);
  return true;
}

void Texture::OutputToPOVRay(std::ostream &stream) const {
  std::string tgaFileName = getFilename();
  tgaFileName.replace(tgaFileName.length() - 4, 4, ".tga");

  stream << "texture { uv_mapping pigment { image_map { tga "
         << "\"Trees\\Textures\\" << tgaFileName << "\" } } }" << std::endl;
}

void Texture::OutputToPOVRayAlphaMapped(std::ostream &stream) const {
  std::string tgaFileName = getFilename();
  std::string alphaMapFileName(tgaFileName);
  tgaFileName.replace(tgaFileName.length() - 4, 4, ".tga");
  alphaMapFileName.replace(tgaFileName.length() - 4, 4, "_tr.tga");

  stream << "texture { uv_mapping \n"
         << "image_pattern { tga \"Trees\\Textures\\" << alphaMapFileName
         << "\" } \n"
         << "texture_map { \n"
         << "	[0.5 pigment{ Clear } finish {ambient 0 diffuse 0} ] \n"
         << "    [0.5 pigment{ image_map{ tga \"Trees\\Textures\\"
         << tgaFileName
         << "\" interpolate 2 } } finish {ambient 0 diffuse 0.8 specular 0.1 "
            "roughness 0.3} ] \n"
         << "} rotate 90*z \n"
         << "}\n";
}

class RGBfile {
public:
  RGBfile(const char *fname) {
    _pImg = iopen(fname, "rb", 0, 0, 0, 0, 0);
    if (0 == _pImg)
      throw Exception("[Texture.cpp] Cannot open file %s for reading\n", fname);
  }
  ~RGBfile() { iclose(_pImg); }
  int xsize() const { return _pImg->xsize; }
  int ysize() const { return _pImg->ysize; }
  int zsize() const { return _pImg->zsize; }
  void GetRow(int row, int z, unsigned short *arr) {
    if (0 != getrow(_pImg, arr, row, z))
      throw Exception("Error reading line %d from file %s\n", row, _pImg->name);
  }

private:
  IMAGE *_pImg;
};

unsigned char *Texture::_LoadRGB(const char *fname) {
  RGBfile rgb(fname);

  _Xsize = 1;
  while (rgb.xsize() > _Xsize)
    _Xsize <<= 1;

  if (_Xsize != rgb.xsize())
    throw Exception("Width of texture %s is not a power of 2\n", fname);

  _Ysize = 1;
  while (rgb.ysize() > _Ysize)
    _Ysize <<= 1;

  if (_Ysize != rgb.ysize())
    throw Exception("Height of texture %s is not a power of 2\n", fname);

  _Zsize = rgb.zsize();

  if (_Zsize != 3 && _Zsize != 4)
    throw Exception("Texture %s is not an RGB or RGBA image\n", fname);

  auto_vector<unsigned short> row(_Xsize);

  int bfsize = _Xsize * _Ysize * _Zsize * sizeof(GLubyte);

  auto_vector<GLubyte> buffer(bfsize);

  for (int y = 0; y < _Ysize; ++y) {
    rgb.GetRow(y, 0, row.get());
    int x;
    for (x = 0; x < _Xsize; ++x) {
      buffer.set(y * _Xsize * _Zsize + x * _Zsize + 0,
                 static_cast<GLubyte>(row.get(x)));
    }

    rgb.GetRow(y, 1, row.get());
    for (x = 0; x < _Xsize; ++x)
      buffer.set(y * _Xsize * _Zsize + x * _Zsize + 1,
                 static_cast<GLubyte>(row.get(x)));

    rgb.GetRow(y, 2, row.get());
    for (x = 0; x < _Xsize; ++x)
      buffer.set(y * _Xsize * _Zsize + x * _Zsize + 2,
                 static_cast<GLubyte>(row.get(x)));

    if (4 == _Zsize) {
      rgb.GetRow(y, 3, row.get());
      for (x = 0; x < _Xsize; ++x)
        buffer.set(y * _Xsize * _Zsize + x * _Zsize + 3,
                   static_cast<GLubyte>(row.get(x)));
    }
  }

  return buffer.release();
}

unsigned char *Texture::_LoadPNG(const char *fname) {
#ifndef WIN32
  // load the image file with given filename
  std::vector<unsigned char> pngbuffer; 
  unsigned error;
  error = lodepng::load_file(pngbuffer, fname); 
  if (error) {
    return NULL;
  }

  // decode
  unsigned width, height;
  lodepng::State state;
  std::vector<unsigned char> sRGBbuffer;
  error = lodepng::decode(sRGBbuffer, width, height, state, pngbuffer);
  if (error) {
    return NULL;
  }

  // convert to sRGB color space if necessary (using ICC profile embeded in PNG file)
  std::vector<unsigned char> image (sRGBbuffer.size()); // raw pixels
  error = lodepng::convertToSrgb (image.data(), sRGBbuffer.data(), width, height, &state);
  if (error) {
    Utils::Message("sRGB conversion did not work on file: %s\n",fname);
    return NULL;
  }
  
  // use image here
  int mipmapImageAllocSize;
  size_t jump = 1;
  unsigned int x, y;

  mipmapImageAllocSize = (width + 1) * (height + 1) * 4 * sizeof(GLubyte);
  auto_vector<GLubyte> buffer(mipmapImageAllocSize);

  if (width / 4096 >= jump)
    jump = width / 4096 + 1;
  if (height / 4096 >= jump)
    jump = height / 4096 + 1;

  _Xsize = width;
  _Ysize = height;
  _Zsize = 4;

  /*plot the pixels of the PNG file*/
  for (y = 0; y + jump - 1 < height; y += jump) {
    for (x = 0; x + jump - 1 < width; x += jump) {
      /*get RGBA components*/
      buffer.set(
          4 * y * width + 4 * x + 0,
          static_cast<GLubyte>(
              image[4 * (height - y - jump) * width + 4 * (x) + 0])); /* r */
      buffer.set(
          4 * y * width + 4 * x + 1,
          static_cast<GLubyte>(
              image[4 * (height - y - jump) * width + 4 * (x) + 1])); /* g */
      buffer.set(
          4 * y * width + 4 * x + 2,
          static_cast<GLubyte>(
              image[4 * (height - y - jump) * width + 4 * (x) + 2])); /* b */
      buffer.set(4 * y * width + 4 * x + 3,
                 static_cast<GLubyte>(image[4 * (height - y - jump) * width +
                                            4 * (x) + 3])); /* alpha */
    }
  }
  return buffer.release();
#else // WIN32
  return NULL;
#endif
}

void Texture::MakeActive() const {
  ASSERT(tUninitialized != _id);
  glBindTexture(GL_TEXTURE_2D, _id);
}

