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



#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "RA.h"
#include "debug.h"
#include "xstring.h"

/******************************************************************************
 *
 * - will create a new directory on a host specified by 'connection'
 *
 *   returns:    0 = success
 *              !0 = failure
 *
 */

int RA::Mkdir(RA_Connection *connection, const char *path,
              mode_t /* mode - ignored */) {
  debug_printf("RA::Mkdir( %s:%s)\n", connection->host_name, path);

  // if connection local, do a local mkdir
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return mkdir(path, 0755);
  }

  if (connection->reconnect())
    return -1;

  // request 'mkdir'
  Message m(RA_MKDIR_REQUEST, path, xstrlen(path) + 1);
  if (connection->messagePipe->send_message(m)) {
    error_code = RA_SOCKET_ERROR;
    return -1;
  }

  // receive reply
  Message *response = connection->messagePipe->get_message(RA_MKDIR_RESPONSE);

  if (response == NULL) {
    error_code = RA_SERVER_ERROR;
    return -2;
  }

  // was the operation successful?
  int result = 0;
  if (response->data[0] != 'y')
    result = 1;

  delete response;
  connection->Disconnect();

  return result;
}

/******************************************************************************
 *
 * - will destroy a directory on a host specified by 'connection'
 *
 *   returns:    0 = success
 *              !0 = failure
 *
 */

int RA::Rmdir(RA_Connection *connection, const char *path) {
  debug_printf("RA::Rmdir( %s:%s)\n", connection->host_name, path);

  // if connection local, do a local rmdir
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return rmdir(path);
  }

  // request 'rmdir'
  if (connection->reconnect())
    return -1;

  Message m(RA_RMDIR_REQUEST, path, xstrlen(path) + 1);
  if (connection->messagePipe->send_message(m)) {
    error_code = RA_SOCKET_ERROR;
    connection->Disconnect();
    return -1;
  }

  // receive reply
  Message *response = connection->messagePipe->get_message(RA_RMDIR_RESPONSE);

  if (response == NULL) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();
    return -2;
  }

  // was the operation successful?
  int result = 0;
  if (response->data[0] != 'y')
    result = 1;

  delete response;
  connection->Disconnect();

  return result;
}
