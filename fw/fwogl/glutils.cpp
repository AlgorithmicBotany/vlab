/**************************************************************************

  File:		glutils.cpp
  Created:	11-Dec-97


  Implementation of various OpenGL utility classes


**************************************************************************/


#include <fw.h>
#include <gl\gl.h>

#include "worldpoint.h"
#include "glutils.h"

#ifdef _DEBUG
bool GLprimitive::_exists = false;
#endif

