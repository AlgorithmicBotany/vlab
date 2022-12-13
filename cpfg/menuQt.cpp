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



#include <string.h>
#include <stdio.h>

#include <sys/param.h>

//#include "platform.h"
//#include "platformmenu.h"
#include "control.h"
#include "comlineparam.h"

#ifdef TEST_MALLOC
#include "test_malloc.h"
#endif

/* common variables */
int enabled_menus = 1;
int is_menu = 0;

extern void MyExit(int status);

/*********************** GLUT Menus **********************/

extern int animateFlag;
int animateMenu;

int main_menu, animate_menu;

/****************************************************************************/
