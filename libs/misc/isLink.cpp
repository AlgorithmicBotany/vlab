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



#include <sys/stat.h>
#include <sys/types.h>

#include "isLink.h"

/******************************************************************************
 *
 * isLink() -
 *
 * returns 1 if the 'name' is a link, returns 0 otherwise
 *
 */

int isLink(char *name) {
  struct stat bufStat;

  if (0 != lstat(name, &bufStat))
    return 0;

  if (S_ISLNK(bufStat.st_mode))
    return 1;
  else
    return 0;
}
