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



#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "RA.h"
#include "debug.h"

/******************************************************************************
 *
 * determines if the 'fname' is a link or not
 *
 * returns:   1 = link
 *            0 = not a link
 *           -1 = error
 */

int RA::Is_link(RA_Connection *connection, const char *path) {
  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Is_link( '%s:%s')\n", connection->host_name, path);

  RA_Stat_Struc s;
  if (Stat(connection, path, &s))
    return -1;

  if (s.is_link == 'l')
    return 1;
  else
    return 0;
}
