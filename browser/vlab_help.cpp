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



#include "vlab_help.h"
#include <stdio.h>
#include <stdlib.h>

int vlab_help(const char *url_str) {
  char command[4096];

  // first try remote netscape
  sprintf(command, "netscape -raise -remote 'openURL(%s)' 1> /dev/null 2>&1",
          url_str);
  if (!system(command))
    return 0;

  // remote failed, try to invoke a new netscape
  sprintf(command, "netscape %s &", url_str);
  if (!system(command))
    return 0;

  return -1;
}
