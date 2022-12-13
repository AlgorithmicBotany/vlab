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



#ifndef __DIR_LIST_H__
#define __DIR_LIST_H__

#include <string>
#include <vector>

/* dirList(dirname, dlist)
 *	- uses scandir to get a listing of the directory 'dirname'
 *	- stores entries in 'dlist' (if not NULL), using malloc & strdup
 *	- returns number of directory entries
 */

int dirList(const char *dirname, char ***dlist);

std::vector<std::string> dirList2(const std::string &dirName);

#endif
