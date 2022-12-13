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
#include <string.h>
#include <unistd.h>

#include "RA.h"
#include "debug.h"
#include "dsprintf.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * int RA::Rename( RA_Connection * connection,
 *                 const char * src_fname,
 *                 const char * dst_fname)
 *
 * - rename a file on a remote host
 *
 * - return:  0 = success
 *           !0 = failure
 *
 */

int RA::Rename(RA_Connection *connection, const char *src_fname,
               const char *dst_fname) {

  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Rename( %s:%s, %s)\n", connection->host_name, src_fname,
               dst_fname);

  // if the connection is local, use rename(2)
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return rename(src_fname, dst_fname);
  }

  // the connection is remote
  if (connection->reconnect())
    return 1;

  // send message to the server requesting a RENAME operation
  Message request;
  request.code = RA_RENAME_REQUEST;
  request.length = xstrlen(src_fname) + 1 + xstrlen(dst_fname) + 1;
  request.data = (char *)xmalloc(request.length);
  memcpy(request.data, src_fname, xstrlen(src_fname) + 1);
  memcpy(request.data + xstrlen(src_fname) + 1, dst_fname,
         xstrlen(dst_fname) + 1);

  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    connection->Disconnect();
    return 1;
  }

  // wait for the response
  Message *response = connection->messagePipe->get_message(RA_RENAME_RESPONSE);
  if (response == NULL) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();
    return 1;
  }

  // is the response 'success'?
  int result = 0;
  if (response->data[0] == 'n')
    result = 1;

  // delete the response and return the result
  delete response;
  connection->Disconnect();

  return result;
}
