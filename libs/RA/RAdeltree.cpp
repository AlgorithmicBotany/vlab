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
#include <unistd.h>

#include "RA.h"
#include "debug.h"
#include "delete_recursive.h"
#include "dsprintf.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * int RA::Deltree( RA_Connection * connection,
 *                  const char * fname);
 *
 * - removes an entire directory (recursively)
 *
 * - returns:  0 = sucess
 *            !0 = failure
 *
 */

int RA::Deltree(RA_Connection *connection, const char *dirname) {

  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Deltree( '%s:%s')\n", connection->host_name, dirname);

  // if connection is local, do system( "rm -rf...")
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    int result = delete_recursive(dirname);
    if (result != 0)
      error_code = RA_DELTREE_FAILED;
    return result;
  }

  // the connection is remote
  if (connection->reconnect())
    return 1;

  // send the request to the server
  Message m(RA_DELTREE_REQUEST, dirname, xstrlen(dirname) + 1);
  if (connection->messagePipe->send_message(m)) {
    error_code = RA_SOCKET_ERROR;
    connection->Disconnect();
    return 1;
  }

  // wait for the response
  Message *r = connection->messagePipe->get_message(RA_DELTREE_RESPONSE);
  if (r == NULL) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();
    return 1;
  }

  // prepare result
  int result = 1;
  if (r->data[0] == 'y')
    result = 0;

  delete r;
  connection->Disconnect();

  return result;
}
