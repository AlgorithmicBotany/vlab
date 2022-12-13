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



#ifndef __SHADERS_H__
#define __SHADERS_H__

// MC - July 2016 - GLSL shaders for generating shadow map

#ifdef __cplusplus
extern "C" {
#endif

void initShadowMap(const char *app_path);
void resizeShadowMap(void);
void freeShadowMap(void);
void beginShadowMap(void);
void endShadowMap(void);
void beginMainShader(void);
void endMainShader(void);

#ifdef __cplusplus
}
#endif

#endif
