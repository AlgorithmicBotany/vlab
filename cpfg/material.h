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



#ifndef _CPFG_MATERIAL__
#define _CPFG_MATERIAL__

enum PropertyType {
  PT_INT,
  PT_FLOAT,
  PT_STRING,
  PT_COLOR,
  PT_VECTOR,
  PT_SHADER,
  PT_OBJECT
};

/* Structure to hold material information.  A material is recognized by
a unique material id, as well as by a material number.  The material number is
unique only among materials of type shaderName.  For example, a "phong" material
and a "rough" material may both have material numbers 5, but their material ids
will always be different.  Two "phong" materials, or two "rough" materials, will
never have the same material number.

The array propertyTypes indicates what kind of information the property returns.
This could be primitive data types (PT_INT, PT_FLOAT), arrays (PT_STRING,
PT_COLOR, PT_VECTOR), or a shader (PT_SHADER).

If the property value is fixed, it will be stored as a string in the
propertyValues array, regardless of the property's type.  For example, a float
property value of 3.14 would be stored as the string "3.14".  A color property
value of [1 0 1] would be stored as the string "1 0 0".

If the property value is returned via a subshader, the element in the
propertyValues array will be null, and a address will be found in the
subMaterials array.  The address points to another material structure containing
the information for the subshader.  A property of type PT_SHADER will always
return an address to a subshader in the subMaterials array.
*/

struct MaterialStruct {
  char *shaderName;
  int materialNumber; // unique number only among materials of type shaderName
  int materialID; // unique id among ALL materials (regardless of shader type)

  enum PropertyType *propertyTypes;
  char **propertyNames;
  char **propertyValues;
  struct MaterialStruct **subMaterials;
  int propertyNum; // number of properties (ie. size of arrays)

  char writeFlag; // has material been written to rendering file?
};

typedef struct MaterialStruct Material;

#ifdef __cplusplus
extern "C" {
#endif
Material *newMaterial(int propertyNumber);
void deleteMaterial(Material *material);
void printMaterial(const Material *const material);
void materialName(const Material *mat, char *name);
void resetMaterials();
void clearMaterialHistory();
const Material *getMaterial(int id);

void queueSubMaterial(Material *material);
Material *popSubMaterial();
void clearSubMaterials();
void rewindSubMaterials();

#ifdef __cplusplus
}
#endif

#endif
