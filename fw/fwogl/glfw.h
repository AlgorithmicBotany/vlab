/**************************************************************************

  File:		glfw.h
  Created:	25-Nov-97


  General include file for Framework


**************************************************************************/


#ifndef __GLFW_H__
#define __GLFW_H__

#ifndef STRICT
	#error STRICT not defined
#endif

#ifndef __FW_H__
	#error fw.h not included
#endif


#ifdef __GL_H__
	#error Do not include gl.h file
#endif

#include <gl\gl.h>
#include <gl\glu.h>


#include "worldpoint.h"
#include "oglcntxt.h"
#include "glutils.h"
#include "openglwnd.h"
#include "materialparams.h"
#include "rotation.h"
#include "viewbox.h"
#include "gltask.h"
#include "gltrackball.h"
#include "displists.h"
#include "fontlist.h"
#include "boundingbox.h"
#include "line.h"


#else
	#error File already included
#endif
