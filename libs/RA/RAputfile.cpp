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
 * int RA::Put_file( const char * src_fname,
 *                   RA_Connection * connection,
 *                   const char * dst_fname);
 *
 * - will copy a local file 'src_fname' to a host specified by
 *   'connection' into file 'dst_fname'
 *
 * - returs:  0 = success
 *           !0 = failure
 *
 */

int RA::Put_file(const char *src_fname, RA_Connection *connection,
                 const char *dst_fname) {
  assert(connection != NULL);
  assert(connection->connection_type != RA_NO_CONNECTION);

  debug_printf("RA::Put_file( '%s', '%s:%s')\n", src_fname,
               connection->host_name, dst_fname);

  // if the connection is local, do a local copy
  if (connection->connection_type == RA_LOCAL_CONNECTION) {
    return cpfile(src_fname, dst_fname);
  }

  // create a message
  if (connection->reconnect())
    return -5;

  Message request;
  request.code = RA_PUTFILE_REQUEST;
  // read the file into message data
  int fd = open(src_fname, O_RDONLY);
  if (fd == -1) {
    error_code = RA_OPEN_FAILED;
    connection->Disconnect();

    return 1;
  }

  // put the destination name at the beginning of 'buf'
  int n = xstrlen(dst_fname) + 1;
  int alloc = n + 4096;
  char *buf = (char *)xmalloc(alloc);
  memcpy(buf, dst_fname, n);
  // read the entire file into the rest of 'buf'
  while (1) {
    int l = read(fd, buf + n, alloc - n);
    if (l <= 0)
      break;
    n += l;
    if (alloc == n) {
      alloc += 4096;
      buf = (char *)xrealloc(buf, alloc);
    }
  }
  close(fd);
  // finish creating the message
  request.data = buf;
  request.length = n;

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
