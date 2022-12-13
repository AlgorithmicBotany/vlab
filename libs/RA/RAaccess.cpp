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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "RA.h"
#include "debug.h"
#include "dsprintf.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * partially implements access(2) function
 *
 * returns  0 = success
 *         !0 = requested access not granted
 *
 */

int RA::Access(RA_Connection *connection, const char *fname, const int amode) {
  assert(connection != NULL);
  if (connection->connection_type == RA_NO_CONNECTION) {
    perror("Request access is not granted because raserver connection is not "
           "open\n");
    return 1;
  }

  debug_printf("RA::Access( %s:%s, %d)\n", connection->host_name, fname, amode);

  // if the connection is local, return what 'access(2)' would return
  if (connection->connection_type == RA_LOCAL_CONNECTION)
    return access(fname, amode);

  // the connection is remote
  RA_Stat_Struc buff;

  // get stats about the file
  if (0 != Stat(connection, fname, &buff))
    return 1;

  // if the file is not present, return error
  if (buff.type == RA_NOEXIST_TYPE)
    return 1;

  // test individual flags

  if (amode & R_OK)
    if (!buff.readable)
      return 1;

  if (amode & W_OK)
    if (!buff.writeable)
      return 1;

  if (amode & X_OK)
    if (!buff.executable)
      return 1;

  // return success
  return 0;
}
