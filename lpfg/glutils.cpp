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



#include "glutils.h"

#include "utils.h"

void DumpMatrices(const char *fnm, int ln) {
  Utils::Message("%s at %d\n", fnm, ln);
  float arr[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, arr);
  Utils::Message("Modelview:\n");
  for (int i = 0; i < 16; ++i) {
    Utils::Message("%f\t", arr[i]);
    if (3 == i % 4)
      Utils::Message("\n");
  }
  glGetFloatv(GL_PROJECTION_MATRIX, arr);
  Utils::Message("Projection:\n");
  for (int i = 0; i < 16; ++i) {
    Utils::Message("%f\t", arr[i]);
    if (3 == i % 4)
      Utils::Message("\n");
  }
  const float x = arr[10];
  const float y = arr[14];
  const float zn = (1.0f - y) / x;
  const float zf = (-y - 1.0f) / x;
  Utils::Message("Z near = %f,\tZ far = %f\n", zn, zf);
}

void CheckGL() {
  GLenum err = glGetError();
  assert(GL_NO_ERROR == err);
}
