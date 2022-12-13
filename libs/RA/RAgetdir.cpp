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
#include "dirList.h"
#include "dsprintf.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * RA::Get_dir( RA_Connection * connection, char * fname, char *** list)
 *
 * - obtains a directory list from a remote host and returns the entries
 *   in list (if list != NULL)
 * - returns: number of entries in that directory (excluding '.' and '..')
 *            -1 = failure
 *
 *
 */

int RA::Get_dir(RA_Connection *connection, const char *dir_name, char ***list) {
  // assert( connection-> connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Get_dir( %s:%s)\n", connection->host_name, dir_name);

  // if the connection is local, return
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return dirList(dir_name, list);
  }

  // the connection is remote
  if (connection->reconnect())
    return -1;

  // send a message to the server requesting a directory list
  Message request(RA_GETDIR_REQUEST, dir_name, xstrlen(dir_name) + 1);
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();
    return -1;
  }

  // receive response
  Message *m = connection->messagePipe->get_message(RA_GETDIR_RESPONSE);
  if (m == NULL) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();
    return -1;
  }

  // does the response contain a valid list of entries?
  if (m->data[0] != 'y') {
    // the directory list could not be retrieved
    delete m;
    connection->Disconnect();
    return -1;
  }

  // allocate and create the list
  int nfiles = 0;
  char *ptr = m->data + 1;
  if (list != NULL)
    *list = NULL;
  while (*ptr != '\0') {
    // put entries into the list
    nfiles += 1;
    if (list != NULL) {
      *list = (char **)xrealloc(*list, sizeof(char *) * nfiles);
      (*list)[nfiles - 1] = xstrdup(ptr);
    }
    ptr += xstrlen(ptr) + 1;

    if (list != NULL)
      debug_printf("\t'%s'\n", (*list)[nfiles - 1]);
  }

  // delete the message
  delete m;
  connection->Disconnect();

  // return number of files in the list
  return nfiles;
}
