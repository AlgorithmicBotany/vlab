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
#include <string.h>
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
 * - will write the 'buffer' of size 'size' into a remote file 'fname'
 *   specified by 'connection'
 *
 * - returs:  0 = success
 *           !0 = failure
 *
 */

int RA::Write_file(RA_Connection *connection, const char *fname,
                   const char *buffer, const long size) {
  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Write_file( '%s:%s' size=%d\n", connection->host_name,
               fname, size);

  // if the connection is local, do a local write
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    // open destination file
    int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd == -1) {
      error_code = RA_OPEN_FAILED;
      return -1;
    }

    // write data to the file
    if (write(fd, buffer, size) != size) {
      error_code = RA_WRITE_FAILED;
      return -2;
    } else {
      // close the file and exit
      close(fd);
      return 0;
    }
  }

  if (connection->reconnect())
    return 2;

  // create a message
  Message request;
  request.code = RA_PUTFILE_REQUEST;
  request.length = size + xstrlen(fname) + 1;
  request.data = (char *)xmalloc(request.length);
  // put the destination name to the beginning of the buffer
  memcpy(request.data, fname, xstrlen(fname) + 1);
  // the actual file follows
  memcpy(request.data + xstrlen(fname) + 1, buffer, size);

  // send the message
  if (connection->messagePipe->send_message(request)) {
    error_code = RA_SOCKET_ERROR;
    connection->Disconnect();
    return 2;
  }

  // receive response
  Message *response = connection->messagePipe->get_message(RA_PUTFILE_RESPONSE);
  if (response == NULL) {
    error_code = RA_SERVER_ERROR;
    connection->Disconnect();
    return 3;
  }

  // determine the result of the operation
  int result = 0;
  if (response->data[0] == 'n')
    result = 1;

  delete response;
  connection->Disconnect();

  return result;
}
