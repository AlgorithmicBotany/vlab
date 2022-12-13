#ifndef __FG_GEOMETRY_H__
#define __FG_GEOMETRY_H__

/*
 * fg_geometry.h
 *
 * The geometry part of the freeglut library include file
 *
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Thu Dec 2 1999
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PAWEL W. OLSZTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Always include OpenGL and GLU headers
 */
#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif WIN32
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

/*
 * Geometry functions, see fg_geometry.c
 */
void glutWireCube(double size);
void glutSolidCube(double size);
void glutWireSphere(double radius, GLint slices, GLint stacks);
void glutSolidSphere(double radius, GLint slices, GLint stacks);
void glutWireCone(double base, double height, GLint slices, GLint stacks);
void glutSolidCone(double base, double height, GLint slices, GLint stacks);
void glutWireTorus(double innerRadius, double outerRadius, GLint sides,
                   GLint rings);
void glutSolidTorus(double innerRadius, double outerRadius, GLint sides,
                    GLint rings);
void glutWireDodecahedron(void);
void glutSolidDodecahedron(void);
void glutWireOctahedron(void);
void glutSolidOctahedron(void);
void glutWireTetrahedron(void);
void glutSolidTetrahedron(void);
void glutWireIcosahedron(void);
void glutSolidIcosahedron(void);

#ifdef __cplusplus
}
#endif

/*** END OF FILE ***/

#endif /* __FG_GEOMETRY_H__ */
