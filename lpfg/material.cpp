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



#ifdef _WINDOWS
#include <windows.h>
#include <GL/gl.h>
#else
#include <qgl.h>
#endif

#include "material.h"

Material::Material() {
  _ambient[0] = 0.2f;
  _ambient[1] = 0.2f;
  _ambient[2] = 0.2f;
  _ambient[3] = 1.0f;

  _diffuse[0] = 0.8f;
  _diffuse[1] = 0.8f;
  _diffuse[2] = 0.8f;
  _diffuse[3] = 1.0f;

  _specular[0] = 0.0f;
  _specular[1] = 0.0f;
  _specular[2] = 0.0f;
  _specular[3] = 1.0f;

  _emission[0] = 0.0f;
  _emission[1] = 0.0f;
  _emission[2] = 0.0f;
  _emission[3] = 1.0f;

  _shininess = 0;
}

void Material::Read(std::istream &src) {
  matfiletype mat;
  src.read(reinterpret_cast<char *>(&mat), sizeof(matfiletype));
  _ambient[0] = mat.ambient[0] / 255.0f;
  _ambient[1] = mat.ambient[1] / 255.0f;
  _ambient[2] = mat.ambient[2] / 255.0f;
  _ambient[3] = 1.0f - mat.transparency / 255.0f;

  _diffuse[0] = mat.diffuse[0] / 255.0f;
  _diffuse[1] = mat.diffuse[1] / 255.0f;
  _diffuse[2] = mat.diffuse[2] / 255.0f;
  _diffuse[3] = 1.0f - mat.transparency / 255.0f;

  _specular[0] = mat.specular[0] / 255.0f;
  _specular[1] = mat.specular[1] / 255.0f;
  _specular[2] = mat.specular[2] / 255.0f;
  _specular[3] = 1.0f - mat.transparency / 255.0f;

  _emission[0] = mat.emission[0] / 255.0f;
  _emission[1] = mat.emission[1] / 255.0f;
  _emission[2] = mat.emission[2] / 255.0f;
  _emission[3] = 1.0f - mat.transparency / 255.0f;

  _shininess = mat.shininess;
}

void Material::Apply() const {
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, _ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, _specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, _emission);
  glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, _shininess);
}

void Material::ApplySolid() const { glColor4fv(_emission); }

void Material::OutputToPOVRay(std::ostream &trg) const {
  trg << "texture { ";
  trg << "pigment { "
      << "color rgbt<" << _diffuse[0] << ", " << _diffuse[1] << ", "
      << _diffuse[2] << ", " << 1.0f - _diffuse[3] << "> }";
  if (HasSpecular()) {
    trg << "finish { "
        << "phong " << _shininess << " phong_size 25"
        << " ambient <" << _ambient[0] << ", " << _ambient[1] << ", "
        << _ambient[2] << "> ";
    trg << " diffuse 1";
    trg << " } ";
  }
  trg << " } ";
}
