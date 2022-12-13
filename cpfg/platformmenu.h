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



#ifndef __PLATFORMMENU_H__
#define __PLATFORMMENU_H__

#if (defined _GLUT_WINDOWING) || (defined WIN32)

void Initialize_Menus(void);

#else

#include <Xm/Xm.h>

void Initialize_Menus(Widget main_window, Widget top_shell);

#endif

#define MAXWINLEN 20 /* maximal length of the displayed filename */

/* menus */

#ifdef __cplusplus
extern "C" {
#endif

void makeMenus(void);
void SetAnimateMenu(void);
void SetMainMenu(void);
void Change_Filename(int i);
void Dialog_Box(char *str, char *filename, int format);
void Change_Resolution(void);
int is_menu_up(void);

#ifdef __cplusplus
}
#endif

#endif
