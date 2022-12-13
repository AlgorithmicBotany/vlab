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
#include "xstring.h"

/******************************************************************************
 *
 * applies 'readlink()' to the specified path
 *
 * returns:   0 = success
 *            1 = failure
 */

int RA::Readlink(RA_Connection *connection, const char *path, char *(&result)) {
  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Readlink( '%s:%s')\n", connection->host_name, path);

  // remote/local?
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    char tmp[4096];
    int length = readlink(path, tmp, sizeof(tmp));
    if (length <= 0) {
      error_code = RA_READLINK_FAILED;
      return 1;
    }
    tmp[length] = '\0';

    // copy the result
    result = xstrdup(tmp);
    debug_printf("...result = %s\n", result);
    return 0;
  }

  // remote execution
  if (connection->reconnect())
    return 1;

  // prepare the message
  Message request(RA_READLINK_REQUEST, path, xstrlen(path) + 1);
  // send the message
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    connection->Disconnect();
    return -1;
  }

  // receive a reply
  Message *reply = connection->messagePipe->get_message(RA_READLINK_RESPONSE);
  if (reply == NULL) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();
    return -2;
  }

  // was it successful?
  if (reply->data[0] != 'y') {
    // remote readlink() was unsuccessful
    delete reply;
    error_code = RA_READLINK_FAILED;
    connection->Disconnect();
    return 1;
  }

  // extract the message
  result = xstrdup(reply->data + 1);

  debug_printf("...result = %s\n", result);

  // delete the reply
  delete reply;
  connection->Disconnect();

  // return success
  return 0;
}
