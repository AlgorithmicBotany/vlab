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
#include "get_extensions.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * RA::Get_extensions( RA_Connection * connection, char * fname, char **& list)
 *
 * - obtains a list of files in the 'fname'/ext directory, and for
 *   each file it will also figure out whether the file is a link,
 *   whether it is a directory, and what the real path is

 *   the format for each entry in 'list' is:
 *
 *        first byte = flag ( 0x01 = it is a directory, 0x02 = it is a link)
 *
 *        then a name (short) follows, terminated by \0
 *
 *        then a realpath follows, also terminated by \0

 * - returns: number of extensions in the directory
 *            -1 = failure
 *            -2 = server connection error
 *
 */

int RA::Get_extensions(RA_Connection *connection, const char *obj_name,
                       char **&list) {
  debug_printf("RA::Get_extensions( %s:%s)\n", connection->host_name, obj_name);

  // if the connection is local, return
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return get_extensions(obj_name, list);
  }

  // the connection is remote
  debug_printf("sending Message to server\n ");

  if (connection->reconnect())
    return -2;

  // send a message to the server requesting a list of extensions
  Message request(RA_GET_EXTENSIONS_REQUEST, obj_name, xstrlen(obj_name) + 1);
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();

    return -1;
  }
  debug_printf(" Message sent, waiting for server response\n ");
  /*
  if (connection->check_connection()){
    debug_printf("connection Ok ");
  }
  else{
    debug_printf("Connection can't be reestablished : Operation is aborted\n");
    error_code = RA_SERVER_ERROR;
    return -1;
  }
  */
  // receive response
  Message *m = connection->messagePipe->get_message(RA_GET_EXTENSIONS_RESPONSE);
  debug_printf("get response from server\n");
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
  list = NULL;
  char *ptr = m->data + 1; // skip the 'y'
  while (*ptr != '\0') {
    // calculate the length of this entry into 'len'
    int len = 0;
    {
      int len1 = xstrlen(ptr) + 1;
      int len2 = xstrlen(ptr + len1) + 1;
      len = len1 + len2 + 1;
    }

    // copy this entry into the result list
    nfiles += 1;
    list = (char **)xrealloc(list, sizeof(char *) * nfiles);
    list[nfiles - 1] = (char *)xmalloc(len);
    memcpy(list[nfiles - 1], ptr, len);

    // update 'ptr'
    ptr += len;
  }

  // delete the message
  delete m;

  connection->Disconnect();

  // return number of files in the list
  return nfiles;
}
