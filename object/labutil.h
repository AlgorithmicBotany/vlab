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




#ifndef _LABUTIL
#define _LABUTIL
/*
 * Labutil.h:
 */

#define STRLEN		6000
#define MAXTOOLS	300
#define MAXITEMS	20

class QAction;

/* generic tools info - as read by object mgr */
struct gtools {
    char *tname;
    char *menu_item[MAXITEMS];
    char *cmdline[MAXITEMS];
} ;

class QMenu;

struct spec {
    char *sname;
    struct spec *nextspec;
    struct spec *subitems;
    char *cmdline;
    QAction *sbutton;
    QMenu *submenu;
};

/* function prototypes */
struct spec * spsearch(struct spec *sp, char *specs);
int gtsearch(struct gtools[], char *line);

#endif
