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

#include "Mem.h"
#include "RA.h"
#include "debug.h"
#include "delete_object.h"
#include "dsprintf.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * int RA::Delete_object( RA_Connection * connection,
 *                        const char * oofs_dir,
 *                        const char * fname)
 *
 * - delete an object in a remote database and update .dbase appropriately
 *
 * - return:  0 = success
 *           !0 = failure
 *
 */

int RA::Delete_object(RA_Connection *connection, const char *oofs_dir,
                      const char *fname) {

  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Delete_object( %s:%s %s)\n", connection->host_name,
               oofs_dir, fname);

  // if the connection is local, do the operations localy
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    int res = delete_object(oofs_dir, fname);
    if (res) {
      // operation failed - report error
      error_code = RA_DELETE_FAILED;
      return 1;
    }

    // report success
    return 0;
  }

  // the connection is remote
  if (connection->reconnect())
    return 1;

  // send message to the server requesting a DELETE operation
  Mem mem;
  mem.append_string0(oofs_dir);
  mem.append_string0(fname);
  Message request(RA_DELETE_OBJECT_REQUEST, (char *)mem.data, mem.size);

  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    connection->Disconnect();
    return 1;
  }

  // wait for the response
  Message *response =
      connection->messagePipe->get_message(RA_DELETE_OBJECT_RESPONSE);
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
