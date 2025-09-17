#include "materials.hpp"
#include <fstream>

/** @brief Constructor.
    @param filename The VLAB material file to read.
*/
util::Materials::Materials(std::string filename) :
  filename(filename)
{
  reread();
}

/** @brief Reread the set material file. */
void util::Materials::reread() {
  unsigned int index = 0;
  std::ifstream in(filename.c_str(), std::ios::binary);

  while (!in.eof() && in.good() && in && ( index < 256 )) {
    unsigned char mat[15];

    for (int i = 0; i < 15; i++) {
	mat[i] = in.get();
    }

    mats[index].transparency = float(mat[1]) / 255.0;

    mats[index].ambient[0] = float(mat[2]) / 255.0;
    if (mats[index].ambient[0] < 0.0 || mats[index].ambient[0] > 1.0) mats[index].ambient[0] = 0.2f;
    mats[index].ambient[1] = float(mat[3]) / 255.0;
    if (mats[index].ambient[1] < 0.0 || mats[index].ambient[1] > 1.0) mats[index].ambient[1] = 0.2f;
    mats[index].ambient[2] = float(mat[4]) / 255.0;
    if (mats[index].ambient[2] < 0.0 || mats[index].ambient[2] > 1.0) mats[index].ambient[2] = 0.2f;
    mats[index].ambient[3] = 1.0f - mats[index].transparency;

    mats[index].diffuse[0] = float(mat[5]) / 255.0;
    if (mats[index].diffuse[0] < 0.0 || mats[index].diffuse[0] > 1.0) mats[index].diffuse[0] = 0.8f;
    mats[index].diffuse[1] = float(mat[6]) / 255.0;
    if (mats[index].diffuse[1] < 0.0 || mats[index].diffuse[1] > 1.0) mats[index].diffuse[1] = 0.8f;
    mats[index].diffuse[2] = float(mat[7]) / 255.0;
    if (mats[index].diffuse[2] < 0.0 || mats[index].diffuse[2] > 1.0) mats[index].diffuse[2] = 0.8f;
    mats[index].diffuse[3] = 1.0f - mats[index].transparency;

    mats[index].emission[0] = float(mat[8]) / 255.0;
    if (mats[index].emission[0] < 0.0 || mats[index].emission[0] > 1.0) mats[index].emission[0] = 0.0f;
    mats[index].emission[1] = float(mat[9]) / 255.0;
    if (mats[index].emission[1] < 0.0 || mats[index].emission[1] > 1.0) mats[index].emission[1] = 0.0f;
    mats[index].emission[2] = float(mat[10]) / 255.0;
    if (mats[index].emission[2] < 0.0 || mats[index].emission[2] > 1.0) mats[index].emission[2] = 0.0f;
    mats[index].emission[3] = 1.0f - mats[index].transparency;

    mats[index].specular[0] = float(mat[11]) / 255.0;
    if (mats[index].specular[0] < 0.0 || mats[index].specular[0] > 1.0) mats[index].specular[0] = 0.0f;
    mats[index].specular[1] = float(mat[12]) / 255.0;
    if (mats[index].specular[1] < 0.0 || mats[index].specular[1] > 1.0) mats[index].specular[1] = 0.0f;
    mats[index].specular[2] = float(mat[13]) / 255.0;
    if (mats[index].specular[2] < 0.0 || mats[index].specular[2] > 1.0) mats[index].specular[2] = 0.0f;
    mats[index].specular[3] = 1.0f - mats[index].transparency;

    mats[index].shiny = float(mat[14]); if (mats[index].shiny < 0.0) mats[index].shiny = 0.0f;

    index++;
  }

  for (; index < 256; index++) {
    mats[index].ambient[0] = 0.2f;
    mats[index].ambient[1] = 0.2f;
    mats[index].ambient[2] = 0.2f;
    mats[index].ambient[3] = 1.0f;

    mats[index].diffuse[0] = 0.8f;
    mats[index].diffuse[1] = 0.8f;
    mats[index].diffuse[2] = 0.8f;
    mats[index].diffuse[3] = 1.0f;

    mats[index].emission[0] = 0.0f;
    mats[index].emission[1] = 0.0f;
    mats[index].emission[2] = 0.0f;
    mats[index].emission[3] = 1.0f;

    mats[index].specular[0] = 0.0f;
    mats[index].specular[1] = 0.0f;
    mats[index].specular[2] = 0.0f;
    mats[index].specular[3] = 1.0f;

    mats[index].shiny = 0.0f;
    mats[index].transparency = 0.0f;
  }
}

/** @brief A call to use a particular material.
    @param index The material index.
*/
void util::Materials::useMaterial(unsigned int index) {
  if (index > 255) index = 255;

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mats[index].ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mats[index].diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mats[index].emission);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mats[index].specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mats[index].shiny);
}

/** @brief A call to get the material structure for a particular index.
    @param index The material index.
*/
const util::Materials::Material& util::Materials::getMaterial(unsigned int index) {
  if (index > 255) index = 255;
  return mats[index];
}

/** @brief Returns a material structure that is a weighted average of two
           materials.
    @param ind1 The first material index.
    @param ind2 The second material index.
    @param t The weight for the first material.  If t is clamped to the
           range [0, 1].  The second material is given the weight (1.0 - t).
*/
void util::Materials::blend(unsigned int ind1, unsigned int ind2, float t) {
  Material m;

  if (t < 0.0) t = 0.0;
  else if (t > 1.0) t = 1.0;

  float t1 = 1.0 - t;

  m.ambient[0] = t * mats[ind1].ambient[0] + t1 * mats[ind2].ambient[0];
  m.ambient[1] = t * mats[ind1].ambient[1] + t1 * mats[ind2].ambient[1];
  m.ambient[2] = t * mats[ind1].ambient[2] + t1 * mats[ind2].ambient[2];
  m.ambient[3] = t * mats[ind1].ambient[3] + t1 * mats[ind2].ambient[3];

  m.diffuse[0] = t * mats[ind1].diffuse[0] + t1 * mats[ind2].diffuse[0];
  m.diffuse[1] = t * mats[ind1].diffuse[1] + t1 * mats[ind2].diffuse[1];
  m.diffuse[2] = t * mats[ind1].diffuse[2] + t1 * mats[ind2].diffuse[2];
  m.diffuse[3] = t * mats[ind1].diffuse[3] + t1 * mats[ind2].diffuse[3];

  m.specular[0] = t * mats[ind1].specular[0] + t1 * mats[ind2].specular[0];
  m.specular[1] = t * mats[ind1].specular[1] + t1 * mats[ind2].specular[1];
  m.specular[2] = t * mats[ind1].specular[2] + t1 * mats[ind2].specular[2];
  m.specular[3] = t * mats[ind1].specular[3] + t1 * mats[ind2].specular[3];

  m.emission[0] = t * mats[ind1].emission[0] + t1 * mats[ind2].emission[0];
  m.emission[1] = t * mats[ind1].emission[1] + t1 * mats[ind2].emission[1];
  m.emission[2] = t * mats[ind1].emission[2] + t1 * mats[ind2].emission[2];
  m.emission[3] = t * mats[ind1].emission[3] + t1 * mats[ind2].emission[3];

  m.shiny = t * mats[ind1].shiny + t1 * mats[ind2].shiny;

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m.ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m.diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, m.emission);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m.specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m.shiny);
}
