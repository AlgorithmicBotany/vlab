#include "texture.hpp"

#include <iostream>
#include <QImage>

/** @brief Default constructor. */
util::Texture1D::Texture1D() :
  data(0),
  size(0),
  tex_name(0)
{}

/** @brief Copy Constructor. */
util::Texture1D::Texture1D(const Texture1D& texture) :
  data(0),
  size(0),
  tex_name(0)
{
  if (!texture.data) return;

  size = texture.size;
  data = new GLubyte[size * 4];

  for (unsigned int i = 0; i < size; i++)
    data[i] = texture.data[i];

  glEnable(GL_TEXTURE_1D);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &tex_name);
  glBindTexture(GL_TEXTURE_1D, tex_name);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  clamp();
  filter();
  modulate();
}

/** @brief Constructor initialising from an image file.
    @param filename The file containing the image.  The image must be
                    in a format supported by the Qt image filters.
*/
// This constructor is unsafe because it calls OpenGL functions with no gaurantee that a OpenGL context is valid
// It is safer to remove it and for the user to call the empty constructor and then load(filename) in render_init.
/* 
util::Texture1D::Texture1D(std::string filename) :
  data(0),
  size(0),
  tex_name(0)
{
  QImage image(filename.c_str());
  if (!image.width()) {
    std::cerr << "Error: Texture initialised by "
	      << filename << " is empty." << std::endl;
    return;
  }

  size = image.width();

  data = new GLubyte[size * 4];
  for (unsigned int x = 0; x < size; x++) {
    data[x * 4 + 0] = (GLubyte)qRed(image.pixel(x, 0));
    data[x * 4 + 1] = (GLubyte)qGreen(image.pixel(x, 0));
    data[x * 4 + 2] = (GLubyte)qBlue(image.pixel(x, 0));
    data[x * 4 + 3] = (GLubyte)qAlpha(image.pixel(x, 0));
  }

  glEnable(GL_TEXTURE_1D);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &tex_name);
  glBindTexture(GL_TEXTURE_1D, tex_name);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  clamp();
  filter();
  modulate();
}
*/

void util::Texture1D::load(std::string filename) 
{
  QImage image(filename.c_str());
  if (!image.width()) {
    std::cerr << "Error: Texture initialised by "
	      << filename << " is empty." << std::endl;
    return;
  }

  size = image.width();

  data = new GLubyte[size * 4];
  for (unsigned int x = 0; x < size; x++) {
    data[x * 4 + 0] = (GLubyte)qRed(image.pixel(x, 0));
    data[x * 4 + 1] = (GLubyte)qGreen(image.pixel(x, 0));
    data[x * 4 + 2] = (GLubyte)qBlue(image.pixel(x, 0));
    data[x * 4 + 3] = (GLubyte)qAlpha(image.pixel(x, 0));
  }

  glEnable(GL_TEXTURE_1D);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &tex_name);
  glBindTexture(GL_TEXTURE_1D, tex_name);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  clamp();
  filter();
  modulate();
}


/** @brief Assignment operator. */
const util::Texture1D& util::Texture1D::operator=(const util::Texture1D& texture) {
  glDeleteTextures(1, &tex_name);
  delete[] data;
  data = 0;
  size = 0;

  if (!texture.data) return *this;

  size = texture.size;
  data = new GLubyte[size * 4];

  for (unsigned int i = 0; i < size; i++)
    data[i] = texture.data[i];

  glEnable(GL_TEXTURE_1D);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &tex_name);
  glBindTexture(GL_TEXTURE_1D, tex_name);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  clamp();
  filter();
  modulate();

  return *this;
}

/** @brief Destructor. */
util::Texture1D::~Texture1D() {
  glDeleteTextures(1, &tex_name);
  delete[] data;
}

/** @brief Bind the texture. */
void util::Texture1D::bind() {
  glBindTexture(GL_TEXTURE_1D, tex_name);
}

/** @brief Enables texture coordinate clamping.
    @param enable If true enable clamping, if false repeat.
*/
void util::Texture1D::clamp(bool enable) {
  if (enable)
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  else
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
}

/** @brief Enables filtering.
    @param enable If true, filtering is enabled, if false it is disabled/
*/
void util::Texture1D::filter(bool enable) {
  if (enable) {
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
  else {
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }     
}

/** @brief Sets the blending mode to modulate. */
void util::Texture1D::modulate() {
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

/** @brief Sets the blending mode to decal. */
void util::Texture1D::decal() {
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

/** @brief Sets the blending mode to blend. */
void util::Texture1D::blend() {
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
}

/** @brief Sets the blending mode to replace. */
void util::Texture1D::replace() {
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/** @brief Default constructor. */
util::Texture2D::Texture2D() :
  data(0),
  size(0),
  width(0),
  height(0),
  tex_name(0)
{}

/** @brief Copy constructor. */
util::Texture2D::Texture2D(const Texture2D& texture) :
  data(0),
  size(0),
  tex_name(0)
{
  if (!texture.data) return;

  size = texture.size;
  width = texture.width;
  height = texture.height;
  data = new GLubyte[size * 4];

  for (unsigned int i = 0; i < size * 4; i++)
    data[i] = texture.data[i];

  glEnable(GL_TEXTURE_2D);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &tex_name);
  glBindTexture(GL_TEXTURE_2D, tex_name);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  clamp();
  filter();
  modulate();
}

/** @brief Constructor initialising from an image file.
    @param filename The file containing the image.  The image must be
                    in a format supported by the Qt image filters.
*/
// This constructor is unsafe because it calls OpenGL functions with no gaurantee that a OpenGL context is valid
// It is safer to remove it and for the user to call the empty constructor and then load(filename) in render_init.
/* 
util::Texture2D::Texture2D(std::string filename) :
  data(0),
  size(0),
  tex_name(0)
{
  QImage image(filename.c_str());
  if (!image.width()) {
    std::cerr << "Error: Texture initialised by "
	      << filename << " is empty." << std::endl;
    return;
  }

  width = image.width();
  height = image.height();
  size = width * height;

  data = new GLubyte[size * 4];
  for (unsigned int x = 0; x < width; x++) {
    for (unsigned int y = 0; y < height; y++) {
      data[(x + y * width) * 4 + 0] = (GLubyte)qRed(image.pixel(x, y));
      data[(x + y * width) * 4 + 1] = (GLubyte)qGreen(image.pixel(x, y));
      data[(x + y * width) * 4 + 2] = (GLubyte)qBlue(image.pixel(x, y));
      data[(x + y * width) * 4 + 3] = (GLubyte)qAlpha(image.pixel(x, y));
    }
  }

  // calling the gl funtions will cause a segmentation fault if the OpenGL context is invalid.
  // these calls should be made in render_init or render.
  glEnable(GL_TEXTURE_2D);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &tex_name);
  glBindTexture(GL_TEXTURE_2D, tex_name);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  clamp();
  filter();
  modulate();
}
*/

void util::Texture2D::load(std::string filename) 
{
  QImage image(filename.c_str());
  if (!image.width()) {
    std::cerr << "Error: Texture initialised by "
	      << filename << " is empty." << std::endl;
    return;
  }

  width = image.width();
  height = image.height();
  size = width * height;

  data = new GLubyte[size * 4];
  for (unsigned int x = 0; x < width; x++) {
    for (unsigned int y = 0; y < height; y++) {
      data[(x + y * width) * 4 + 0] = (GLubyte)qRed(image.pixel(x, y));
      data[(x + y * width) * 4 + 1] = (GLubyte)qGreen(image.pixel(x, y));
      data[(x + y * width) * 4 + 2] = (GLubyte)qBlue(image.pixel(x, y));
      data[(x + y * width) * 4 + 3] = (GLubyte)qAlpha(image.pixel(x, y));
    }
  }

  glEnable(GL_TEXTURE_2D);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &tex_name);
  glBindTexture(GL_TEXTURE_2D, tex_name);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  clamp();
  filter();
  modulate();
}


/** @brief Assignment operator. */
const util::Texture2D& util::Texture2D::operator=(const util::Texture2D& texture) {
  glDeleteTextures(1, &tex_name);
  delete[] data;
  data = 0;
  size = 0;
  width = 0;
  height = 0;

  if (!texture.data) return *this;

  size = texture.size;
  data = new GLubyte[size * 4];

  for (unsigned int i = 0; i < size * 4; i++)
    data[i] = texture.data[i];

  glEnable(GL_TEXTURE_2D);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &tex_name);
  glBindTexture(GL_TEXTURE_2D, tex_name);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  clamp();
  filter();
  modulate();

  return *this;
}

/** @brief Destructor. */
util::Texture2D::~Texture2D() {
  glDeleteTextures(1, &tex_name);
  delete[] data;
}

/** @brief Bind the texture. */
void util::Texture2D::bind() {
  glBindTexture(GL_TEXTURE_2D, tex_name);
}

/** @brief Enables texture coordinate clamping.
    @param enable If true enable clamping, if false repeat.
*/
void util::Texture2D::clamp(bool enable) {
  if (enable) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  }
  else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
}

/** @brief Enables filtering.
    @param enable If true, filtering is enabled, if false it is disabled/
*/
void util::Texture2D::filter(bool enable) {
  if (enable) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
  else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }     
}

/** @brief Sets the blending mode to modulate. */
void util::Texture2D::modulate() {
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

/** @brief Sets the blending mode to decal. */
void util::Texture2D::decal() {
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

/** @brief Sets the blending mode to blend. */
void util::Texture2D::blend() {
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
}

/** @brief Sets the blending mode to replace. */
void util::Texture2D::replace() {
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
