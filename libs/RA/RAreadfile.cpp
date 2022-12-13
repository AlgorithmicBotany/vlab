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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "RA.h"
#include "debug.h"
#include "dsprintf.h"
#include "utilities.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 *   - copies a remote file to a 'buffer' and put its size into 'size'
 *
 *   - returns:  0 = success
 *               1 = failure
 *
 */

int RA::Read_file(RA_Connection *connection, const char *fname, char *(&buffer),
                  long &size) {
  assert(connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Read_file( %s:%s)\n", connection->host_name, fname);

  // if the connection is local, do a local read
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    // open source file
    int fd = open(fname, O_RDONLY);
    if (fd == -1) {
      error_code = RA_OPEN_FAILED;
      return -1;
    }

    // read the file into buffer and determine 'size'
    size = 0;
    int alloc = 16384;
    buffer = (char *)xmalloc(alloc);
    while (1) {
      int n = read(fd, buffer + size, alloc - size);
      if (n <= 0)
        break;
      size += n;
      if (alloc <= size) {
        alloc += 16384;
        buffer = (char *)xrealloc(buffer, alloc);
      }
    }

    // we are done
    close(fd);
    return 0;
  }

  //
  // the connection is to a remote host
  //

  if (connection->reconnect())
    return 1;

  // create a request message
  Message request;
  request.code = RA_FETCH_FILE_REQUEST;
  request.data = (char *)xstrdup(fname);
  request.length = xstrlen(fname) + 1;

  // send the message
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_FETCH_FAILED;
    connection->Disconnect();
    return 1;
  }

  // wait for the reply
  Message *response =
      connection->messagePipe->get_message(RA_FETCH_FILE_RESPONSE);
  if (response == NULL) {
    error_code = RA_FETCH_FAILED;
    connection->Disconnect();
    return 1;
  }

  // check if the content of the file arrived
  if (response->length < 1) {
    error_code = RA_FETCH_FAILED;
    delete response;
    connection->Disconnect();
    return 1;
  }

  // check the first character of the response
  if (response->data[0] != 'y') {
    error_code = RA_FETCH_FAILED;
    delete response;
    connection->Disconnect();
    return 1;
  }

  // copy the file into the buffer and copy size as well
  size = response->length - 1;
  buffer = (char *)xmalloc(size);
  memcpy(buffer, response->data + 1, size);

  // delete response
  delete response;
  connection->Disconnect();

  // return success
  return 0;
}
